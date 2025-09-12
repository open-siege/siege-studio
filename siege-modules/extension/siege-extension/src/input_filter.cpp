#include <siege/platform/win/file.hpp>
#include <siege/extension/input_filter.hpp>

namespace siege
{
  void init_active_input_state()
  {
    std::vector<RAWINPUTDEVICELIST> temp(64, {});

    UINT size = temp.size();
    auto& state = get_active_input_state();

    for (auto& existing : state.devices)
    {
      existing.id = 0;
      existing.device_handle = nullptr;
    }

    if (auto result = ::GetRawInputDeviceList(temp.data(), &size, sizeof(RAWINPUTDEVICELIST)); result > 0)
    {
      temp.resize(result);

      for (auto& device : temp)
      {
        if (!update_device_id(device))
        {
          continue;
        }
      }
    }
  }

  bool update_device_id(const RAWINPUTDEVICELIST& device)
  {
    auto& state = get_active_input_state();
    auto existing_iter = std::find_if(state.devices.begin(), state.devices.end(), [&](auto& device_info) {
      return device_info.device_handle == device.hDevice;
    });

    if (existing_iter != state.devices.end())
    {
      return false;
    }

    auto device_iter = std::find_if(state.devices.begin(), state.devices.end(), [](auto& device) {
      return device.id == 0;
    });

    if (device_iter == state.devices.end())
    {
      return false;
    }

    auto device_id = ::RegisterWindowMessageW(L"SiegeInputDeviceId") + std::distance(state.devices.begin(), device_iter);

    auto typed_iter = std::find_if(device_iter, state.devices.end(), [&](auto& device_state) {
      return device_state.id == 0 && device_state.type == device.dwType;
    });

    if (typed_iter != state.devices.end())
    {
      typed_iter->id = ::RegisterWindowMessageW(L"SiegeInputDeviceId") + std::distance(state.devices.begin(), typed_iter);
      typed_iter->device_handle = device.hDevice;
    }
    else
    {
      device_iter->type = device.dwType;
      device_iter->id = device_id;
      device_iter->device_handle = device.hDevice;
    }

    return true;
  }

  bool remove_device(HANDLE handle)
  {
    auto& state = get_active_input_state();
    auto existing = std::find_if(state.devices.begin(), state.devices.end(), [handle](auto& device) {
      return device.device_handle == handle;
    });

    if (existing == state.devices.end())
    {
      return false;
    }

    existing->id = 0;
    existing->device_handle = nullptr;
    return true;
  }

  std::optional<std::uint16_t> find_device_id(HANDLE handle)
  {
    if (handle == nullptr)
    {
      return std::nullopt;
    }

    auto& state = get_active_input_state();

    auto device_iter = std::find_if(state.devices.begin(), state.devices.end(), [handle](auto& device) {
      return device.device_handle == handle;
    });

    if (device_iter == state.devices.end())
    {
      return std::nullopt;
    }

    return device_iter->id;
  }

  active_input_state& get_active_input_state()
  {
    static std::pair<win32::file_mapping, win32::file_view> mapping = [] {
      auto existing = ::OpenFileMappingW(FILE_MAP_WRITE, FALSE, L"SiegeAppActiveInputState");

      if (existing == nullptr)
      {
        auto mapping = win32::file_mapping(::CreateFileMappingW(
          INVALID_HANDLE_VALUE,// use paging file
          nullptr,// default security attributes
          PAGE_READWRITE,// read/write access
          0,// size: high 32-bits
          sizeof(active_input_state),// size: low 32-bits
          L"SiegeAppActiveInputState"));

        auto result = mapping.MapViewOfFile(FILE_MAP_WRITE, sizeof(sizeof(active_input_state)));
        new (result.get()) active_input_state();
        return std::make_pair(std::move(mapping), std::move(result));
      }

      auto mapping = win32::file_mapping(existing);
      auto result = mapping.MapViewOfFile(FILE_MAP_WRITE, sizeof(sizeof(active_input_state)));
      return std::make_pair(std::move(mapping), std::move(result));
    }();

    return *(active_input_state*)mapping.second.get();
  }
}// namespace siege
