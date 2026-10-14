module;
#include <siege/platform/windows.hpp>
#include <shobjidl.h>
#include <ShlObj.h>

export module win32.shell_dialogs;

import std;
export import win32.com;

export namespace win32::com
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
      ::CoTaskMemFree(pszFilePath);
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

  std::expected<FileOpenDialogEx, HRESULT> CreateFileOpenDialog()
  {
    FileOpenDialogEx pFileOpen;

    auto hr = CoCreateInstance(CLSID_FileOpenDialog, nullptr, CLSCTX_ALL, __uuidof(::IFileOpenDialog), pFileOpen.put_void());

    if (hr != S_OK)
    {
      return std::unexpected(hr);
    }

    return pFileOpen;
  }

  std::expected<FileSaveDialogEx, HRESULT> CreateFileSaveDialog()
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

