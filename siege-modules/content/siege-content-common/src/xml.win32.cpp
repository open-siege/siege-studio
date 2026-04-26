#include <siege/configuration/xml.hpp>
#include <siege/platform/win/wic.hpp>
#include <siege/platform/win/shell.hpp>
#include <xmllite.h>
#include <string>
#include <string_view>
#include <map>
#include <vector>
#include <stack>
#include <memory>

struct xml_element
{
  std::string_view raw_line;
  std::string_view name;
  std::string_view content;
  std::stack<std::string_view> parents;
  std::map<std::string_view, std::string_view> attributes;
};

std::vector<size_t> get_line_offsets(std::string_view data);
std::list<xml_element> parse_xml_elements(std::string_view raw_xml);

namespace siege::configuration::common::xml
{
  using config_line = text_game_config::config_line;

  std::optional<text_game_config> load_config(std::istream& raw_data, std::size_t stream_size)
  {
    if (stream_size == 0)
    {
      return std::nullopt;
    }

    std::unique_ptr<char[]> buffer(new char[stream_size]);
    std::fill_n(buffer.get(), stream_size, '\0');
    std::string_view buffer_str(buffer.get(), stream_size);

    auto current_pos = raw_data.tellg();
    raw_data.seekg(current_pos, std::ios::beg);
    raw_data.read(buffer.get(), stream_size);
    raw_data.seekg(current_pos, std::ios::beg);

    auto tag_list = parse_xml_elements(std::string_view(buffer.get(), stream_size));

    std::vector<config_line> config_data;
    config_data.reserve(tag_list.size());

    std::vector<std::string_view> temp_key;
    for (auto& tag : tag_list)
    {
      if (tag.content.empty() && tag.attributes.empty())
      {
        continue;
      }

      temp_key.clear();
      auto parents = tag.parents;
      while (!parents.empty())
      {
        temp_key.insert(temp_key.begin(), parents.top());
        parents.pop();
      }

      temp_key.emplace_back(tag.name);
      if (!tag.content.empty())
      {
        config_data.emplace_back(config_line{ tag.raw_line, temp_key, tag.content });
      }

      if (!tag.attributes.empty())
      {
        temp_key.emplace_back("@");
        for (auto& attr : tag.attributes)
        {
          temp_key.emplace_back(attr.first);
          config_data.emplace_back(config_line{ tag.raw_line, temp_key, attr.second });
          temp_key.pop_back();
        }
      }
    }


    return std::make_optional<text_game_config>(std::any{}, std::move(buffer), std::move(config_data), save_config);
  }

  void save_config(const std::any&, const std::vector<text_game_config::config_line>& entries, std::ostream& raw_data)
  {
  }

}// namespace siege::configuration::common::xml


std::vector<size_t> get_line_offsets(std::string_view data)
{
  std::vector<size_t> offsets = { 0 };
  for (size_t i = 0; i < data.size(); ++i)
  {
    if (data[i] == '\n')
    {
      offsets.push_back(i + 1);
    }
    else if (data[i] == '\r')
    {
      if (i + 1 < data.size() && data[i + 1] == '\n')
      {
        offsets.push_back(i + 2);
        i++;
      }
      else
      {
        offsets.push_back(i + 1);
      }
    }
  }
  return offsets;
}

std::list<xml_element> parse_xml_elements(std::string_view raw_xml)
{
  std::list<xml_element> out_list;
  auto* stream = [&]() -> IStream* {
    IWICImagingFactory* pWICFactory = nullptr;
    auto cleanup = std::shared_ptr<void>(nullptr, [&](void*) {
      if (pWICFactory)
      {
        pWICFactory->Release();
      }
    });

    IWICStream* stream = nullptr;

    if (::CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, __uuidof(IWICImagingFactory), (void**)&pWICFactory) == S_OK && pWICFactory->CreateStream(&stream) == S_OK)
    {
      if (stream->InitializeFromMemory(
            reinterpret_cast<BYTE*>(const_cast<char*>(raw_xml.data())),
            static_cast<DWORD>(raw_xml.size()))
          == S_OK)
      {
        return stream;
      }
      else
      {
        stream->Release();
      }
    }

    return ::SHCreateMemStream((BYTE*)raw_xml.data(), raw_xml.size());
  }();


  if (!stream)
  {
    return out_list;
  }
  IXmlReader* xml_reader = nullptr;

  // Scoped defer block for COM cleanup
  auto cleanup = std::shared_ptr<void>(nullptr, [&](void*) {
    if (xml_reader)
    {
      xml_reader->Release();
    }

    if (stream)
    {
      stream->Release();
    }
  });


  if (::CreateXmlReader(__uuidof(IXmlReader), (void**)&xml_reader, nullptr) != S_OK)
  {
    return out_list;
  }


  if (xml_reader->SetInput(stream) != S_OK)
  {
    return out_list;
  }

  auto line_offsets = get_line_offsets(raw_xml);
  std::stack<std::string_view> parent_stack;
  XmlNodeType nodeType;

  std::string temp_conversion;
  temp_conversion.reserve(255);

  while (xml_reader->Read(&nodeType) == S_OK)
  {
    if (nodeType == XmlNodeType_Element)
    {
      UINT line, pos;
      xml_reader->GetLineNumber(&line);
      xml_reader->GetLinePosition(&pos);

      const WCHAR* pwszName = nullptr;
      UINT name_size = 0;
      if (xml_reader->GetLocalName(&pwszName, &name_size) != S_OK && !pwszName)
      {
        return out_list;
      }

      std::wstring_view name_view = pwszName;

      temp_conversion.resize(::WideCharToMultiByte(CP_UTF8, 0, name_view.data(), name_view.size(), nullptr, 0, nullptr, nullptr));
      ::WideCharToMultiByte(CP_UTF8, 0, name_view.data(), (int)name_view.size(), temp_conversion.data(), (int)temp_conversion.size(), nullptr, nullptr);


      size_t start_search = line_offsets[line - 1] + (pos - 1);
      size_t name_pos = raw_xml.find(temp_conversion, start_search);

      xml_element current_element;
      current_element.name = { &raw_xml[name_pos], name_size };
      
      for (HRESULT hr = xml_reader->MoveToFirstAttribute(); hr == S_OK; hr = xml_reader->MoveToNextAttribute())
      {
        const WCHAR* attr = nullptr;
        UINT attr_size = 0, line_number, line_column;
        xml_reader->GetLineNumber(&line_number);
        xml_reader->GetLinePosition(&line_column);
        xml_reader->GetLocalName(&attr, &attr_size);

        if (!attr)
        {
          break;
        }

        std::wstring_view attr_view = attr;

        temp_conversion.resize(::WideCharToMultiByte(CP_UTF8, 0, attr_view.data(), attr_view.size(), nullptr, 0, nullptr, nullptr));
        ::WideCharToMultiByte(CP_UTF8, 0, attr_view.data(), (int)attr_view.size(), temp_conversion.data(), (int)temp_conversion.size(), nullptr, nullptr);

        size_t attr_start = line_offsets[line_number - 1] + (line_column - 1);
        size_t attr_name_pos = raw_xml.find(temp_conversion, attr_start);
        std::string_view attrKey(&raw_xml[attr_name_pos], attr_size);

        size_t search_pos = attr_name_pos + attr_size;
        size_t open_pos = raw_xml.find_first_of("\"'", search_pos);
        if (open_pos != std::string_view::npos)
        {
          char16_t quoteChar = raw_xml[open_pos];
          size_t closing_pos = raw_xml.find(quoteChar, open_pos + 1);
          if (closing_pos != std::string_view::npos)
          {
            current_element.attributes[attrKey] = { &raw_xml[open_pos + 1], closing_pos - open_pos - 1 };
          }
        }
      }

      std::string_view parent = parent_stack.empty() ? std::string_view{} : parent_stack.top();
      current_element.parents = parent_stack;
      out_list.push_back(current_element);

      xml_reader->MoveToElement(); 
      if (xml_reader->IsEmptyElement() == FALSE)
      {
        parent_stack.push(current_element.name);
      }
    }
    else if (nodeType == XmlNodeType_Text)
    {
      if (!parent_stack.empty())
      {
        const WCHAR* text_value = nullptr;

        xml_reader->GetValue(&text_value, nullptr);

        if (!text_value)
        {
          continue;
        }
        std::wstring_view value_view = text_value;
        temp_conversion.resize(::WideCharToMultiByte(CP_UTF8, 0, value_view.data(), value_view.size(), nullptr, 0, nullptr, nullptr));
        ::WideCharToMultiByte(CP_UTF8, 0, value_view.data(), (int)value_view.size(), temp_conversion.data(), (int)temp_conversion.size(), nullptr, nullptr);

        std::string_view parentName = parent_stack.top();

        auto expected_parents = parent_stack;
        expected_parents.pop();

        auto item = std::find_if(out_list.begin(), out_list.end(), [&](auto& element) {
          return element.name == parentName && element.parents == expected_parents;
        });

        if (item != out_list.end())
        {
          size_t content_pos = raw_xml.find(temp_conversion, item->name.data() - raw_xml.data());

          if (content_pos != std::string_view::npos)
          {
            item->content = raw_xml.substr(content_pos, temp_conversion.size());
          }
        }
      }
    }
    else if (nodeType == XmlNodeType_EndElement)
    {
      if (!parent_stack.empty())
      {
        parent_stack.pop();
      }
    }
  }

  for (auto& element : out_list)
  {
    auto start = element.name;
    auto end = element.content;

    if (end.empty())
    {
      auto max_attr = std::max_element(element.attributes.begin(), element.attributes.end(), [](auto& a, auto& b) {
        return a.first.data() < b.first.data();
      });

      if (max_attr != element.attributes.end())
      {
        end = max_attr->first;
      }
      else
      {
        end = start;
      }
    }

    auto start_index = raw_xml.rfind("<", start.data() - raw_xml.data());
    auto end_index = raw_xml.find(">", end.data() - raw_xml.data());

    if (start_index != std::string_view::npos && end_index != std::string_view::npos && end_index > start_index)
    {
      element.raw_line = raw_xml.substr(start_index, end_index - start_index + 1);
    }
  }

  return out_list;
}
