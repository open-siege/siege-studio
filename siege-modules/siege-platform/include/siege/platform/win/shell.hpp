#ifndef WIN32_DIALOGS_HPP
#define WIN32_DIALOGS_HPP
#include <expected>
#include <memory>
#include <filesystem>
#include <shobjidl.h>
#include <ShlObj.h>
#include <shlwapi.h>
#include <commoncontrols.h>
#include <siege/platform/win/com.hpp>

namespace win32
{
  inline void launch_shell_process(const std::filesystem::path& path)
  {
    auto desktop = ::GetDesktopWindow();

    auto filename = path.filename();

    auto shell_window = ::FindWindowExW(desktop, nullptr, nullptr, filename.c_str());

    if (shell_window)
    {
      ::ShowWindow(shell_window, SW_SHOW);
    }
    else if (!shell_window)
    {
      SHELLEXECUTEINFOW info{
        .cbSize = sizeof(SHELLEXECUTEINFOW),
        .fMask = SEE_MASK_DEFAULT | SEE_MASK_NOCLOSEPROCESS,
        .lpVerb = L"explore",
        .lpFile = path.c_str(),
        .nShow = SW_NORMAL,
      };

      ::ShellExecuteExW(&info);
    }
  }
}// namespace win32

namespace win32::com
{
  struct ShellItemEx : com_ptr<::IShellItem>
  {
    using com_ptr<::IShellItem>::com_ptr;

    std::expected<std::filesystem::path, HRESULT> GetFileSysPath()
    {
      wchar_t* pszFilePath;
      auto hr = get()->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);

      if (hr != S_OK)
      {
        return std::unexpected(hr);
      }

      std::filesystem::path temp(pszFilePath);
      CoTaskMemFree(pszFilePath);
      return temp;
    }
  };

  struct FileOpenDialogEx : com_ptr<::IFileOpenDialog>
  {
    using com_ptr<::IFileOpenDialog>::com_ptr;

    std::expected<ShellItemEx, HRESULT> GetResult()
    {
      ShellItemEx result(nullptr);

      auto hr = get()->GetResult(result.put());

      if (hr != S_OK)
      {
        return std::unexpected(hr);
      }

      return result;
    }

    HRESULT SetFolder(std::filesystem::path path)
    {
      HRESULT result = S_OK;
      ITEMIDLIST* path_id;
      if (SHParseDisplayName(path.c_str(), nullptr, &path_id, SFGAO_FOLDER, nullptr) == S_OK)
      {
        win32::com::com_ptr<IShellItem> shell_item;

        if (SHCreateItemFromIDList(path_id, IID_IShellItem, shell_item.put_void()) == S_OK)
        {
          result = get()->SetFolder(shell_item.get());
        }

        ::CoTaskMemFree(path_id);
      }
      return result;
    }
  };

  struct FileSaveDialogEx : com_ptr<::IFileSaveDialog>
  {
    using com_ptr<::IFileSaveDialog>::com_ptr;

    std::expected<ShellItemEx, HRESULT> GetResult()
    {
      ShellItemEx result(nullptr);

      auto hr = get()->GetResult(result.put());

      if (hr != S_OK)
      {
        return std::unexpected(hr);
      }

      return result;
    }
  };

  inline std::expected<FileOpenDialogEx, HRESULT> CreateFileOpenDialog()
  {
    FileOpenDialogEx pFileOpen;

    auto hr = CoCreateInstance(CLSID_FileOpenDialog, nullptr, CLSCTX_ALL, __uuidof(::IFileOpenDialog), pFileOpen.put_void());

    if (hr != S_OK)
    {
      return std::unexpected(hr);
    }

    return pFileOpen;
  }

  inline std::expected<FileSaveDialogEx, HRESULT> CreateFileSaveDialog()
  {
    FileSaveDialogEx pFileOpen;

    auto hr = CoCreateInstance(CLSID_FileSaveDialog, nullptr, CLSCTX_ALL, __uuidof(::IFileSaveDialog), pFileOpen.put_void());

    if (hr != S_OK)
    {
      return std::unexpected(hr);
    }

    return pFileOpen;
  }
}// namespace win32::com

#endif// !WIN32_DIALOGS_HPP
