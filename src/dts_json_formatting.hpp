#ifndef DARKSTARDTSCONVERTER_DTS_JSON_FORMATTING_HPP
#define DARKSTARDTSCONVERTER_DTS_JSON_FORMATTING_HPP

#include <map>
#include <bitset>
#include "complex_serializer.hpp"
#include "json_boost.hpp"

void format_json(nlohmann::ordered_json& item_as_json)
{
  auto sequences = nlohmann::json::object();

  for (auto& sequence : item_as_json["sequences"])
  {
    auto sequence_name = item_as_json["names"][sequence["nameIndex"].get<int>()].get<std::string>();
    sequence["name"] = sequence_name;
    sequences[sequence_name] = sequence;
  }

  auto nodes = nlohmann::json::object();
  auto transform_keyframe_mapping = std::map<int, std::tuple<std::string, std::string, int>>{};

  for (auto& node : item_as_json["nodes"])
  {
    auto parent_index = node["parentNodeIndex"].get<int>();

    if (parent_index != -1)
    {
      auto& parent_node = item_as_json["nodes"][parent_index];

      auto parent_name = item_as_json["names"][parent_node["nameIndex"].get<int>()].get<std::string>();
      node["parentNodeIndex"] = parent_name;
    }
    else
    {
      node["parentNodeIndex"] = nullptr;
    }
    std::string name = item_as_json["names"][node["nameIndex"].get<int>()].get<std::string>();
    node["name"] = name;


    nlohmann::json element = node;

    element["objectIndexes"] = nlohmann::json::array();

    element["sequences"] = nlohmann::json::object();

    auto& sub_sequences = item_as_json["subSequences"];

    const auto default_transform_index = element["defaultTransformIndex"].get<int>();
    const auto num_sub_sequences = element["numSubSequences"].get<int>();
    const auto first_sub_sequence = element["firstSubSequenceIndex"].get<int>();
    for (auto i = first_sub_sequence; i < first_sub_sequence + num_sub_sequences; ++i)
    {
      auto& local_sub_sequence = sub_sequences[i];
      auto& sequence = item_as_json["sequences"][local_sub_sequence["sequenceIndex"].get<int>()];
      auto sequence_name = sequence["name"].get<std::string>();

      auto& key_frames = element["subSequences"][sequence_name] = nlohmann::json::array();
      const auto first_key_frame = local_sub_sequence["firstKeyFrameIndex"].get<int>();
      const auto num_key_frames = local_sub_sequence["numKeyFrames"].get<int>();

      auto& keyframes = item_as_json["keyframes"];
      for (auto j = first_key_frame; j < first_key_frame + num_key_frames; ++j)
      {
        auto& keyframe = keyframes[j];
        keyframe["keyframeIndex"] = j;
        auto transform_index = keyframe["transformIndex"].get<int>();

        if (!element.contains("defaultTransformationSequence") && default_transform_index == transform_index)
        {
          element["defaultTransformationSequence"] = sequence_name;
          element["defaultTransformationFrameIndex"] = key_frames.size();
        }

        transform_keyframe_mapping.emplace(transform_index, std::make_tuple(name, sequence_name, key_frames.size()));

        auto& transform = item_as_json["transforms"][transform_index];

        keyframe["transformation"] = transform;

        const auto raw_flags = keyframe["matIndex"].get<std::uint32_t>();
        std::bitset<sizeof(std::uint32_t) * 8> flags{ raw_flags };

        {
          auto flags_as_string = flags.to_string();
          std::reverse(flags_as_string.begin(), flags_as_string.end());
          flags = decltype(flags){ flags_as_string };
        };

        keyframe["matIndex"] = flags.to_string();
        keyframe["isVisible"] = bool(flags[0]);
        keyframe["doesVisibilityMatter"] = bool(flags[1]);
        keyframe["doesMaterialMatter"] = bool(flags[2]);
        keyframe["doesFrameMatter"] = bool(flags[3]);


        key_frames.emplace_back(keyframe);
      }
    }

    if (!element.contains("defaultTransformationSequence"))
    {
      element["defaultTransformation"] = item_as_json["transforms"][default_transform_index];
    }

    //element.erase("defaultTransformIndex");

    nodes.emplace(name, element);
  }

  auto objects = nlohmann::json::object();
  auto meshes = nlohmann::json::object();

  for (auto& object : item_as_json["objects"])
  {
    auto name = item_as_json["names"][object["nameIndex"].get<int>()].get<std::string>();
    object["name"] = name;

    meshes.emplace(name, item_as_json["meshes"][object["meshIndex"].get<int>()]);

    if (nodes.contains(name))
    {
      auto& node = nodes[name];
      object["parentObjectIndex"] = node["parentNodeIndex"];
      object["nodeIndex"] = name;
      node["objectIndexes"].emplace_back(name);
    }


    nlohmann::json element = object;
    element["sequences"] = nlohmann::json::object();

    auto& sub_sequences = item_as_json["subSequences"];
    const auto num_sub_sequences = element["numSubSequences"].get<int>();
    const auto first_sub_sequence = element["firstSubSequenceIndex"].get<int>();
    for (auto i = first_sub_sequence; i < first_sub_sequence + num_sub_sequences; ++i)
    {
      auto& local_sub_sequence = sub_sequences[i];
      auto& sequence = item_as_json["sequences"][local_sub_sequence["sequenceIndex"].get<int>()];
      auto sequence_name = sequence["name"].get<std::string>();

      auto& key_frames = element["subSequences"][sequence_name] = nlohmann::json::array();
      const auto first_key_frame = local_sub_sequence["firstKeyFrameIndex"].get<int>();
      const auto num_key_frames = local_sub_sequence["numKeyFrames"].get<int>();

      auto& keyframes = item_as_json["keyframes"];
      for (auto j = first_key_frame; j < first_key_frame + num_key_frames; ++j)
      {
        nlohmann::json keyframe = keyframes[j];
        keyframe["keyframeIndex"] = j;
        auto transform_index = keyframe["transformIndex"].get<int>();

        for (const auto& [key, node] : nodes.items())
        {
          if (node["defaultTransformIndex"].get<int>() == transform_index)
          {
            keyframe["transformationNodeIndex"] = key;
            break;
          }
        }

        if (!keyframe.contains("transformationNodeIndex"))
        {
          if (const auto& result = transform_keyframe_mapping.find(transform_index); result != transform_keyframe_mapping.end())
          {
            const auto& [node_name, other_sequence_name, key_frame_index] = result->second;
            keyframe["transformationNodeIndex"] = node_name;
            keyframe["transformationSequenceIndex"] = other_sequence_name;
            keyframe["transformationFrameIndex"] = key_frame_index;
          }
          else
          {
            auto& transform = item_as_json["transforms"][transform_index];

            keyframe["transformation"] = transform;
          }
        }

        const auto raw_flags = keyframe["matIndex"].get<std::uint32_t>();
        std::bitset<sizeof(std::uint32_t) * 8> flags{ raw_flags };

        {
          auto flags_as_string = flags.to_string();
          std::reverse(flags_as_string.begin(), flags_as_string.end());
          flags = decltype(flags){ flags_as_string };
        };

        keyframe["matIndex"] = flags.to_string();
        keyframe["isVisible"] = bool(flags[0]);
        keyframe["doesVisibilityMatter"] = bool(flags[1]);
        keyframe["doesMaterialMatter"] = bool(flags[2]);
        keyframe["doesFrameMatter"] = bool(flags[3]);

        key_frames.emplace_back(keyframe);
      }
    }
    objects.emplace(name, element);
  }

  for (auto& element : item_as_json["details"])
  {
    auto& node = item_as_json["nodes"][element["rootNodeIndex"].get<int>()];

    element["rootNodeIndex"] = node["name"];
  }

  auto count = 0u;
  for (auto& element : item_as_json["transitions"])
  {
    const auto start_index = element["startSequenceIndex"].get<int>();
    const auto end_index = element["endSequenceIndex"].get<int>();

    element["startSequenceIndex"] = item_as_json["sequences"][start_index]["name"];
    element["endSequenceIndex"] = item_as_json["sequences"][end_index]["name"];

    std::stringstream possible_name;
    possible_name << "Trans";

    if (count < 10)
    {
      possible_name << '0' << count;
    }
    else
    {
      possible_name << count;
    }

    for (auto& name : item_as_json["names"])
    {
      if (const auto raw_name = name.get<std::string>(); raw_name.rfind(possible_name.str(), 0) == 0)
      {
        element["name"] = raw_name;
      }
    }
    count++;
  }

  for (auto& element : item_as_json["materialList"]["materials"])
  {
    const auto raw_flags = element["flags"].get<std::uint32_t>();

    const std::bitset<sizeof(std::uint32_t) * 8> flags{ raw_flags };

    element["flags"] = flags.to_string();
  }

  item_as_json["nodes"] = nodes;
  item_as_json["objects"] = objects;
  item_as_json["sequences"] = sequences;
  item_as_json["meshes"] = meshes;
  item_as_json.erase("transforms");
  item_as_json.erase("keyframes");
  item_as_json.erase("subSequences");
  item_as_json.erase("names");
  item_as_json.erase("header");
}

#endif//DARKSTARDTSCONVERTER_DTS_JSON_FORMATTING_HPP
