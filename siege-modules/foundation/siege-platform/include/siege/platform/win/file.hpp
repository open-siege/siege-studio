#ifndef WIN_FILE_MODULE_HPP
#define WIN_FILE_MODULE_HPP

#include <siege/platform/win/auto_handle.hpp>
#include <filesystem>
#include <exception>
#include <expected>
#include <string>
#include <array>
#include <span>
#include <wtypes.h>
#include <WinDef.h>
#include <WinBase.h>
#include <psapi.h>

#undef CreateFileMapping

namespace win32
{
  struct handle_deleter
  {
    void operator()(HANDLE lib)
    {
      CloseHandle(lib);
    }
  };

  struct file_view_deleter
  {
    void operator()(void* view)
    {
      ::UnmapViewOfFile(view);
    }
  };

  struct file_view : std::unique_ptr<void, file_view_deleter>
  {
    using base = std::unique_ptr<void, file_view_deleter>;
    using base::base;

    static WORD get_mapped_file_name(HANDLE process, LPVOID address, LPWSTR filename, DWORD size)
    {
      using proc = std::add_pointer_t<decltype(::K32GetMappedFileNameW)>;
      static auto kernel32 = ::GetModuleHandleW(L"kernel32");

      if (!kernel32)
      {
        return 0;
      }
      static auto kernel_get_mapped_filename = (proc)::GetProcAddress(kernel32, "K32GetMappedFileNameW");

      if (kernel_get_mapped_filename)
      {
        return kernel_get_mapped_filename(process, address, filename, size);
      }

      static HMODULE psapi = ::LoadLibraryW(L"psapi.dll");

      static auto psapi_get_mapped_filename = (proc)::GetProcAddress(psapi, "GetMappedFileNameW");

      if (psapi_get_mapped_filename)
      {
        return psapi_get_mapped_filename(process, address, filename, size);
      }

      return 0;
    }

    std::optional<std::wstring> GetMappedFilename()
    {
      std::wstring filename(255, L'\0');

      auto size = get_mapped_file_name(::GetCurrentProcess(), get(), filename.data(), filename.size());

      if (size == 0)
      {
        return std::nullopt;
      }


      std::wstring drive = L"A:";

      std::wstring buffer(32, L'\0');

      for (auto i = drive[0]; i <= L'Z'; ++i)
      {
        drive[0] = i;

        auto vol_size = ::QueryDosDeviceW(drive.c_str(), buffer.data(), buffer.size());

        if (vol_size != 0)
        {
          buffer = buffer.c_str();

          auto index = filename.find(buffer, 0);

          if (index == 0)
          {
            filename = filename.replace(0, buffer.size(), drive);
            break;
          }
        }
        buffer.clear();
        buffer.resize(32);
      }

      filename.erase(filename.find(L'\0'));

      return filename;
    }

    operator std::span<char>()
    {
      auto data = this->get();
      if (!data)
      {
        return std::span<char>();
      }
      MEMORY_BASIC_INFORMATION info{};
      auto size = ::VirtualQuery(data, &info, sizeof(info));

      if (!size)
      {
        return std::span<char>();
      }

      return std::span<char>((char*)data, info.RegionSize);
    }
  };

  struct file_mapping : std::unique_ptr<void, handle_deleter>
  {
    using base = std::unique_ptr<void, handle_deleter>;
    using base::base;

    file_view MapViewOfFile(DWORD desired_access, std::size_t size)
    {
      return file_view(::MapViewOfFile(get(), desired_access, 0, 0, size));
    }
  };

  struct file : std::unique_ptr<void, handle_deleter>
  {
    using base = std::unique_ptr<void, handle_deleter>;
    using base::base;

    file(std::filesystem::path path, DWORD access, DWORD share_mode, std::optional<SECURITY_ATTRIBUTES> attributes, DWORD creation_disposition, DWORD flags)
      : base(::CreateFileW(path.c_str(), access, share_mode, attributes.has_value() ? &*attributes : nullptr, creation_disposition, flags, nullptr))
    {
      if (this->get() == INVALID_HANDLE_VALUE)
      {
        throw std::system_error(std::error_code(GetLastError(), std::system_category()));
      }
    }

    std::expected<file_mapping, DWORD> CreateFileMapping(std::optional<SECURITY_ATTRIBUTES> attributes, DWORD protect, LARGE_INTEGER maxSize, std::wstring name)
    {
      auto mapping = ::CreateFileMappingW(get(), attributes.has_value() ? &*attributes : nullptr, protect, maxSize.HighPart, maxSize.LowPart, name.empty() ? nullptr : name.c_str());

      if (mapping == nullptr)
      {
        return std::unexpected(::GetLastError());
      }

      return file_mapping(mapping);
    }

    std::expected<LARGE_INTEGER, DWORD> GetFileSizeEx()
    {
      LARGE_INTEGER result{};

      if (::GetFileSizeEx(get(), &result))
      {
        return result;
      }

      return std::unexpected(::GetLastError());
    }
  };

  inline std::optional<std::filesystem::path> get_path_from_handle(HANDLE handle)
  {
    static std::array<wchar_t, 256> filename;

    auto file_type = ::GetFileType(handle);

    if (file_type != FILE_TYPE_UNKNOWN || (file_type == FILE_TYPE_UNKNOWN && ::GetLastError() == 0))
    {
      if (::GetFinalPathNameByHandleW(handle, filename.data(), (DWORD)filename.size(), FILE_NAME_NORMALIZED) > 0)
      {
        return filename.data();
      }
    }

    auto data = ::MapViewOfFile(handle, FILE_MAP_READ, 0, 0, 1);

    if (data != nullptr)
    {
      auto size = ::GetFinalPathNameByHandleW(handle, filename.data(), (DWORD)filename.size(), FILE_NAME_NORMALIZED);

      if (size == 0)
      {
        win32::file_view temp(data);

        auto string = temp.GetMappedFilename();

        if (string)
        {
          return *string;
        }
      }
      else
      {
        ::UnmapViewOfFile(data);
        return filename.data();
      }
    }

    ATOM handle_as_atom = (ATOM)handle;

    if (handle_as_atom >= MAXINTATOM)
    {
      auto atom_size = ::GetAtomNameW(handle_as_atom, filename.data(), (int)filename.size());

      if (atom_size > 0)
      {
        return filename.data();
      }

      atom_size = ::GlobalGetAtomNameW(handle_as_atom, filename.data(), (int)filename.size());

      if (atom_size > 0)
      {
        return filename.data();
      }
    }

    return std::nullopt;
  }
}// namespace win32

#endif