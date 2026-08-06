module;

#include <WinSock2.h>
#include <ws2tcpip.h>
#include <MSWSock.h>

#include <siege/platform/win/module.hpp>
#include <siege/platform/win/threading.hpp>
#include <cassert>

export module wsock32.shared.client;

import wsock32.shared;
import std;

extern "C++" bool use_custom_backend();

extern "C" {
int __stdcall siege_sendto(SOCKET ws, const char* buf, int len, int flags, const sockaddr* to, int tolen) noexcept;
int __stdcall siege_recvfrom(SOCKET ws, char* buf, int len, int flags, sockaddr* from, int* fromLen) noexcept;
SOCKET __stdcall siege_socket(int af, int type, int protocol) noexcept;
int __stdcall siege_ioctlsocket(SOCKET ws, long cmd, u_long* argp) noexcept;
int __stdcall siege_select(int value, fd_set* read, fd_set* write, fd_set* except, const timeval* timeout) noexcept;
int __stdcall siege_getsockopt(SOCKET ws, int level, int optname, char* optval, int* optlen) noexcept;
int __stdcall siege_getpeername(SOCKET ws, sockaddr* name, int* length) noexcept;
SOCKET __stdcall siege_accept(SOCKET ws, sockaddr* from, int* fromLen) noexcept;
hostent* __stdcall siege_gethostbyname(const char* name) noexcept;
int __stdcall siege_gethostname(char* name, int namelen) noexcept;
servent* __stdcall siege_getservbyname(const char* name, const char* proto) noexcept;
servent* __stdcall siege_getservbyport(int port, const char* proto) noexcept;
protoent* __stdcall siege_getprotobyname(const char* name) noexcept;
protoent* __stdcall siege_getprotobynumber(int number) noexcept;
}

enum struct worker_action : bool
{
  as_is,
  restart_if_stopped
};

export std::jthread& get_select_worker(worker_action action = worker_action::as_is);
export std::jthread& get_overlapped_worker(worker_action action = worker_action::as_is);

namespace fs = std::filesystem;
namespace stl = std::ranges;

// Dual-DLL: wsock32-rpc forwards into ws2_32-rpc, so only one copy of this state.
export struct socket_handle_info
{
  enum struct client_socket_state
  {
    unconnected,
    connecting,
    connected,
    accepted
  };

  enum struct overlapped_state : bool
  {
    non_overlapped,
    overlapped
  };

  void insert(SOCKET socket, int socket_type, overlapped_state is_overlapped, client_socket_state client_state = client_socket_state::unconnected)
  {
    std::unique_lock<std::shared_mutex> lock(mutex);

    auto item = handles.find(socket);

    if (item != handles.end() && item->second.is_closed)
    {
      handles.erase(item);
    }

    handles.emplace(socket, socket_context{ .is_overlapped = is_overlapped, .socket_type = socket_type, .client_state = client_state });
  }

  void close(SOCKET socket)
  {
    std::unique_lock<std::shared_mutex> lock(mutex);

    auto item = handles.find(socket);
    if (item == handles.end())
    {
      return;
    }

    item->second.is_closed = true;
    item->second.io_flags = item->second.select_flags = 0;
  }

  void set_virtual_blocking(SOCKET socket, bool should_block)
  {
    std::unique_lock<std::shared_mutex> lock(mutex);

    auto item = handles.find(socket);
    if (item == handles.end())
    {
      return;
    }

    item->second.is_virtual_blocking = should_block;
  }


  std::shared_ptr<void> temp_disable_virtual_blocking(SOCKET socket)
  {
    auto current_state = is_virtual_blocking(socket);

    set_virtual_blocking(socket, false);
    return std::shared_ptr<void>{
      nullptr, [current_state, socket, this](...) {
        // because someone could change the state in between calls
        // we should only reset it if it is still false.
        if (auto new_state = is_virtual_blocking(socket); new_state == false)
        {
          set_virtual_blocking(socket, current_state);
        }
      }
    };
  }


  void set_client_socket_state(SOCKET socket, client_socket_state state)
  {
    std::unique_lock<std::shared_mutex> lock(mutex);

    auto item = handles.find(socket);
    if (item == handles.end())
    {
      return;
    }

    item->second.client_state = state;
  }

  void set_listening(SOCKET socket, bool is_listening)
  {
    std::unique_lock<std::shared_mutex> lock(mutex);

    auto item = handles.find(socket);
    if (item == handles.end())
    {
      return;
    }

    item->second.is_listening = is_listening;
  }

  void set_overlapped(SOCKET socket, overlapped_state is_overlapped)
  {
    std::unique_lock<std::shared_mutex> lock(mutex);

    auto item = handles.find(socket);
    if (item == handles.end())
    {
      return;
    }

    item->second.is_overlapped = is_overlapped;
  }

  bool is_virtual_blocking(SOCKET socket) const
  {
    std::shared_lock<std::shared_mutex> lock(mutex);
    auto item = handles.find(socket);

    if (item == handles.end())
    {
      return false;
    }

    return item->second.is_virtual_blocking;
  }

  overlapped_state is_overlapped(SOCKET socket) const
  {
    std::shared_lock<std::shared_mutex> lock(mutex);
    auto item = handles.find(socket);

    if (item == handles.end())
    {
      return overlapped_state::non_overlapped;
    }

    return item->second.is_overlapped;
  }

  [[maybe_unused]] int clear_io_flags(SOCKET socket, int flag)
  {
    std::unique_lock<std::shared_mutex> lock(mutex);

    auto item = handles.find(socket);
    if (item == handles.end())
    {
      return 0;
    }

    item->second.io_flags = item->second.io_flags & ~flag;
    return item->second.io_flags;
  }

  [[maybe_unused]] int set_io_flags(SOCKET socket, int flags, std::array<int, FD_MAX_EVENTS> errors)
  {
    std::unique_lock<std::shared_mutex> lock(mutex);

    auto item = handles.find(socket);
    if (item == handles.end())
    {
      return 0;
    }

    item->second.io_flags = item->second.io_flags | flags;
    item->second.event_flags = item->second.event_flags | flags;

    static constexpr std::pair<long, int> network_flags[] = {
      { FD_READ, FD_READ_BIT },
      { FD_WRITE, FD_WRITE_BIT },
      { FD_OOB, FD_OOB_BIT },
      { FD_ACCEPT, FD_ACCEPT_BIT },
      { FD_CONNECT, FD_CONNECT_BIT },
      { FD_CLOSE, FD_CLOSE_BIT },
    };

    for (auto [mask, bit] : network_flags)
    {
      if (flags & mask)
      {
        item->second.event_errors[bit] = errors[bit];
      }
    }

    return item->second.io_flags;
  }

  std::optional<WSANETWORKEVENTS> consume_event_flags(SOCKET socket)
  {
    std::unique_lock<std::shared_mutex> lock(mutex);

    auto item = handles.find(socket);

    if (item == handles.end())
    {
      return std::nullopt;
    }

    if (item->second.is_closed)
    {
      return std::nullopt;
    }

    WSANETWORKEVENTS result{
      .lNetworkEvents = item->second.event_flags
    };
    std::memcpy(result.iErrorCode, item->second.event_errors.data(), sizeof(result.iErrorCode));

    item->second.event_flags = 0;
    item->second.event_errors = decltype(item->second.event_errors){};

    return result;
  }

  bool contains(SOCKET socket) const
  {
    std::shared_lock<std::shared_mutex> lock(mutex);
    return handles.contains(socket);
  }

  struct window_target
  {
    decltype(::PostMessageW)* post_message;
    HWND window;
    u_int message;
  };

  struct event_target
  {
    WSAEVENT event;
  };


  bool set_window_target(SOCKET socket, HWND window, u_int message, int flags)
  {
    std::unique_lock<std::shared_mutex> lock(mutex);

    auto item = handles.find(socket);
    if (item == handles.end())
    {
      return false;
    }

    if (item->second.is_closed)
    {
      return false;
    }

    item->second.target = window_target{ .post_message = ::IsWindowUnicode(window) ? ::PostMessageW : ::PostMessageA, .window = window, .message = message };
    item->second.select_flags = flags;
    item->second.io_flags = 0;
    item->second.event_flags = 0;
    item->second.event_errors = decltype(item->second.event_errors){};
    return true;
  }

  bool set_event_target(SOCKET socket, WSAEVENT event, int flags)
  {
    std::unique_lock<std::shared_mutex> lock(mutex);

    auto item = handles.find(socket);
    if (item == handles.end())
    {
      return false;
    }

    if (item->second.is_closed)
    {
      return false;
    }

    item->second.target = event_target{ .event = event };
    item->second.select_flags = flags;
    item->second.io_flags = 0;
    item->second.event_flags = 0;
    item->second.event_errors = decltype(item->second.event_errors){};
    return true;
  }

  bool clear_target(SOCKET socket)
  {
    std::unique_lock<std::shared_mutex> lock(mutex);

    auto item = handles.find(socket);
    if (item == handles.end())
    {
      return false;
    }

    item->second.target = std::monostate{};
    item->second.select_flags = 0;
    item->second.io_flags = 0;
    item->second.event_flags = 0;
    item->second.event_errors = decltype(item->second.event_errors){};
    return true;
  }


  // TODO rename this
  struct socket_work
  {
    SOCKET socket;
    bool is_closed;
    int socket_type;
    bool is_listening;
    client_socket_state client_state;
    int select_flags;
    int io_flags;
    int event_flags;
    std::array<int, FD_MAX_EVENTS> event_errors;
    std::variant<window_target, event_target> target;
  };

  void get_sockets_to_watch(std::unordered_map<SOCKET, socket_work>& items) const
  {
    std::shared_lock<std::shared_mutex> lock(mutex);
    items.clear();

    // TODO reserve only non-blocking sockets
    items.reserve(handles.size());

    for (auto& handle : handles)
    {
      if (handle.second.select_flags == handle.second.io_flags)
      {
        continue;
      }

      if (std::holds_alternative<std::monostate>(handle.second.target))
      {
        continue;
      }

      std::variant<window_target, event_target> target{};

      if (std::holds_alternative<window_target>(handle.second.target))
      {
        auto& temp = std::get<window_target>(handle.second.target);

        if (!(temp.post_message || temp.window || temp.message))
        {
          continue;
        }

        target = temp;
      }
      else
      {
        auto& temp = std::get<event_target>(handle.second.target);

        if (!(temp.event))
        {
          continue;
        }
        target = temp;
      }

      // leaving out .event_flags = handle.second.event_flags
      // so that fresh event flags can be raised.
      // same goes for .event_errors.
      items.emplace(handle.first, socket_work{ .socket = handle.first, .is_closed = handle.second.is_closed, .socket_type = handle.second.socket_type, .is_listening = handle.second.is_listening, .client_state = handle.second.client_state, .select_flags = handle.second.select_flags, .io_flags = handle.second.io_flags, .target = target

                                  });
    }
  }

  struct overlapped_event
  {
    HANDLE event;
  };

  struct overlapped_callback
  {
    HANDLE thread_handle;
    LPWSAOVERLAPPED_COMPLETION_ROUTINE completion_routine;
  };

  struct overlapped_write_work
  {
    SOCKET socket;
    WSAOVERLAPPED* overlapped;
    std::span<char> data_to_write;
    std::shared_ptr<void> data_token;
    sockaddr addr;
    int addr_size;
    DWORD flags;
    std::variant<std::monostate, overlapped_event, overlapped_callback> target;
  };

  struct overlapped_read_work
  {
    SOCKET socket;
    WSAOVERLAPPED* overlapped;
    std::vector<std::span<char>> read_targets;
    DWORD flags;
    int read_addr_size;
    std::variant<std::monostate, overlapped_event, overlapped_callback> target;
  };

  struct overlapped_result
  {
    std::expected<DWORD, DWORD> result;
    WSAOVERLAPPED* overlapped;
    DWORD flags;
    sockaddr* target_addr;
    int* target_addr_size;
    sockaddr received_addr;
    int received_addr_size;
    LPWSAOVERLAPPED_COMPLETION_ROUTINE completion_callback;
  };

  struct overlapped_write_params
  {
    SOCKET socket;
    WSAOVERLAPPED* overlapped;
    std::vector<char> data;
    sockaddr addr;
    int addr_size;
    DWORD flags;
    HANDLE thread_handle;
    LPWSAOVERLAPPED_COMPLETION_ROUTINE callback;
  };

  struct overlapped_read_params
  {
    SOCKET socket;
    WSAOVERLAPPED* overlapped;
    std::vector<std::span<char>> read_targets;
    sockaddr* addr;
    int* addr_size;
    DWORD flags;
    HANDLE thread_handle;
    LPWSAOVERLAPPED_COMPLETION_ROUTINE callback;
  };

  // TODO: reject in-flight OVERLAPPED* reuse (Pure)
  void queue_overlapped_write_work(overlapped_write_params params)
  {
    std::unique_lock<std::shared_mutex> lock(mutex);
    auto socket_data = handles.find(params.socket);

    if (socket_data == handles.end())
    {
      return;
    }

    auto& state = socket_data->second.overlapped_write_queue.emplace_back();
    state.overlapped = params.overlapped;
    auto temp = std::make_shared<std::vector<char>>(std::move(params.data));
    state.data_to_write = *temp;
    state.data_token = temp;
    state.addr = params.addr;
    state.addr_size = params.addr_size;
    state.flags = params.flags;

    if (params.thread_handle && params.callback)
    {
      state.target = overlapped_callback{ .thread_handle = params.thread_handle, .completion_routine = params.callback };
    }
    else if (params.overlapped->hEvent)
    {
      state.target = overlapped_event{ .event = params.overlapped->hEvent };
    }

    overlapped_sockets.emplace(params.overlapped, params.socket);
  }

  // TODO: reject in-flight OVERLAPPED* reuse (Pure)
  void queue_overlapped_read_work(overlapped_read_params params)
  {
    std::unique_lock<std::shared_mutex> lock(mutex);
    auto socket_data = handles.find(params.socket);

    if (socket_data == handles.end())
    {
      return;
    }

    auto& state = socket_data->second.overlapped_read_queue.emplace_back();
    state.overlapped = params.overlapped;

    state.read_targets = std::move(params.read_targets);
    state.target_addr = params.addr;
    state.target_addr_size = params.addr_size;
    state.flags = params.flags;

    if (params.thread_handle && params.callback)
    {
      state.target = overlapped_callback{ .thread_handle = params.thread_handle, .completion_routine = params.callback };
    }
    else if (params.overlapped->hEvent)
    {
      state.target = overlapped_event{ .event = params.overlapped->hEvent };
    }

    overlapped_sockets.emplace(params.overlapped, params.socket);
  }

  void get_overlapped_sockets_with_work(std::unordered_set<SOCKET>& sockets) const
  {
    std::shared_lock<std::shared_mutex> lock(mutex);
    sockets.clear();
    sockets.reserve(overlapped_sockets.size());

    for (auto& [socket, context] : handles)
    {
      if (stl::any_of(context.overlapped_read_queue, [](auto& work) {
            return std::holds_alternative<std::monostate>(work.socket_result);
          }))
      {
        sockets.emplace(socket);
      }
      else if (stl::any_of(context.overlapped_write_queue, [](auto& work) {
                 return std::holds_alternative<std::monostate>(work.socket_result);
               }))
      {
        sockets.emplace(socket);
      }
    }
  }

  std::optional<overlapped_write_work> peek_overlapped_write_work(SOCKET socket)
  {
    std::shared_lock<std::shared_mutex> lock(mutex);
    auto data = handles.find(socket);

    if (data == handles.end())
    {
      return std::nullopt;
    }

    if (data->second.overlapped_write_queue.empty())
    {
      return std::nullopt;
    }

    auto pending = stl::find_if(data->second.overlapped_write_queue, [](auto& work) {
      return std::holds_alternative<std::monostate>(work.socket_result);
    });


    if (pending == data->second.overlapped_write_queue.end())
    {
      return std::nullopt;
    }

    return std::make_optional(overlapped_write_work{
      .socket = socket,
      .overlapped = pending->overlapped,
      .data_to_write = pending->data_to_write,
      .data_token = pending->data_token,
      .addr = pending->addr,
      .addr_size = pending->addr_size,
      .flags = pending->flags,
      .target = pending->target,
    });
  }

  std::optional<overlapped_read_work> peek_overlapped_read_work(SOCKET socket)
  {
    std::shared_lock<std::shared_mutex> lock(mutex);

    auto data = handles.find(socket);

    if (data == handles.end())
    {
      return std::nullopt;
    }

    if (data->second.overlapped_read_queue.empty())
    {
      return std::nullopt;
    }

    auto pending = stl::find_if(data->second.overlapped_read_queue, [](auto& work) {
      return std::holds_alternative<std::monostate>(work.socket_result);
    });


    if (pending == data->second.overlapped_read_queue.end())
    {
      return std::nullopt;
    }

    return std::make_optional(overlapped_read_work{
      .socket = socket,
      .overlapped = pending->overlapped,
      .read_targets = pending->read_targets,
      .flags = pending->flags,
      .read_addr_size = pending->target_addr_size ? *pending->target_addr_size : 0,
      .target = pending->target,
    });
  }

  bool overlapped_work_is_complete(WSAOVERLAPPED* overlapped) const
  {
    std::shared_lock<std::shared_mutex> lock(mutex);
    auto socket = overlapped_sockets.find(overlapped);

    if (socket == overlapped_sockets.end())
    {
      return false;
    }
    auto& data = handles.at(socket->second);

    auto read_work = stl::find_if(data.overlapped_read_queue, [overlapped](auto& state) {
      return state.overlapped == overlapped;
    });

    auto write_work = stl::find_if(data.overlapped_write_queue, [overlapped](auto& state) {
      return state.overlapped == overlapped;
    });

    if (read_work != data.overlapped_read_queue.end())
    {
      return !std::holds_alternative<std::monostate>(read_work->socket_result);
    }
    else if (write_work != data.overlapped_write_queue.end())
    {
      return !std::holds_alternative<std::monostate>(write_work->socket_result);
    }

    return false;
  }

  void complete_overlapped_write_work(WSAOVERLAPPED* overlapped, DWORD transferred)
  {
    std::unique_lock<std::shared_mutex> lock(mutex);
    auto socket = overlapped_sockets.find(overlapped);

    if (socket == overlapped_sockets.end())
    {
      return;
    }

    auto& data = handles.at(socket->second);

    auto write_work = stl::find_if(data.overlapped_write_queue, [overlapped](auto& state) {
      return state.overlapped == overlapped;
    });

    if (write_work != data.overlapped_write_queue.end())
    {
      write_work->socket_result = transferred;
    }
  }

  void complete_overlapped_read_work(WSAOVERLAPPED* overlapped, DWORD transferred, DWORD flags, sockaddr addr, int addr_len)
  {
    std::unique_lock<std::shared_mutex> lock(mutex);
    auto socket = overlapped_sockets.find(overlapped);

    if (socket == overlapped_sockets.end())
    {
      return;
    }

    auto& data = handles.at(socket->second);

    auto read_state = stl::find_if(data.overlapped_read_queue, [overlapped](auto& state) {
      return state.overlapped == overlapped;
    });

    if (read_state != data.overlapped_read_queue.end())
    {
      read_state->socket_result = transferred;
      read_state->flags = flags;
      read_state->received_addr = addr;
      read_state->received_addr_size = addr_len;
    }
  }

  void error_overlapped_work(WSAOVERLAPPED* overlapped, int error_code)
  {
    std::unique_lock<std::shared_mutex> lock(mutex);
    auto socket = overlapped_sockets.find(overlapped);

    if (socket == overlapped_sockets.end())
    {
      return;
    }

    auto& data = handles.at(socket->second);

    auto read_work = stl::find_if(data.overlapped_read_queue, [overlapped](auto& state) {
      return state.overlapped == overlapped;
    });

    auto write_work = stl::find_if(data.overlapped_write_queue, [overlapped](auto& state) {
      return state.overlapped == overlapped;
    });

    if (read_work != data.overlapped_read_queue.end())
    {
      read_work->socket_result = error_code;
    }
    else if (write_work != data.overlapped_write_queue.end())
    {
      write_work->socket_result = error_code;
    }
  }

  void remove_overlapped_work(WSAOVERLAPPED* overlapped)
  {
    std::unique_lock<std::shared_mutex> lock(mutex);
    auto socket = overlapped_sockets.find(overlapped);

    if (socket == overlapped_sockets.end())
    {
      return;
    }

    auto& data = handles.at(socket->second);

    auto read_work = stl::find_if(data.overlapped_read_queue, [overlapped](auto& state) {
      return state.overlapped == overlapped;
    });

    auto write_work = stl::find_if(data.overlapped_write_queue, [overlapped](auto& state) {
      return state.overlapped == overlapped;
    });

    if (read_work != data.overlapped_read_queue.end())
    {
      data.overlapped_read_queue.erase(read_work);
    }

    if (write_work != data.overlapped_write_queue.end())
    {
      data.overlapped_write_queue.erase(write_work);
    }

    overlapped_sockets.erase(overlapped);
  }

  std::expected<overlapped_result, std::errc> get_overlapped_result(WSAOVERLAPPED* overlapped)
  {
    std::shared_lock<std::shared_mutex> lock(mutex);
    auto socket = overlapped_sockets.find(overlapped);

    if (socket == overlapped_sockets.end())
    {
      return std::unexpected(std::errc::bad_address);
    }

    auto& data = handles.at(socket->second);

    auto read_work = stl::find_if(data.overlapped_read_queue, [overlapped](auto& state) {
      return state.overlapped == overlapped;
    });

    auto get_result = [overlapped](auto& work) {
      std::expected<DWORD, DWORD> final_result;
      if (std::holds_alternative<DWORD>(work.socket_result))
      {
        final_result = std::get<DWORD>(work.socket_result);
      }
      else
      {
        final_result = std::unexpected(static_cast<DWORD>(std::get<int>(work.socket_result)));
      }
      LPWSAOVERLAPPED_COMPLETION_ROUTINE callback = nullptr;

      if (std::holds_alternative<overlapped_callback>(work.target))
      {
        callback = std::get<overlapped_callback>(work.target).completion_routine;
      }
      return overlapped_result{
        .result = final_result,
        .overlapped = overlapped,
        .flags = work.flags,
        .completion_callback = callback
      };
    };

    if (read_work != data.overlapped_read_queue.end() && !std::holds_alternative<std::monostate>(read_work->socket_result))
    {
      auto common_result = get_result(*read_work);

      common_result.received_addr = read_work->received_addr;
      common_result.received_addr_size = read_work->received_addr_size;
      common_result.target_addr = read_work->target_addr;
      common_result.target_addr_size = read_work->target_addr_size;

      return common_result;
    }

    auto write_work = stl::find_if(data.overlapped_write_queue, [overlapped](auto& state) {
      return state.overlapped == overlapped;
    });

    if (write_work != data.overlapped_write_queue.end() && !std::holds_alternative<std::monostate>(write_work->socket_result))
    {
      return get_result(*write_work);
    }

    return std::unexpected(std::errc::operation_in_progress);
  }

private:
  struct socket_context
  {
    bool is_closed = false;
    // backend sockets are non-blocking,
    // so we have to block by default on the client-side
    bool is_virtual_blocking = true;

    // purely client-side. backends shouldn't know what overlapping is.
    // definitely non-standard bsd.
    overlapped_state is_overlapped = overlapped_state::non_overlapped;

    int socket_type;// likely SOCK_STREAM or SOCK_DGRAM

    bool is_listening = false;

    client_socket_state client_state = client_socket_state::unconnected;

    int select_flags{};
    int io_flags{};

    int event_flags{};
    std::array<int, FD_MAX_EVENTS> event_errors{};
    std::variant<std::monostate, window_target, event_target> target{};

    struct overlapped_write_state
    {
      WSAOVERLAPPED* overlapped;
      std::span<char> data_to_write;
      std::shared_ptr<void> data_token;

      sockaddr addr;
      int addr_size;
      DWORD flags;

      std::variant<std::monostate, overlapped_event, overlapped_callback> target;
      std::variant<std::monostate, DWORD, int> socket_result;
    };

    struct overlapped_read_state
    {
      WSAOVERLAPPED* overlapped;

      DWORD flags;

      std::vector<std::span<char>> read_targets;

      sockaddr received_addr;
      int received_addr_size;

      sockaddr* target_addr;
      int* target_addr_size;
      std::variant<std::monostate, overlapped_event, overlapped_callback> target;
      std::variant<std::monostate, DWORD, int> socket_result;
    };

    std::deque<overlapped_read_state> overlapped_read_queue;
    std::deque<overlapped_write_state> overlapped_write_queue;
  };

  std::map<SOCKET, socket_context> handles;
  std::map<WSAOVERLAPPED*, SOCKET> overlapped_sockets;
  mutable std::shared_mutex mutex;
};

export socket_handle_info& get_socket_handles()
{
  static socket_handle_info info{};
  return info;
}


extern "C" {
hostent* __stdcall siege_gethostbyaddr(const char* addr, int len, int type)
{
  ensure_imports();
  if (!use_custom_backend())
  {
    return imports->gethostbyaddr(addr, len, type);
  }

  if (!addr || type != AF_INET || len < static_cast<int>(sizeof(in_addr)))
  {
    imports->WSASetLastError(WSAEINVAL);
    return nullptr;
  }

  in_addr query{};
  std::memcpy(&query, addr, sizeof(query));

  if (query.S_un.S_addr == imports->htonl(INADDR_LOOPBACK))
  {
    static constexpr std::string_view localhost_name = "localhost";
    static char* localhost_aliases[] = { nullptr };
    static in_addr localhost_addr{};
    static char* localhost_addrs[] = { reinterpret_cast<char*>(&localhost_addr), nullptr };
    static hostent localhost_host{
      .h_aliases = localhost_aliases,
      .h_addrtype = AF_INET,
      .h_length = sizeof(in_addr),
      .h_addr_list = localhost_addrs,
    };

    localhost_addr.S_un.S_addr = imports->htonl(INADDR_LOOPBACK);
    localhost_host.h_name = const_cast<char*>(localhost_name.data());
    imports->WSASetLastError(0);
    return &localhost_host;
  }

  std::array<char, 256> hostname{};
  if (siege_gethostname(hostname.data(), static_cast<int>(hostname.size())) != 0)
  {
    return nullptr;
  }

  auto* self = siege_gethostbyname(hostname.data());
  if (!self || self->h_addrtype != AF_INET || self->h_length < static_cast<short>(sizeof(in_addr)) || !self->h_addr_list)
  {
    imports->WSASetLastError(WSAHOST_NOT_FOUND);
    return nullptr;
  }

  for (auto** entry = self->h_addr_list; *entry; ++entry)
  {
    in_addr candidate{};
    std::memcpy(&candidate, *entry, sizeof(candidate));
    if (candidate.S_un.S_addr == query.S_un.S_addr)
    {
      imports->WSASetLastError(0);
      return self;
    }
  }

  imports->WSASetLastError(WSAHOST_NOT_FOUND);
  return nullptr;
}

HANDLE __stdcall siege_WSAAsyncGetHostByName(HWND window, u_int message, const char* name, char* buffer, int buffer_length)
{
  get_log() << "siege_WSAAsyncGetHostByName.\n";
  if (!use_custom_backend())
  {
    return imports->WSAAsyncGetHostByName(window, message, name, buffer, buffer_length);
  }

  if (!name)
  {
    imports->WSASetLastError(WSAEFAULT);
    return nullptr;
  }

  if (!buffer)
  {
    imports->WSASetLastError(WSAEFAULT);
    return nullptr;
  }

  if (buffer_length <= 0)
  {
    imports->WSASetLastError(WSAENOBUFS);
    return nullptr;
  }

  if (buffer_length < MAXGETHOSTSTRUCT)
  {
    imports->WSASetLastError(WSAENOBUFS);
    return nullptr;
  }

  HANDLE cancel = ::CreateEventW(/*lpEventAttributes*/ nullptr, /*bManualReset*/ TRUE, /*bInitialState*/ FALSE, /*lpName*/ nullptr);

  if (!cancel)
  {
    imports->WSASetLastError(WSAENETDOWN);
    return nullptr;
  }

  auto post_message = ::IsWindowUnicode(window) ? ::PostMessageW : ::PostMessageA;

  auto started = win32::queue_user_work_item([window, message, name = std::string{ name }, buffer = std::span(buffer, buffer_length), cancel, post_message]() {
    auto auto_close = std::shared_ptr<void>{
      nullptr, [cancel](...) {
        ::CloseHandle(cancel);
      }
    };

    if (::WaitForSingleObject(cancel, 0) == WAIT_OBJECT_0)
    {
      return;
    }

    auto result = siege_gethostbyname(name.c_str());

    if (::WaitForSingleObject(cancel, 0) == WAIT_OBJECT_0)
    {
      return;
    }

    if (result)
    {
      auto* packed = new (buffer.data()) packed_hostent{ *result };

      post_message(window, message, (WPARAM)cancel, 0);
    }
    else
    {
      post_message(window, message, (WPARAM)cancel, MAKELPARAM(0, imports->WSAGetLastError()));
    }
  });


  if (!started)
  {
    imports->WSASetLastError(WSAENETDOWN);
    ::CloseHandle(cancel);
    return nullptr;
  }

  return cancel;
}

HANDLE __stdcall siege_WSAAsyncGetHostByAddr(HWND window, u_int message, const char* addr, int len, int type, char* buffer, int buffer_length)
{
  if (!use_custom_backend())
  {
    return imports->WSAAsyncGetHostByAddr(window, message, addr, len, type, buffer, buffer_length);
  }

  if (!addr)
  {
    imports->WSASetLastError(WSAEFAULT);
    return nullptr;
  }

  if (!buffer)
  {
    imports->WSASetLastError(WSAEFAULT);
    return nullptr;
  }

  if (buffer_length < MAXGETHOSTSTRUCT)
  {
    imports->WSASetLastError(WSAENOBUFS);
    return nullptr;
  }

  HANDLE cancel = ::CreateEventW(/*lpEventAttributes*/ nullptr, /*bManualReset*/ TRUE, /*bInitialState*/ FALSE, /*lpName*/ nullptr);

  if (!cancel)
  {
    imports->WSASetLastError(WSAENETDOWN);
    return nullptr;
  }

  auto post_message = ::IsWindowUnicode(window) ? ::PostMessageW : ::PostMessageA;

  auto started = win32::queue_user_work_item([window, message, addr = std::string(addr, addr + std::max(len, 0)), len, type, buffer = std::span(buffer, buffer_length), cancel, post_message]() {
    auto auto_close = std::shared_ptr<void>{
      nullptr, [cancel](...) {
        ::CloseHandle(cancel);
      }
    };

    if (::WaitForSingleObject(cancel, 0) == WAIT_OBJECT_0)
    {
      return;
    }

    auto* result = siege_gethostbyaddr(addr.data(), len, type);

    if (::WaitForSingleObject(cancel, 0) == WAIT_OBJECT_0)
    {
      return;
    }

    if (result)
    {
      new (buffer.data()) packed_hostent{ *result };
      post_message(window, message, (WPARAM)cancel, 0);
    }
    else
    {
      post_message(window, message, (WPARAM)cancel, MAKELPARAM(0, imports->WSAGetLastError()));
    }
  });

  if (!started)
  {
    imports->WSASetLastError(WSAENETDOWN);
    ::CloseHandle(cancel);
    return nullptr;
  }

  return cancel;
}

HANDLE __stdcall siege_WSAAsyncGetProtoByNumber(HWND window, u_int message, int number, char* buffer, int buffer_length)
{
  if (!use_custom_backend())
  {
    return imports->WSAAsyncGetProtoByNumber(window, message, number, buffer, buffer_length);
  }

  if (!buffer)
  {
    imports->WSASetLastError(WSAEFAULT);
    return nullptr;
  }

  if (buffer_length < MAXGETHOSTSTRUCT)
  {
    imports->WSASetLastError(WSAENOBUFS);
    return nullptr;
  }

  HANDLE cancel = ::CreateEventW(/*lpEventAttributes*/ nullptr, /*bManualReset*/ TRUE, /*bInitialState*/ FALSE, /*lpName*/ nullptr);

  if (!cancel)
  {
    imports->WSASetLastError(WSAENETDOWN);
    return nullptr;
  }

  auto post_message = ::IsWindowUnicode(window) ? ::PostMessageW : ::PostMessageA;

  auto started = win32::queue_user_work_item([window, message, number, buffer = std::span(buffer, buffer_length), cancel, post_message]() {
    auto auto_close = std::shared_ptr<void>{
      nullptr, [cancel](...) {
        ::CloseHandle(cancel);
      }
    };

    if (::WaitForSingleObject(cancel, 0) == WAIT_OBJECT_0)
    {
      return;
    }

    auto* result = siege_getprotobynumber(number);

    if (::WaitForSingleObject(cancel, 0) == WAIT_OBJECT_0)
    {
      return;
    }

    if (result)
    {
      new (buffer.data()) packed_protoent{ *result };
      post_message(window, message, (WPARAM)cancel, 0);
    }
    else
    {
      post_message(window, message, (WPARAM)cancel, MAKELPARAM(0, imports->WSAGetLastError()));
    }
  });

  if (!started)
  {
    imports->WSASetLastError(WSAENETDOWN);
    ::CloseHandle(cancel);
    return nullptr;
  }

  return cancel;
}

HANDLE __stdcall siege_WSAAsyncGetProtoByName(HWND window, u_int message, const char* name, char* buffer, int buffer_length)
{
  if (!use_custom_backend())
  {
    return imports->WSAAsyncGetProtoByName(window, message, name, buffer, buffer_length);
  }

  if (!name)
  {
    imports->WSASetLastError(WSAEFAULT);
    return nullptr;
  }

  if (!buffer)
  {
    imports->WSASetLastError(WSAEFAULT);
    return nullptr;
  }

  if (buffer_length < MAXGETHOSTSTRUCT)
  {
    imports->WSASetLastError(WSAENOBUFS);
    return nullptr;
  }

  HANDLE cancel = ::CreateEventW(/*lpEventAttributes*/ nullptr, /*bManualReset*/ TRUE, /*bInitialState*/ FALSE, /*lpName*/ nullptr);

  if (!cancel)
  {
    imports->WSASetLastError(WSAENETDOWN);
    return nullptr;
  }

  auto post_message = ::IsWindowUnicode(window) ? ::PostMessageW : ::PostMessageA;

  auto started = win32::queue_user_work_item([window, message, name = std::string{ name }, buffer = std::span(buffer, buffer_length), cancel, post_message]() {
    auto auto_close = std::shared_ptr<void>{
      nullptr, [cancel](...) {
        ::CloseHandle(cancel);
      }
    };

    if (::WaitForSingleObject(cancel, 0) == WAIT_OBJECT_0)
    {
      return;
    }

    auto* result = siege_getprotobyname(name.c_str());

    if (::WaitForSingleObject(cancel, 0) == WAIT_OBJECT_0)
    {
      return;
    }

    if (result)
    {
      new (buffer.data()) packed_protoent{ *result };
      post_message(window, message, (WPARAM)cancel, 0);
    }
    else
    {
      post_message(window, message, (WPARAM)cancel, MAKELPARAM(0, imports->WSAGetLastError()));
    }
  });

  if (!started)
  {
    imports->WSASetLastError(WSAENETDOWN);
    ::CloseHandle(cancel);
    return nullptr;
  }

  return cancel;
}

HANDLE __stdcall siege_WSAAsyncGetServByPort(HWND window, u_int message, int port, const char* proto, char* buffer, int buffer_length)
{
  if (!use_custom_backend())
  {
    return imports->WSAAsyncGetServByPort(window, message, port, proto, buffer, buffer_length);
  }

  if (!buffer)
  {
    imports->WSASetLastError(WSAEFAULT);
    return nullptr;
  }

  if (buffer_length < MAXGETHOSTSTRUCT)
  {
    imports->WSASetLastError(WSAENOBUFS);
    return nullptr;
  }

  HANDLE cancel = ::CreateEventW(/*lpEventAttributes*/ nullptr, /*bManualReset*/ TRUE, /*bInitialState*/ FALSE, /*lpName*/ nullptr);

  if (!cancel)
  {
    imports->WSASetLastError(WSAENETDOWN);
    return nullptr;
  }

  auto post_message = ::IsWindowUnicode(window) ? ::PostMessageW : ::PostMessageA;

  auto started = win32::queue_user_work_item([window, message, port, proto = proto ? std::optional<std::string>{ proto } : std::nullopt, buffer = std::span(buffer, buffer_length), cancel, post_message]() {
    auto auto_close = std::shared_ptr<void>{
      nullptr, [cancel](...) {
        ::CloseHandle(cancel);
      }
    };

    if (::WaitForSingleObject(cancel, 0) == WAIT_OBJECT_0)
    {
      return;
    }

    auto* result = siege_getservbyport(port, proto ? proto->c_str() : nullptr);

    if (::WaitForSingleObject(cancel, 0) == WAIT_OBJECT_0)
    {
      return;
    }

    if (result)
    {
      new (buffer.data()) packed_servent{ *result };
      post_message(window, message, (WPARAM)cancel, 0);
    }
    else
    {
      post_message(window, message, (WPARAM)cancel, MAKELPARAM(0, imports->WSAGetLastError()));
    }
  });

  if (!started)
  {
    imports->WSASetLastError(WSAENETDOWN);
    ::CloseHandle(cancel);
    return nullptr;
  }

  return cancel;
}

HANDLE __stdcall siege_WSAAsyncGetServByName(HWND window, u_int message, const char* name, const char* proto, char* buffer, int buffer_length)
{
  if (!use_custom_backend())
  {
    return imports->WSAAsyncGetServByName(window, message, name, proto, buffer, buffer_length);
  }

  if (!name)
  {
    imports->WSASetLastError(WSAEFAULT);
    return nullptr;
  }

  if (!buffer)
  {
    imports->WSASetLastError(WSAEFAULT);
    return nullptr;
  }

  if (buffer_length < MAXGETHOSTSTRUCT)
  {
    imports->WSASetLastError(WSAENOBUFS);
    return nullptr;
  }

  HANDLE cancel = ::CreateEventW(/*lpEventAttributes*/ nullptr, /*bManualReset*/ TRUE, /*bInitialState*/ FALSE, /*lpName*/ nullptr);

  if (!cancel)
  {
    imports->WSASetLastError(WSAENETDOWN);
    return nullptr;
  }

  auto post_message = ::IsWindowUnicode(window) ? ::PostMessageW : ::PostMessageA;

  auto started = win32::queue_user_work_item([window, message, name = std::string{ name }, proto = proto ? std::optional<std::string>{ proto } : std::nullopt, buffer = std::span(buffer, buffer_length), cancel, post_message]() {
    auto auto_close = std::shared_ptr<void>{
      nullptr, [cancel](...) {
        ::CloseHandle(cancel);
      }
    };

    if (::WaitForSingleObject(cancel, 0) == WAIT_OBJECT_0)
    {
      return;
    }

    auto* result = siege_getservbyname(name.c_str(), proto ? proto->c_str() : nullptr);

    if (::WaitForSingleObject(cancel, 0) == WAIT_OBJECT_0)
    {
      return;
    }

    if (result)
    {
      new (buffer.data()) packed_servent{ *result };
      post_message(window, message, (WPARAM)cancel, 0);
    }
    else
    {
      post_message(window, message, (WPARAM)cancel, MAKELPARAM(0, imports->WSAGetLastError()));
    }
  });

  if (!started)
  {
    imports->WSASetLastError(WSAENETDOWN);
    ::CloseHandle(cancel);
    return nullptr;
  }

  return cancel;
}

auto __stdcall siege_WSACancelAsyncRequest(HANDLE request)
{
  if (!use_custom_backend())
  {
    return imports->WSACancelAsyncRequest(request);
  }

  get_log() << "siege_WSACancelAsyncRequest";

  if (!request || !::SetEvent(request))
  {
    imports->WSASetLastError(WSAEINVAL);
    return SOCKET_ERROR;
  }

  return 0;
}

int __stdcall siege_recv(SOCKET ws, char* buf, int len, int flags) noexcept
{
  if (use_custom_backend())
  {
    return siege_recvfrom(ws, buf, len, flags, nullptr, nullptr);
  }

  return imports->recv(ws, buf, len, flags);
}


int __stdcall siege_send(SOCKET ws, const char* buf, int len, int flags) noexcept
{
  if (use_custom_backend())
  {
    return siege_sendto(ws, buf, len, flags, nullptr, 0);
  }

  return imports->send(ws, buf, len, flags);
}

#ifdef USE_WINSOCK2
SOCKET __stdcall siege_WSASocketW(int af, int type, int protocol, LPWSAPROTOCOL_INFOW lpProtocolInfo, GROUP g, DWORD dwFlags)
{
  get_log() << "siege_WSASocketW " << '\n';
  if (!use_custom_backend())
  {
    return imports->WSASocketW(af, type, protocol, lpProtocolInfo, g, dwFlags);
  }

  if (lpProtocolInfo)
  {
    imports->WSASetLastError(WSAEPROVIDERFAILEDINIT);
    return INVALID_SOCKET;
  }

  constexpr static auto invalid_flags = std::array<DWORD, 4>{ { WSA_FLAG_MULTIPOINT_C_ROOT, WSA_FLAG_MULTIPOINT_C_LEAF, WSA_FLAG_MULTIPOINT_D_ROOT, WSA_FLAG_MULTIPOINT_D_LEAF } };

  for (auto flag : invalid_flags)
  {
    if (dwFlags & flag)
    {
      imports->WSASetLastError(WSAEINVAL);
      return INVALID_SOCKET;
    }
  }

  if (g != 0)
  {
    imports->WSASetLastError(WSAEINVAL);
    return INVALID_SOCKET;
  }

  auto result = siege_socket(af, type, protocol);

  if (result != INVALID_SOCKET)
  {
    if (dwFlags & WSA_FLAG_OVERLAPPED)
    {
      get_overlapped_worker(worker_action::restart_if_stopped);
      get_socket_handles().set_overlapped(result, socket_handle_info::overlapped_state::overlapped);
    }
    else
    {
      get_socket_handles().set_overlapped(result, socket_handle_info::overlapped_state::non_overlapped);
    }
  }

  return result;
}

SOCKET __stdcall siege_WSASocketA(int af, int type, int protocol, LPWSAPROTOCOL_INFOA lpProtocolInfo, GROUP g, DWORD dwFlags)
{
  if (!use_custom_backend())
  {
    return imports->WSASocketA(af, type, protocol, lpProtocolInfo, g, dwFlags);
  }

  get_log() << "siege_WSASocketA " << '\n';

  if (lpProtocolInfo)
  {
    imports->WSASetLastError(WSAEPROVIDERFAILEDINIT);
    return INVALID_SOCKET;
  }

  return siege_WSASocketW(af, type, protocol, nullptr, g, dwFlags);
}

int __stdcall siege_WSAIoctl(SOCKET s, DWORD controlCode, LPVOID inBuffer, DWORD inBufferCount, LPVOID outBuffer, DWORD outBufferCount, LPDWORD bytesReturned, LPWSAOVERLAPPED overlapped, LPWSAOVERLAPPED_COMPLETION_ROUTINE completionRoutine) noexcept
{
  if (!use_custom_backend())
  {
    return imports->WSAIoctl(s, controlCode, inBuffer, inBufferCount, outBuffer, outBufferCount, bytesReturned, overlapped, completionRoutine);
  }

  if ((bool)get_socket_handles().is_overlapped(s) && (overlapped || completionRoutine))
  {
    WSASetLastError(WSAEOPNOTSUPP);
    return SOCKET_ERROR;
  }

  if (controlCode == SIO_UDP_CONNRESET)
  {
    if (!inBuffer || inBufferCount < sizeof(BOOL))
    {
      imports->WSASetLastError(WSAEINVAL);
      return SOCKET_ERROR;
    }

    if (bytesReturned)
    {
      *bytesReturned = 0;
    }

    imports->WSASetLastError(0);
    return 0;
  }

  if (controlCode == SIO_GET_INTERFACE_LIST)
  {
    if (!outBuffer)
    {
      imports->WSASetLastError(WSAEFAULT);
      return SOCKET_ERROR;
    }

    std::array<char, 256> hostname{};
    if (siege_gethostname(hostname.data(), static_cast<int>(hostname.size())) != 0)
    {
      return SOCKET_ERROR;
    }

    auto* host = siege_gethostbyname(hostname.data());
    if (!host || host->h_addrtype != AF_INET || host->h_length < static_cast<short>(sizeof(in_addr)) || !host->h_addr_list || !host->h_addr_list[0])
    {
      imports->WSASetLastError(WSAEINVAL);
      return SOCKET_ERROR;
    }

    std::size_t count = 0;
    for (auto** addr = host->h_addr_list; *addr; ++addr)
    {
      ++count;
    }

    auto needed = count * sizeof(INTERFACE_INFO);
    if (outBufferCount < needed)
    {
      imports->WSASetLastError(WSAEFAULT);
      return SOCKET_ERROR;
    }

    auto* infos = reinterpret_cast<INTERFACE_INFO*>(outBuffer);
    std::memset(infos, 0, needed);

    in_addr netmask{};
    netmask.S_un.S_addr = imports->htonl(0xFFFFFF00);

    for (std::size_t i = 0; i < count; ++i)
    {
      in_addr address{};
      std::memcpy(&address, host->h_addr_list[i], sizeof(address));

      in_addr broadcast{};
      broadcast.S_un.S_addr = address.S_un.S_addr | ~netmask.S_un.S_addr;

      infos[i].iiFlags = IFF_UP | IFF_BROADCAST | IFF_MULTICAST;
      infos[i].iiAddress.AddressIn.sin_family = AF_INET;
      infos[i].iiAddress.AddressIn.sin_addr = address;
      infos[i].iiBroadcastAddress.AddressIn.sin_family = AF_INET;
      infos[i].iiBroadcastAddress.AddressIn.sin_addr = broadcast;
      infos[i].iiNetmask.AddressIn.sin_family = AF_INET;
      infos[i].iiNetmask.AddressIn.sin_addr = netmask;
    }

    if (bytesReturned)
    {
      *bytesReturned = static_cast<DWORD>(needed);
    }

    imports->WSASetLastError(0);
    return 0;
  }

  if (!(controlCode == FIONBIO || controlCode == FIONREAD))
  {
    imports->WSASetLastError(WSAEOPNOTSUPP);
    return SOCKET_ERROR;
  }

  if (inBufferCount > 4)
  {
    imports->WSASetLastError(WSAEINVAL);
    return SOCKET_ERROR;
  }

  u_long temp{};

  if (inBuffer && inBufferCount <= 4)
  {
    std::memcpy(&temp, inBuffer, inBufferCount);
  }
  auto result = siege_ioctlsocket(s, controlCode, &temp);

  auto out_size = std::clamp<std::size_t>(outBufferCount, 0, sizeof(temp));

  if (result == 0 && outBuffer && outBufferCount >= 4)
  {
    std::memcpy(outBuffer, &temp, out_size);
  }

  if (result == 0 && bytesReturned)
  {
    *bytesReturned = static_cast<DWORD>(out_size);
  }

  return result;
}

SOCKET __stdcall siege_WSAAccept(SOCKET s, sockaddr* addr, LPINT addrlen, LPCONDITIONPROC lpfnCondition, DWORD_PTR dwCallbackData)
{

  if (!use_custom_backend())
  {

    return imports->WSAAccept(s, addr, addrlen, lpfnCondition, dwCallbackData);
  }

  if (lpfnCondition)
  {
    imports->WSASetLastError(WSAEINVAL);
    return INVALID_SOCKET;
  }

  return siege_accept(s, addr, addrlen);
}

// This and freeaddrinfo needed by AMD's open GL driver for the RPC case
struct siege_addrinfo
{
  addrinfo info;
  sockaddr_in address;
  std::array<char, 256> canonname;
};

void __stdcall siege_freeaddrinfo(addrinfo* results);

int __stdcall siege_getaddrinfo(const char* node_name, const char* service_name, const addrinfo* hints, addrinfo** results)
{
  ensure_imports();
  if (!use_custom_backend())
  {
    return imports->getaddrinfo(node_name, service_name, hints, results);
  }

  if (!results)
  {
    imports->WSASetLastError(WSAEINVAL);
    return WSAEINVAL;
  }

  *results = nullptr;

  const bool has_node = node_name && *node_name;
  const bool has_service = service_name && *service_name;
  if (!has_node && !has_service)
  {
    imports->WSASetLastError(WSAHOST_NOT_FOUND);
    return WSAHOST_NOT_FOUND;
  }

  int socktype = hints ? hints->ai_socktype : 0;
  int protocol = hints ? hints->ai_protocol : 0;
  const int flags = hints ? hints->ai_flags : 0;

  if (hints && hints->ai_family != AF_UNSPEC && hints->ai_family != AF_INET)
  {
    imports->WSASetLastError(WSAEAFNOSUPPORT);
    return WSAEAFNOSUPPORT;
  }

  u_short port = 0;
  if (has_service)
  {
    const bool numeric_service = (flags & AI_NUMERICSERV) || std::all_of(service_name, service_name + std::strlen(service_name), [](unsigned char c) {
      return std::isdigit(c) != 0;
    });

    if (numeric_service)
    {
      port = imports->htons(static_cast<u_short>(std::atoi(service_name)));
    }
    else
    {
      const char* proto_name = nullptr;
      if (socktype == SOCK_STREAM || protocol == IPPROTO_TCP)
      {
        proto_name = "tcp";
      }
      else if (socktype == SOCK_DGRAM || protocol == IPPROTO_UDP)
      {
        proto_name = "udp";
      }

      auto* service = siege_getservbyname(service_name, proto_name);
      if (!service)
      {
        imports->WSASetLastError(WSANO_DATA);
        return WSANO_DATA;
      }

      port = service->s_port;
      if (socktype == 0)
      {
        socktype = _stricmp(service->s_proto, "udp") == 0 ? SOCK_DGRAM : SOCK_STREAM;
      }
      if (protocol == 0)
      {
        if (auto* proto = siege_getprotobyname(service->s_proto))
        {
          protocol = proto->p_proto;
        }
      }
    }
  }

  addrinfo* tail = nullptr;
  auto append_result = [&](in_addr address, const char* canonname) -> int {
    auto* node = new (std::nothrow) siege_addrinfo{};
    if (!node)
    {
      siege_freeaddrinfo(*results);
      *results = nullptr;
      imports->WSASetLastError(WSA_NOT_ENOUGH_MEMORY);
      return WSA_NOT_ENOUGH_MEMORY;
    }

    node->address = sockaddr_in{
      .sin_family = AF_INET,
      .sin_port = port,
      .sin_addr = address,
    };
    node->info = addrinfo{
      .ai_family = AF_INET,
      .ai_socktype = socktype,
      .ai_protocol = protocol,
      .ai_addrlen = sizeof(sockaddr_in),
      .ai_addr = reinterpret_cast<sockaddr*>(&node->address),
    };

    if ((flags & AI_CANONNAME) && canonname && !*results)
    {
      std::strncpy(node->canonname.data(), canonname, node->canonname.size() - 1);
      node->canonname.back() = '\0';
      node->info.ai_canonname = node->canonname.data();
    }

    if (!*results)
    {
      *results = &node->info;
    }
    else
    {
      tail->ai_next = &node->info;
    }
    tail = &node->info;
    return 0;
  };

  if (!has_node)
  {
    in_addr address{};
    address.S_un.S_addr = imports->htonl((flags & AI_PASSIVE) ? INADDR_ANY : INADDR_LOOPBACK);
    if (auto error = append_result(address, nullptr); error != 0)
    {
      return error;
    }
    imports->WSASetLastError(0);
    return 0;
  }

  in_addr numeric{};
  numeric.S_un.S_addr = imports->inet_addr(node_name);
  const bool parsed = numeric.S_un.S_addr != INADDR_NONE || std::string_view{ node_name } == "255.255.255.255";

  if ((flags & AI_NUMERICHOST) || parsed)
  {
    if (!parsed)
    {
      imports->WSASetLastError(WSAHOST_NOT_FOUND);
      return WSAHOST_NOT_FOUND;
    }
    if (auto error = append_result(numeric, nullptr); error != 0)
    {
      return error;
    }
    imports->WSASetLastError(0);
    return 0;
  }

  auto* host = siege_gethostbyname(node_name);
  if (!host || host->h_addrtype != AF_INET || host->h_length < static_cast<short>(sizeof(in_addr)) || !host->h_addr_list || !host->h_addr_list[0])
  {
    imports->WSASetLastError(WSAHOST_NOT_FOUND);
    return WSAHOST_NOT_FOUND;
  }

  for (auto** entry = host->h_addr_list; *entry; ++entry)
  {
    in_addr address{};
    std::memcpy(&address, *entry, sizeof(address));
    if (auto error = append_result(address, host->h_name); error != 0)
    {
      return error;
    }
  }

  imports->WSASetLastError(0);
  return 0;
}

void __stdcall siege_freeaddrinfo(addrinfo* results)
{
  ensure_imports();
  if (!use_custom_backend())
  {
    imports->freeaddrinfo(results);
    return;
  }

  while (results)
  {
    auto* node = reinterpret_cast<siege_addrinfo*>(results);
    results = results->ai_next;
    delete node;
  }
}

auto __stdcall siege_inet_ntop(int family, const void* addr, char* buf, std::size_t buf_size)
{
  ensure_imports();
  get_log() << "siege_inet_ntop " << '\n';
  return imports->inet_ntop(family, addr, buf, buf_size);
}


auto __stdcall siege_WSAGetOverlappedResult(SOCKET socket, WSAOVERLAPPED* overlapped, DWORD* transferred, BOOL wait, DWORD* flags)
{
  if (!use_custom_backend())
  {
    return imports->WSAGetOverlappedResult(socket, overlapped, transferred, wait, flags);
  }

  if (!overlapped || !transferred || !flags)
  {
    imports->WSASetLastError(WSA_INVALID_PARAMETER);
    return FALSE;
  }

  auto result = get_socket_handles().get_overlapped_result(overlapped);

  if (!result && result.error() == std::errc::bad_address)
  {
    imports->WSASetLastError(WSA_INVALID_PARAMETER);
    return FALSE;
  }

  if (!result && !wait && result.error() == std::errc::operation_in_progress)
  {
    imports->WSASetLastError(WSA_IO_INCOMPLETE);
    return FALSE;
  }

  if (wait && !result)
  {
    auto wait_result = ::WaitForSingleObject(overlapped->hEvent, INFINITE);

    if (wait_result == WAIT_FAILED)
    {
      imports->WSASetLastError(WSA_INVALID_PARAMETER);
      return FALSE;
    }
  }

  result = get_socket_handles().get_overlapped_result(overlapped);

  if (!result)
  {
    imports->WSASetLastError(WSA_OPERATION_ABORTED);
    return FALSE;
  }

  if (!result->result)
  {
    imports->WSASetLastError(result->result.error());
    get_socket_handles().remove_overlapped_work(overlapped);
    return FALSE;
  }

  *transferred = *result->result;
  *flags = result->flags;

  if (result->target_addr && result->target_addr_size && result->received_addr_size > 0)
  {
    auto real_size = std::clamp<int>(result->received_addr_size, 0, *result->target_addr_size);
    std::memcpy(result->target_addr, &result->received_addr, real_size);
    *result->target_addr_size = real_size;
  }

  get_socket_handles().remove_overlapped_work(overlapped);

  return TRUE;
}

auto __stdcall siege_WSARecvFrom(SOCKET socket, WSABUF* buffers, DWORD buffer_count, DWORD* bytes_received, DWORD* flags, sockaddr* from, INT* from_len, OVERLAPPED* overlapped, LPWSAOVERLAPPED_COMPLETION_ROUTINE completion_handler) noexcept
{
  if (!use_custom_backend())
  {
    return imports->WSARecvFrom(socket, buffers, buffer_count, bytes_received, flags, from, from_len, overlapped, completion_handler);
  }

  get_log() << "siege_WSARecvFrom called.\n";

  if (!flags)
  {
    imports->WSASetLastError(WSAEFAULT);
    return SOCKET_ERROR;
  }

  if (*flags & MSG_PARTIAL)
  {
    get_log() << "siege_WSARecvFrom MSG_PARTIAL requested. Not supported.";
    imports->WSASetLastError(WSAEOPNOTSUPP);
    return SOCKET_ERROR;
  }

  if (!buffers)
  {
    imports->WSASetLastError(WSAEFAULT);
    return SOCKET_ERROR;
  }

  for (auto i = 0; i < buffer_count; ++i)
  {
    if (!buffers[i].buf && buffers[i].len > 0)
    {
      imports->WSASetLastError(WSAEINVAL);
      return SOCKET_ERROR;
    }
  }

  bool should_queue_overlapped = (bool)get_socket_handles().is_overlapped(socket) && (overlapped || completion_handler);

  if (!bytes_received && !should_queue_overlapped)
  {
    imports->WSASetLastError(WSAEINVAL);
    return SOCKET_ERROR;
  }

  constexpr static auto max_int = static_cast<std::size_t>(std::numeric_limits<int>::max());
  std::size_t size = 0;
  for (auto i = 0; i < buffer_count; ++i)
  {
    auto len = static_cast<std::size_t>(buffers[i].len);

    if (len > max_int - size)
    {
      imports->WSASetLastError(WSAEMSGSIZE);
      return SOCKET_ERROR;
    }

    size += len;
  }


  if (should_queue_overlapped)
  {
    HANDLE thread_handle = nullptr;
    if (completion_handler)
    {
      thread_handle = ::OpenThread(THREAD_SET_CONTEXT, FALSE, ::GetCurrentThreadId());

      if (!thread_handle)
      {
        imports->WSASetLastError(WSAEINVAL);
        return SOCKET_ERROR;
      }
    }

    std::vector<std::span<char>> spans;

    spans.reserve(buffer_count);

    for (auto i = 0; i < buffer_count; ++i)
    {
      if (buffers[i].len == 0)
      {
        continue;
      }
      spans.emplace_back(buffers[i].buf, buffers[i].len);
    }

    get_overlapped_worker(worker_action::restart_if_stopped);
    get_socket_handles().queue_overlapped_read_work(socket_handle_info::overlapped_read_params{
      .socket = socket,
      .overlapped = overlapped,
      .read_targets = std::move(spans),
      .addr = from,
      .addr_size = from_len,
      .flags = *flags,
      .thread_handle = thread_handle,
      .callback = completion_handler });

    imports->WSASetLastError(WSA_IO_PENDING);
    return SOCKET_ERROR;
  }

  struct span_pair
  {
    std::span<char> from_buffer;
    std::span<char> to_param;
  };

  // thread_local for memory caching
  thread_local std::vector<char> temp_buffer;
  thread_local std::vector<span_pair> span_buffer;

  span_buffer.reserve(buffer_count);
  span_buffer.clear();

  temp_buffer.resize(size);

  auto begin = temp_buffer.begin();

  for (auto i = 0; i < buffer_count; ++i)
  {
    if (buffers[i].len == 0)
    {
      continue;
    }

    span_pair& pair = span_buffer.emplace_back();
    pair.to_param = std::span{ buffers[i].buf, buffers[i].len };
    pair.from_buffer = std::span{ begin, buffers[i].len };

    if (begin + buffers[i].len > temp_buffer.end())
    {
      break;
    }
    std::advance(begin, buffers[i].len);
  }

  auto received_size = siege_recvfrom(socket, temp_buffer.data(), static_cast<int>(temp_buffer.size()), *flags, from, from_len);

  if (received_size == SOCKET_ERROR)
  {
    return SOCKET_ERROR;
  }

  *bytes_received = static_cast<DWORD>(received_size);

  if (span_buffer.empty())
  {
    return 0;
  }

  std::size_t remaining = static_cast<std::size_t>(received_size);
  for (auto& pair : span_buffer)
  {
    auto to_copy = std::min(pair.to_param.size(), remaining);

    if (to_copy == 0)
    {
      continue;
    }

    assert(pair.to_param.size() == pair.from_buffer.size());
    std::memcpy(pair.to_param.data(), pair.from_buffer.data(), to_copy);
    remaining -= to_copy;
  }

  return 0;
}

auto __stdcall siege_WSARecv(SOCKET ws, LPWSABUF buffers, DWORD bufferCount, LPDWORD numberOfBytesRecvd, LPDWORD flags, LPWSAOVERLAPPED lpOverlapped, LPWSAOVERLAPPED_COMPLETION_ROUTINE completionRoutine)
{
  if (!use_custom_backend())
  {
    return imports->WSARecv(ws, buffers, bufferCount, numberOfBytesRecvd, flags, lpOverlapped, completionRoutine);
  }

  return siege_WSARecvFrom(ws, buffers, bufferCount, numberOfBytesRecvd, flags, nullptr, 0, lpOverlapped, completionRoutine);
}

auto __stdcall siege_WSASendTo(SOCKET socket, WSABUF* buffers, DWORD buffer_count, DWORD* bytes_sent, DWORD flags, const sockaddr* to, int len, OVERLAPPED* overlapped, LPWSAOVERLAPPED_COMPLETION_ROUTINE completion_handler) noexcept
{
  if (!use_custom_backend())
  {
    return imports->WSASendTo(socket, buffers, buffer_count, bytes_sent, flags, to, len, overlapped, completion_handler);
  }

  if (flags & MSG_PARTIAL)
  {
    get_log() << "siege_WSASendTo MSG_PARTIAL requested. Not supported.";
    imports->WSASetLastError(WSAEOPNOTSUPP);
    return SOCKET_ERROR;
  }

  if (!buffers)
  {
    imports->WSASetLastError(WSAEINVAL);
    return SOCKET_ERROR;
  }

  for (auto i = 0; i < buffer_count; ++i)
  {
    if (!buffers[i].buf && buffers[i].len > 0)
    {
      imports->WSASetLastError(WSAEINVAL);
      return SOCKET_ERROR;
    }
  }

  bool should_queue_overlapped = (bool)get_socket_handles().is_overlapped(socket) && (overlapped || completion_handler);

  if (!bytes_sent && !should_queue_overlapped)
  {
    imports->WSASetLastError(WSAEINVAL);
    return SOCKET_ERROR;
  }

  constexpr static auto max_int = static_cast<std::size_t>(std::numeric_limits<int>::max());
  std::size_t size = 0;
  for (auto i = 0; i < buffer_count; ++i)
  {
    auto len = static_cast<std::size_t>(buffers[i].len);

    if (len > max_int - size)
    {
      imports->WSASetLastError(WSAEMSGSIZE);
      return SOCKET_ERROR;
    }

    size += len;
  }

  thread_local std::vector<char> temp_buffer;
  temp_buffer.reserve(size);
  temp_buffer.resize(0);

  for (auto i = 0; i < buffer_count; ++i)
  {
    if (!buffers[i].buf || buffers[i].len == 0)
    {
      continue;
    }

    temp_buffer.insert(temp_buffer.end(), buffers[i].buf, buffers[i].buf + buffers[i].len);
  }

  if (should_queue_overlapped)
  {
    HANDLE thread_handle = nullptr;
    if (completion_handler)
    {
      thread_handle = ::OpenThread(THREAD_SET_CONTEXT, FALSE, ::GetCurrentThreadId());

      if (!thread_handle)
      {
        imports->WSASetLastError(WSAEINVAL);
        return SOCKET_ERROR;
      }
    }

    sockaddr temp{};
    int temp_len = std::clamp<int>(len, 0, static_cast<int>(sizeof(sockaddr)));

    if (to)
    {
      std::memcpy(&temp, to, temp_len);
    }
    else
    {
      temp_len = sizeof(sockaddr);
      auto peer_result = siege_getpeername(socket, &temp, &temp_len);

      if (peer_result == SOCKET_ERROR)
      {
        imports->WSASetLastError(WSAEINVAL);
        return SOCKET_ERROR;
      }
    }

    get_overlapped_worker(worker_action::restart_if_stopped);
    get_socket_handles().queue_overlapped_write_work(socket_handle_info::overlapped_write_params{
      .socket = socket,
      .overlapped = overlapped,
      .data = std::move(temp_buffer),
      .addr = temp,
      .addr_size = temp_len,
      .flags = flags,
      .thread_handle = thread_handle,
      .callback = completion_handler });

    imports->WSASetLastError(WSA_IO_PENDING);
    return SOCKET_ERROR;
  }

  auto sent_size = siege_sendto(socket, temp_buffer.data(), static_cast<int>(temp_buffer.size()), flags, to, len);

  if (sent_size == SOCKET_ERROR)
  {
    return SOCKET_ERROR;
  }

  *bytes_sent = static_cast<DWORD>(sent_size);
  return 0;
}

auto __stdcall siege_WSASend(SOCKET socket, WSABUF* buffers, DWORD buffer_count, DWORD* bytes_sent, DWORD flags, OVERLAPPED* overlapped, LPWSAOVERLAPPED_COMPLETION_ROUTINE completion_handler) noexcept
{
  if (!use_custom_backend())
  {
    return imports->WSASend(socket, buffers, buffer_count, bytes_sent, flags, overlapped, completion_handler);
  }

  return siege_WSASendTo(socket, buffers, buffer_count, bytes_sent, flags, nullptr, 0, overlapped, completion_handler);
}

auto __stdcall siege_WSAEventSelect(SOCKET socket, WSAEVENT event, long flags) noexcept
{
  get_log() << "siege_WSAEventSelect";

  if (!use_custom_backend())
  {
    return imports->WSAEventSelect(socket, event, flags);
  }

  if (flags & FD_QOS)
  {
    get_log() << "FD_QOS not supported for siege_WSAAsyncSelect.\n";
    imports->WSASetLastError(WSAEINVAL);
    return SOCKET_ERROR;
  }

  if (flags & FD_ROUTING_INTERFACE_CHANGE)
  {
    get_log() << "FD_ROUTING_INTERFACE_CHANGE not supported for siege_WSAAsyncSelect.\n";
    imports->WSASetLastError(WSAEINVAL);
    return SOCKET_ERROR;
  }

  if (flags & FD_ADDRESS_LIST_CHANGE)
  {
    get_log() << "FD_ADDRESS_LIST_CHANGE not supported for siege_WSAAsyncSelect.\n";
    imports->WSASetLastError(WSAEINVAL);
    return SOCKET_ERROR;
  }

  if (flags == 0)
  {
    if (!get_socket_handles().clear_target(socket))
    {
      imports->WSASetLastError(WSAENOTSOCK);
      return SOCKET_ERROR;
    }
    return 0;
  }

  if (!get_socket_handles().set_event_target(socket, event, static_cast<int>(flags)))
  {
    imports->WSASetLastError(WSAENOTSOCK);
    return SOCKET_ERROR;
  }

  get_socket_handles().set_virtual_blocking(socket, false);
  get_select_worker(worker_action::restart_if_stopped);
  return 0;
}
#endif
auto __stdcall siege_WSAAsyncSelect(SOCKET socket, HWND window, u_int message, long flags)
{
  if (!use_custom_backend())
  {
    return imports->WSAAsyncSelect(socket, window, message, flags);
  }
#ifdef USE_WINSOCK2
  if (flags & FD_QOS)
  {
    get_log() << "FD_QOS not supported for siege_WSAAsyncSelect.\n";
    imports->WSASetLastError(WSAEINVAL);
    return SOCKET_ERROR;
  }

  if (flags & FD_ROUTING_INTERFACE_CHANGE)
  {
    get_log() << "FD_ROUTING_INTERFACE_CHANGE not supported for siege_WSAAsyncSelect.\n";
    imports->WSASetLastError(WSAEINVAL);
    return SOCKET_ERROR;
  }

  if (flags & FD_ADDRESS_LIST_CHANGE)
  {
    get_log() << "FD_ADDRESS_LIST_CHANGE not supported for siege_WSAAsyncSelect.\n";
    imports->WSASetLastError(WSAEINVAL);
    return SOCKET_ERROR;
  }
#endif

  if (flags == 0)
  {
    if (!get_socket_handles().clear_target(socket))
    {
      imports->WSASetLastError(WSAENOTSOCK);
      return SOCKET_ERROR;
    }
    return 0;
  }

  if (!get_socket_handles().set_window_target(socket, window, message, static_cast<int>(flags)))
  {
    imports->WSASetLastError(WSAENOTSOCK);
    return SOCKET_ERROR;
  }

  get_socket_handles().set_virtual_blocking(socket, false);
  get_select_worker(worker_action::restart_if_stopped);
  return 0;
}

#ifdef USE_WINSOCK2
auto __stdcall siege_WSAEnumNetworkEvents(SOCKET s, WSAEVENT hEventObject, LPWSANETWORKEVENTS lpNetworkEvents) noexcept
{
  if (!use_custom_backend())
  {
    return imports->WSAEnumNetworkEvents(s, hEventObject, lpNetworkEvents);
  }

  if (!lpNetworkEvents)
  {
    imports->WSASetLastError(WSAEINVAL);
    return SOCKET_ERROR;
  }

  auto result = get_socket_handles().consume_event_flags(s);

  if (!result)
  {
    imports->WSASetLastError(WSAENOTSOCK);
    return SOCKET_ERROR;
  }

  std::memcpy(lpNetworkEvents, &*result, sizeof(*lpNetworkEvents));
  if (hEventObject)
  {
    ::ResetEvent(hEventObject);
  }

  return 0;
}

auto __stdcall siege_WSACreateEvent() noexcept
{
  if (!use_custom_backend())
  {
    ensure_imports();
    return imports->WSACreateEvent();
  }

  auto result = ::CreateEventW(nullptr, TRUE, FALSE, nullptr);

  if (result == nullptr)
  {
    // TODO map error code
    imports->WSASetLastError(WSA_NOT_ENOUGH_MEMORY);
    return WSA_INVALID_EVENT;
  }
  return result;
}

auto __stdcall siege_WSASetEvent(HANDLE event) noexcept
{
  if (!use_custom_backend())
  {
    ensure_imports();
    return imports->WSASetEvent(event);
  }

  if (!::SetEvent(event))
  {
    // TODO map error code
    imports->WSASetLastError(WSA_INVALID_HANDLE);
    return FALSE;
  }

  return TRUE;
}

auto __stdcall siege_WSAResetEvent(HANDLE event) noexcept
{
  if (!use_custom_backend())
  {
    ensure_imports();
    return imports->WSAResetEvent(event);
  }

  if (!::ResetEvent(event))
  {
    // TODO map error code
    imports->WSASetLastError(WSA_INVALID_HANDLE);
    return FALSE;
  }

  return TRUE;
}

auto __stdcall siege_WSACloseEvent(HANDLE event) noexcept
{
  if (!use_custom_backend())
  {
    ensure_imports();
    return imports->WSACloseEvent(event);
  }

  if (!::CloseHandle(event))
  {
    // TODO map error code
    imports->WSASetLastError(WSA_INVALID_HANDLE);
    return FALSE;
  }

  return TRUE;
}

auto __stdcall siege_WSAWaitForMultipleEvents(DWORD event_count, const HANDLE* events, BOOL wait_all, DWORD timeout, BOOL alertable) noexcept
{
  if (!use_custom_backend())
  {
    ensure_imports();
    return imports->WSAWaitForMultipleEvents(event_count, events, wait_all, timeout, alertable);
  }

  if (event_count == 0 || event_count > WSA_MAXIMUM_WAIT_EVENTS)
  {
    imports->WSASetLastError(WSA_INVALID_PARAMETER);
    return WSA_WAIT_FAILED;
  }

  static_assert(WAIT_TIMEOUT == WSA_WAIT_TIMEOUT);
  static_assert(WAIT_IO_COMPLETION == WSA_WAIT_IO_COMPLETION);
  static_assert(WAIT_OBJECT_0 == WSA_WAIT_EVENT_0);

  auto result = ::WaitForMultipleObjectsEx(event_count, events, wait_all, timeout, alertable);

  if (result == WAIT_FAILED)
  {
    // TODO map error code
    imports->WSASetLastError(WSA_INVALID_HANDLE);
  }

  return result;
}
#endif


namespace {
struct service_entry
{
  std::string_view name;
  u_short port;
  std::string_view proto;
};

struct protocol_entry
{
  std::string_view name;
  short number;
};

constexpr service_entry service_entries[] = {
  { "directplaysrvr", 47624, "tcp" },
  { "directplaysrvr", 47624, "udp" },
  { "directplay8", 6073, "tcp" },
  { "directplay8", 6073, "udp" },
  { "xbox", 3074, "tcp" },
  { "xbox", 3074, "udp" },
  { "directplay", 2234, "tcp" },
  { "directplay", 2234, "udp" },
  { "close-combat", 1944, "tcp" },
  { "close-combat", 1944, "udp" },
  { "remote-winsock", 1745, "tcp" },
  { "remote-winsock", 1745, "udp" },
  { "doom", 666, "tcp" },
  { "doom", 666, "udp" },
  { "https", 443, "tcp" },
  { "https", 443, "udp" },
  { "http", 80, "tcp" },
  { "ftp-data", 20, "tcp" },
  { "ftp", 21, "tcp" },
  { "ipx", 213, "udp" },
};

constexpr protocol_entry protocol_entries[] = {
  { "ip", 0 },
  { "icmp", 1 },
  { "tcp", 6 },
  { "udp", 17 },
};

bool name_equals(std::string_view left, std::string_view right);
servent* make_servent(const service_entry& entry);
protoent* make_protoent(const protocol_entry& entry);
}

servent* __stdcall siege_getservbyname(const char* name, const char* proto) noexcept
{
  ensure_imports();
  if (!use_custom_backend())
  {
    return imports->getservbyname(name, proto);
  }

  if (!name)
  {
    imports->WSASetLastError(WSAEINVAL);
    return nullptr;
  }

  for (const auto& entry : service_entries)
  {
    if (name_equals(entry.name, name) && (!proto || name_equals(entry.proto, proto)))
    {
      imports->WSASetLastError(0);
      return make_servent(entry);
    }
  }

  imports->WSASetLastError(WSANO_DATA);
  return nullptr;
}

servent* __stdcall siege_getservbyport(int port, const char* proto) noexcept
{
  ensure_imports();
  if (!use_custom_backend())
  {
    return imports->getservbyport(port, proto);
  }

  const auto host_port = imports->ntohs(static_cast<u_short>(port));
  for (const auto& entry : service_entries)
  {
    if (entry.port == host_port && (!proto || name_equals(entry.proto, proto)))
    {
      imports->WSASetLastError(0);
      return make_servent(entry);
    }
  }

  imports->WSASetLastError(WSANO_DATA);
  return nullptr;
}

protoent* __stdcall siege_getprotobyname(const char* name) noexcept
{
  ensure_imports();
  if (!use_custom_backend())
  {
    return imports->getprotobyname(name);
  }

  if (!name)
  {
    imports->WSASetLastError(WSAEINVAL);
    return nullptr;
  }

  for (const auto& entry : protocol_entries)
  {
    if (name_equals(entry.name, name))
    {
      imports->WSASetLastError(0);
      return make_protoent(entry);
    }
  }

  imports->WSASetLastError(WSANO_DATA);
  return nullptr;
}

protoent* __stdcall siege_getprotobynumber(int number) noexcept
{
  ensure_imports();
  if (!use_custom_backend())
  {
    return imports->getprotobynumber(number);
  }

  for (const auto& entry : protocol_entries)
  {
    if (entry.number == number)
    {
      imports->WSASetLastError(0);
      return make_protoent(entry);
    }
  }

  imports->WSASetLastError(WSANO_DATA);
  return nullptr;
}

int __stdcall siege_gethostname(char* name, int namelen) noexcept
{
  ensure_imports();
  if (!use_custom_backend())
  {
    return imports->gethostname(name, namelen);
  }

  if (!name || namelen <= 0)
  {
    imports->WSASetLastError(WSAEFAULT);
    return SOCKET_ERROR;
  }

  DWORD size = static_cast<DWORD>(namelen);

  if (::GetComputerNameExA(ComputerNamePhysicalDnsHostname, name, &size))
  {
    return 0;
  }

  imports->WSASetLastError(WSAENETDOWN);
  return SOCKET_ERROR;
}

auto __stdcall siege_WSAGetLastError() noexcept
{
  ensure_imports();
  return imports->WSAGetLastError();
}

auto __stdcall siege_htonl(u_long value) noexcept
{
  ensure_imports();
  return imports->htonl(value);
}

auto __stdcall siege_htons(u_short value) noexcept
{
  ensure_imports();
  return imports->htons(value);
}

auto __stdcall siege_ntohl(u_long value) noexcept
{
  ensure_imports();
  return imports->ntohl(value);
}

auto __stdcall siege_ntohs(u_short value) noexcept
{
  ensure_imports();
  return imports->ntohs(value);
}

auto __stdcall siege_inet_addr(const char* addr) noexcept
{
  ensure_imports();
  return imports->inet_addr(addr);
}

auto __stdcall siege_inet_ntoa(in_addr in) noexcept
{
  ensure_imports();
  return imports->inet_ntoa(in);
}

#ifdef USE_WINSOCK2
// TODO: add WSAAddressToStringA (Celtic Kings)
auto __stdcall siege_WSAStringToAddressA(LPSTR address_str, INT family, LPWSAPROTOCOL_INFOA info, LPSOCKADDR out_address, LPINT out_len)
{
  ensure_imports();
  return imports->WSAStringToAddressA(address_str, family, info, out_address, out_len);
}

INT __stdcall siege_WSAAddressToStringA(LPSOCKADDR lpsaAddress, DWORD dwAddressLength, LPWSAPROTOCOL_INFOA lpProtocolInfo, LPSTR lpszAddressString, LPDWORD lpdwAddressStringLength)
{
  ensure_imports();
  return imports->WSAAddressToStringA(lpsaAddress, dwAddressLength, lpProtocolInfo, lpszAddressString, lpdwAddressStringLength);
}

#endif

auto __stdcall siege_WSASetLastError(int error)
{
  ensure_imports();
  return imports->WSASetLastError(error);
}

auto __stdcall siege_WSASetBlockingHook(FARPROC proc)
{
  ensure_imports();
  get_log() << "siege_WSASetBlockingHook " << '\n';
  return imports->WSASetBlockingHook(proc);
}

auto __stdcall siege_WSAUnhookBlockingHook()
{
  ensure_imports();
  get_log() << "siege_WSAUnhookBlockingHook " << '\n';
  return imports->WSAUnhookBlockingHook();
}

auto __stdcall siege_WSACancelBlockingCall()
{
  ensure_imports();
  get_log() << "siege_WSACancelBlockingCall " << '\n';
  return imports->WSACancelBlockingCall();
}
}

std::jthread& get_select_worker(worker_action action)
{
  static auto worker_impl = [](std::stop_token token) {
    while (!token.stop_requested())
    {
      thread_local std::unordered_map<SOCKET, socket_handle_info::socket_work> socket_work;
      thread_local std::unordered_set<SOCKET> closed_sockets;
      closed_sockets.clear();

      get_socket_handles().get_sockets_to_watch(socket_work);
      fd_set read_set{};
      fd_set write_set{};
      fd_set except_set{};


      if (socket_work.empty())
      {
        std::this_thread::sleep_for(std::chrono::milliseconds{ 10 });
        continue;
      }

      for (auto& [socket, work] : socket_work)
      {
        if (work.client_state != socket_handle_info::client_socket_state::unconnected && work.is_closed && work.select_flags & FD_CLOSE && ~work.io_flags & FD_CLOSE)
        {
          closed_sockets.emplace(socket);
          continue;
        }

        if (work.is_listening && work.select_flags & FD_ACCEPT && ~work.io_flags & FD_ACCEPT)
        {
          FD_SET(work.socket, &read_set);
          continue;
        }

        bool is_client_connected = work.client_state == socket_handle_info::client_socket_state::connected;
        bool is_client_accepted = work.client_state == socket_handle_info::client_socket_state::accepted;

        if ((is_client_connected || is_client_accepted) && work.select_flags & FD_CLOSE && ~work.io_flags & FD_CLOSE)
        {
          FD_SET(work.socket, &read_set);
        }

        bool is_client_connecting = work.client_state == socket_handle_info::client_socket_state::connecting;
        if ((is_client_connected || is_client_connecting) && work.select_flags & FD_CONNECT && ~work.io_flags & FD_CONNECT)
        {
          FD_SET(work.socket, &write_set);
          FD_SET(work.socket, &except_set);
        }

        if (work.select_flags & FD_READ && ~work.io_flags & FD_READ)
        {
          FD_SET(work.socket, &read_set);
        }

        if (work.select_flags & FD_WRITE && ~work.io_flags & FD_WRITE)
        {
          // TODO: edge after WSAEWOULDBLOCK, not level-trigger (GTR2)
          FD_SET(work.socket, &write_set);
        }

        if (work.select_flags & FD_OOB && ~work.io_flags & FD_OOB)
        {
          FD_SET(work.socket, &except_set);
        }
      }

      timeval time{};
      auto count = siege_select(0, &read_set, &write_set, &except_set, &time);

      if (count == 0 && closed_sockets.empty())
      {
        std::this_thread::yield();
        continue;
      }

      // the flags could have been reset in-between, however unlikely
      get_socket_handles().get_sockets_to_watch(socket_work);

      auto complete_connect = [](SOCKET socket, socket_handle_info::socket_work& work) {
        bool is_client_connected = work.client_state == socket_handle_info::client_socket_state::connected;
        bool is_client_connecting = work.client_state == socket_handle_info::client_socket_state::connecting;

        if (!(is_client_connected || is_client_connecting) || !(work.select_flags & FD_CONNECT) || work.io_flags & FD_CONNECT || work.event_flags & FD_CONNECT)
        {
          return;
        }

        int connect_error = 0;
        int connect_error_size = sizeof(connect_error);
        if (siege_getsockopt(socket, SOL_SOCKET, SO_ERROR, reinterpret_cast<char*>(&connect_error), &connect_error_size) != 0)
        {
          connect_error = 0;
        }

        work.event_errors[FD_CONNECT_BIT] = connect_error;
        work.event_flags |= FD_CONNECT;

        if (connect_error != 0)
        {
          get_socket_handles().set_client_socket_state(socket, socket_handle_info::client_socket_state::unconnected);
        }
        else if (is_client_connecting)
        {
          get_socket_handles().set_client_socket_state(socket, socket_handle_info::client_socket_state::connected);
        }
      };

      for (auto i = 0; i < read_set.fd_count; i++)
      {
        try
        {
          auto& work = socket_work.at(read_set.fd_array[i]);

          if (work.is_listening && work.select_flags & FD_ACCEPT && ~work.io_flags & FD_ACCEPT)
          {
            work.event_flags |= FD_ACCEPT;
            continue;
          }

          if (work.select_flags & FD_READ || work.select_flags & FD_CLOSE)
          {
            u_long bytes = 0;
            if (siege_ioctlsocket(work.socket, FIONREAD, &bytes) == 0)
            {
              if (bytes > 0)
              {
                if (work.select_flags & FD_READ && ~work.io_flags & FD_READ)
                {
                  work.event_flags |= FD_READ;
                }
              }
              else if (work.select_flags & FD_CLOSE && ~work.io_flags & FD_CLOSE)
              {
                DWORD error = 0;
                int error_size = sizeof(error);
                if (siege_getsockopt(work.socket, SOL_SOCKET, SO_ERROR, reinterpret_cast<char*>(&error), &error_size) == 0 && error != 0)
                {
                  work.event_errors[FD_CLOSE_BIT] = static_cast<int>(error);
                }
                else
                {
                  work.event_errors[FD_CLOSE_BIT] = 0;
                }
                work.event_flags |= FD_CLOSE;
              }
            }
          }
        }
        catch (const std::out_of_range&)
        {
        }
      }

      for (auto i = 0; i < write_set.fd_count; i++)
      {
        try
        {
          auto& work = socket_work.at(write_set.fd_array[i]);
          complete_connect(write_set.fd_array[i], work);

          if (work.select_flags & FD_WRITE && ~work.io_flags & FD_WRITE)
          {
            work.event_flags |= FD_WRITE;
          }
        }
        catch (const std::out_of_range&)
        {
        }
      }

      for (auto i = 0; i < except_set.fd_count; i++)
      {
        try
        {
          auto& work = socket_work.at(except_set.fd_array[i]);
          complete_connect(except_set.fd_array[i], work);

          if (work.select_flags & FD_OOB && ~work.io_flags & FD_OOB)
          {
            work.event_flags |= FD_OOB;
          }
        }
        catch (const std::out_of_range&)
        {
        }
      }

      for (auto socket : closed_sockets)
      {
        try
        {
          auto& work = socket_work.at(socket);
          work.event_flags |= FD_CLOSE;
          work.event_errors[FD_CLOSE_BIT] = 0;
        }
        catch (const std::out_of_range&)
        {
        }
      }

      for (auto& [socket, work] : socket_work)
      {
        if (work.event_flags == 0)
        {
          continue;
        }

        get_socket_handles().set_io_flags(socket, work.event_flags, work.event_errors);

        if (std::holds_alternative<socket_handle_info::window_target>(work.target))
        {
          auto& window = std::get<socket_handle_info::window_target>(work.target);
          assert(window.post_message != nullptr);

          // TODO: high-word error for FD_CLOSE too (GTR2)
          auto error = work.event_flags & FD_CONNECT ? work.event_errors[FD_CONNECT_BIT] : 0;
          window.post_message(window.window, window.message, static_cast<WPARAM>(socket), MAKELPARAM(work.event_flags, error));
        }
        else
        {
          auto& event = std::get<socket_handle_info::event_target>(work.target);

          ::SetEvent(event.event);
        }
      }
    }
  };

  static std::jthread worker = std::jthread(worker_impl);

  if (action == worker_action::restart_if_stopped && worker.get_stop_token().stop_requested())
  {
    worker = std::jthread(worker_impl);
  }

  return worker;
}

#ifdef USE_WINSOCK2
void __stdcall apc_callback(ULONG_PTR overlapped_raw)
{
  WSAOVERLAPPED* overlapped = (WSAOVERLAPPED*)overlapped_raw;

  auto result = get_socket_handles().get_overlapped_result(overlapped);

  if (!result)
  {
    return;
  }

  if (result->target_addr && result->target_addr_size && result->received_addr_size > 0)
  {
    auto real_size = std::clamp<int>(result->received_addr_size, 0, *result->target_addr_size);
    std::memcpy(result->target_addr, &result->received_addr, real_size);
    *result->target_addr_size = real_size;
  }

  if (result->result && result->completion_callback)
  {
    result->completion_callback(0, *result->result, overlapped, result->flags);
  }
  else if (result->completion_callback)
  {
    result->completion_callback(result->result.error(), 0, overlapped, result->flags);
  }

  get_socket_handles().remove_overlapped_work(overlapped);
}

std::jthread& get_overlapped_worker(worker_action action)
{
  static auto worker_impl = [](std::stop_token token) {
    while (!token.stop_requested())
    {
      thread_local std::unordered_set<SOCKET> sockets;

      get_socket_handles().get_overlapped_sockets_with_work(sockets);

      // do one job for each socket each cycle
      for (auto socket : sockets)
      {
        auto read_work = get_socket_handles().peek_overlapped_read_work(socket);
        auto write_work = get_socket_handles().peek_overlapped_write_work(socket);

        if (!(read_work || write_work))
        {
          continue;
        }
        fd_set read_set{};
        fd_set write_set{};

        if (read_work)
        {
          FD_SET(socket, &read_set);
        }

        if (write_work)
        {
          FD_SET(socket, &write_set);
        }

        timeval zero{};

        auto result = siege_select(0, &read_set, &write_set, nullptr, &zero);

        if (result == 0)
        {
          continue;
        }

        auto dispatch_work = [](auto& work) {
          if (std::holds_alternative<socket_handle_info::overlapped_callback>(work.target))
          {
            auto& target = std::get<socket_handle_info::overlapped_callback>(work.target);
            assert(target.thread_handle != nullptr);
            assert(target.completion_routine != nullptr);

            ::QueueUserAPC(apc_callback, target.thread_handle, (ULONG_PTR)work.overlapped);
            ::CloseHandle(target.thread_handle);
          }
          else if (std::holds_alternative<socket_handle_info::overlapped_event>(work.target))
          {
            auto& target = std::get<socket_handle_info::overlapped_event>(work.target);
            assert(target.event != nullptr);
            ::SetEvent(target.event);
          }
        };

        if (read_set.fd_count > 0)
        {
          assert(read_work.has_value());

          thread_local std::vector<WSABUF> temp;
          temp.clear();
          temp.reserve(read_work->read_targets.size());

          for (auto& target : read_work->read_targets)
          {
            temp.emplace_back(static_cast<ULONG>(target.size()), target.data());
          }

          DWORD bytes_received = 0;
          sockaddr addr{};
          int* addr_size = read_work->read_addr_size > 0 ? &read_work->read_addr_size : nullptr;

          auto token = get_socket_handles().temp_disable_virtual_blocking(socket);
          auto socket_result = siege_WSARecvFrom(socket, temp.data(), static_cast<DWORD>(temp.size()), &bytes_received, &read_work->flags, &addr, addr_size, nullptr, nullptr);
          token.reset();

          if (socket_result != SOCKET_ERROR)
          {
            get_socket_handles().complete_overlapped_read_work(read_work->overlapped, bytes_received, read_work->flags, addr, read_work->read_addr_size);
          }
          else
          {
            get_socket_handles().error_overlapped_work(read_work->overlapped, siege_WSAGetLastError());
          }

          dispatch_work(*read_work);
        }

        if (write_set.fd_count > 0)
        {
          assert(write_work.has_value());

          auto token = get_socket_handles().temp_disable_virtual_blocking(socket);
          auto socket_result = siege_sendto(socket, write_work->data_to_write.data(), static_cast<int>(write_work->data_to_write.size()), write_work->flags, &write_work->addr, write_work->addr_size);
          token.reset();

          if (socket_result != SOCKET_ERROR)
          {
            get_socket_handles().complete_overlapped_write_work(write_work->overlapped, socket_result);
          }
          else
          {
            get_socket_handles().error_overlapped_work(write_work->overlapped, siege_WSAGetLastError());
          }

          dispatch_work(*write_work);
        }
      }

      std::this_thread::yield();
    }
  };

  static std::jthread worker = std::jthread(worker_impl);

  if (action == worker_action::restart_if_stopped && worker.get_stop_token().stop_requested())
  {
    worker = std::jthread(worker_impl);
  }

  return worker;
}
#endif

namespace {
char* empty_aliases[] = { nullptr };

bool name_equals(std::string_view left, std::string_view right)
{
  return left.size() == right.size() && _strnicmp(left.data(), right.data(), left.size()) == 0;
}

char* as_c_str(std::string_view value)
{
  return const_cast<char*>(value.data());
}

servent* make_servent(const service_entry& entry)
{
  static servent result{};
  result.s_name = as_c_str(entry.name);
  result.s_aliases = empty_aliases;
  result.s_port = imports->htons(entry.port);
  result.s_proto = as_c_str(entry.proto);
  return &result;
}

protoent* make_protoent(const protocol_entry& entry)
{
  static protoent result{};
  result.p_name = as_c_str(entry.name);
  result.p_aliases = empty_aliases;
  result.p_proto = entry.number;
  return &result;
}
}