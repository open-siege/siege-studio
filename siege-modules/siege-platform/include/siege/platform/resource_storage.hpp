#ifndef SIEGE_PLATFORM_RESOURCE_STORAGE_HPP
#define SIEGE_PLATFORM_RESOURCE_STORAGE_HPP

#include <siege/platform/resource.hpp>
#include <siege/platform/win/core/com/storage.hpp>
#include <siege/platform/win/core/com/stream.hpp>
#include <siege/platform/win/core/com/enumerable.hpp>
#include <siege/platform/stream.hpp>

namespace siege::platform
{
  class StorageReaderRef : virtual public win32::com::StorageBase
  {
    std::istream& stream;
    resource_reader& reader;
    siege::platform::listing_query current_query;
    std::streampos initial_position;
    std::vector<siege::platform::resource_reader::content_info> storage_contents;
    std::vector<::STATSTG> enum_data;

  public:
    StorageReaderRef(std::istream& stream, resource_reader& reader, siege::platform::listing_query query) : stream(stream), reader(reader), current_query(std::move(query))
    {
      initial_position = stream.tellg();
      storage_contents = reader.get_content_listing(stream, current_query);
    }

    StorageReaderRef(std::istream& stream, resource_reader& reader) : StorageReaderRef(stream, reader, [&] {
                                           auto path = siege::platform::get_stream_path(stream);

                                           if (path)
                                           {
                                                return listing_query{ *path, std::move(*path) };
                                           }
                                           return listing_query{};
                                        }())
    {
    }

    HRESULT __stdcall OpenStream(const OLECHAR* name, void*, DWORD flags, DWORD, IStream** outstream) override
    {
      if (outstream == nullptr)
      {
        return STG_E_INVALIDPOINTER;
      }

      std::string filename = std::string(name, name + std::wcslen(name));

      auto find_result = STG_E_FILENOTFOUND;

      auto existing_file = std::find_if(storage_contents.begin(), storage_contents.end(), [&](auto& info) {
        if (auto* file_info = std::get_if<siege::platform::resource_reader::file_info>(&info); file_info)
        {
          return file_info->filename == filename;
        }

        if (auto* folder_info = std::get_if<siege::platform::resource_reader::folder_info>(&info); folder_info && folder_info->name == filename)
        {
          find_result = STG_E_INVALIDNAME;
          return false;
        }

        return false;
      });

      if (existing_file == storage_contents.end())
      {
        return find_result;
      }

      auto& file_info = std::get<siege::platform::resource_reader::file_info>(*existing_file);

      if (file_info.compression_type == compression_type::none)
      {
        auto new_stream = siege::platform::shallow_clone(stream);
        reader.set_stream_position(*new_stream, file_info);

        auto temp = std::make_unique<win32::com::OwningStdIStream<>>(std::move(new_stream), file_info.size);
        *outstream = temp.release();
      }
      else
      {
        auto vector_stream = std::unique_ptr<std::iostream>(new vectorstream(std::vector<char>(file_info.size, '\0')));

        reader.extract_file_contents(stream, file_info, *vector_stream);
        auto temp = std::make_unique<win32::com::OwningStdIStream<>>(std::move(vector_stream), file_info.size);
        *outstream = temp.release();
      }

      return S_OK;
    }

    HRESULT __stdcall OpenStorage(const OLECHAR* name, IStorage*, DWORD, SNB, DWORD, IStorage** outstorage) override
    {
      std::string filename = std::string(name, name + std::wcslen(name));

      auto find_result = STG_E_FILENOTFOUND;

      auto existing_folder = std::find_if(storage_contents.begin(), storage_contents.end(), [&](auto& info) {
        if (auto* folder_info = std::get_if<siege::platform::resource_reader::folder_info>(&info); folder_info)
        {
          return folder_info->name == filename;
        }

        if (auto* file_info = std::get_if<siege::platform::resource_reader::file_info>(&info); file_info && file_info->filename == filename)
        {
          find_result = STG_E_INVALIDNAME;
          return false;
        }

        return false;
      });

      if (existing_folder == storage_contents.end())
      {
        return find_result;
      }

      auto& folder_info = std::get<siege::platform::resource_reader::folder_info>(*existing_folder);

      auto temp = std::make_unique<StorageReaderRef>(stream, reader, listing_query{ .archive_path = folder_info.archive_path, .folder_path = folder_info.full_path });
      *outstorage = temp.release();

      return E_NOTIMPL;
    }

    struct STATSTG_EnumTraits : win32::com::Struct_EnumTraits<STATSTG>
    {
      using EnumType = IEnumSTATSTG;
      using OutType = STATSTG*;

      template<typename Iter>
      static void Copy(STATSTG* output, Iter iter)
      {
        std::memcpy(output, &*iter, sizeof(STATSTG));
      }
    };

    HRESULT __stdcall EnumElements(DWORD, void*, DWORD, IEnumSTATSTG** result) override
    {
      enum_data.resize(storage_contents.size());

      std::transform(storage_contents.begin(), storage_contents.end(), enum_data.begin(), [](auto& info) {
        if (auto* file_info = std::get_if<siege::platform::resource_reader::file_info>(&info); file_info)
        {
          auto filename = file_info->filename.wstring();
          wchar_t* text = (wchar_t*)::CoTaskMemAlloc(sizeof(wchar_t) * filename.size() + 1);
          std::memcpy(text, filename.data(), filename.size() * sizeof(wchar_t));
          text[filename.size()] = 0;
          return ::STATSTG{
            .pwcsName = text,
            .type = STGTY::STGTY_STREAM,
            .cbSize = file_info->size
          };
        }

        if (auto* folder_info = std::get_if<siege::platform::resource_reader::folder_info>(&info); folder_info)
        {
          auto filename = std::filesystem::path(folder_info->name);
          wchar_t* text = (wchar_t*)::CoTaskMemAlloc(sizeof(wchar_t) * folder_info->name.size());
          std::memcpy(text, filename.c_str(), folder_info->name.size() * sizeof(wchar_t));

          return ::STATSTG{
            .pwcsName = text,
            .type = STGTY::STGTY_STORAGE
          };
        }

        return ::STATSTG{};
      });

      auto enumerator = win32::com::make_unique_range_enumerator_from_traits<STATSTG_EnumTraits>(enum_data.begin(), enum_data.begin(), enum_data.end());

      *result = enumerator.release();
      return S_OK;
    }

    HRESULT __stdcall CopyTo(DWORD, const IID*, SNB, IStorage*) override
    {
      return E_NOTIMPL;
    }

    HRESULT __stdcall Stat(STATSTG* info, DWORD flags) override
    {
      if (info == nullptr)
      {
        return E_POINTER;
      }

      return S_OK;
    }
  };


  template<typename TStream = std::unique_ptr<std::istream>, typename TReader = std::unique_ptr<resource_reader>>
  struct OwningStorageReader : StorageReaderRef
  {
    TStream stream;
    TReader reader;

    OwningStorageReader(TStream stream, TReader reader) : StorageReaderRef(*stream, *reader),
                                                          stream(std::move(stream)),
                                                          reader(std::move(reader))
    {
    }
  };
}// namespace siege::platform

#endif