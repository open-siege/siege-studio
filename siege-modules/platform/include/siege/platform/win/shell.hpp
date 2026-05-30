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
#include <siege/platform/win/common_controls.hpp>

namespace win32
{
  inline int shell_image_list_source_kind(int target_size)
  {
    if (target_size <= 16)
    {
      return SHIL_SMALL;
    }

    if (target_size <= 32)
    {
      return SHIL_LARGE;
    }

    if (target_size <= 48)
    {
      return SHIL_EXTRALARGE;
    }

    return SHIL_JUMBO;
  }

  // Scales a shell icon into image-list slot `index` with its alpha intact. CopyImage(IMAGE_ICON)
  // drops alpha on resize, turning transparent file icons into black squares; DrawIconEx into a
  // 32-bpp DIB doesn't.
  inline bool replace_image_list_icon_scaled(HIMAGELIST list, int index, HICON icon, int size)
  {
    if (!list || !icon || index < 0 || size <= 0)
    {
      return false;
    }

    win32::gdi::bitmap dib(SIZE{ .cx = size, .cy = size }, win32::gdi::bitmap::skip_shared_handle);

    if (!dib)
    {
      return false;
    }

    {
      win32::gdi::memory_drawing_context dc;
      ::SelectObject(dc, dib.get());

      if (!::DrawIconEx(dc, 0, 0, icon, size, size, 0, nullptr, DI_NORMAL))
      {
        return false;
      }
    }

    return ::ImageList_Replace(list, index, dib.get(), nullptr) == TRUE;
  }

  inline win32::image_list create_dpi_shell_image_list(int target_size)
  {
    auto source_kind = shell_image_list_source_kind(target_size);

    win32::com::com_ptr<IImageList> source;
    if (::SHGetImageList(source_kind, IID_IImageList, (void**)source.put()) != S_OK || !source)
    {
      return {};
    }

    int total = 0;
    if (source->GetImageCount(&total) != S_OK || total <= 0)
    {
      return {};
    }

    win32::image_list result(target_size, target_size, ILC_COLOR32 | ILC_MASK, total, 16);
    ::ImageList_SetImageCount(result.get(), total);

    for (int i = 0; i < total; ++i)
    {
      HICON raw = nullptr;
      if (source->GetIcon(i, ILD_TRANSPARENT, &raw) != S_OK || !raw)
      {
        continue;
      }

      replace_image_list_icon_scaled(result.get(), i, raw, target_size);
      ::DestroyIcon(raw);
    }

    return result;
  }

  inline win32::image_list create_dpi_small_shell_image_list(UINT dpi)
  {
    return create_dpi_shell_image_list(::MulDiv(16, dpi, USER_DEFAULT_SCREEN_DPI));
  }

  inline win32::image_list create_dpi_large_shell_image_list(UINT dpi)
  {
    return create_dpi_shell_image_list(::MulDiv(32, dpi, USER_DEFAULT_SCREEN_DPI));
  }

  // Extracts a system icon by image-list index, forcing it present first (some shell icons,
  // e.g. compressed folders, are realized on a background thread). Caller owns the result.
  inline HICON extract_present_shell_icon(int sys_index, int target_size)
  {
    if (sys_index < 0)
    {
      return nullptr;
    }

    auto source_kind = shell_image_list_source_kind(target_size);

    win32::com::com_ptr<IImageList2> source;
    if (::SHGetImageList(source_kind, IID_IImageList2, (void**)source.put()) != S_OK || !source)
    {
      return nullptr;
    }

    source->ForceImagePresent(sys_index, ILFIP_ALWAYS);

    HICON raw = nullptr;
    if (source->GetIcon(sys_index, ILD_TRANSPARENT, &raw) != S_OK)
    {
      return nullptr;
    }

    return raw;
  }

  // Patches a DPI-resized icon into a private shell image list at sys_index. Pass a known-good
  // source (e.g. a synchronous SHGSI_ICON stock icon, owned by the caller); otherwise the icon
  // is re-extracted (forced present) from the system list.
  inline void patch_shell_image_list_icon(win32::image_list& list, int sys_index, HICON source = nullptr)
  {
    if (!list || sys_index < 0)
    {
      return;
    }

    auto size = list.GetIconSize();
    if (!size)
    {
      return;
    }

    HICON present = source ? source : extract_present_shell_icon(sys_index, size->cx);
    if (!present)
    {
      return;
    }

    if (::ImageList_GetImageCount(list.get()) <= sys_index)
    {
      ::ImageList_SetImageCount(list.get(), sys_index + 1);
    }

    replace_image_list_icon_scaled(list.get(), sys_index, present, size->cx);

    if (!source)
    {
      ::DestroyIcon(present);
    }
  }

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

namespace win32
{
  std::expected<std::filesystem::path, HRESULT> get_path_via_file_dialog(OPENFILENAMEW);
}

#endif// !WIN32_DIALOGS_HPP
