#include <sstream>
#include <execution>
#include <algorithm>
#include "sfx_view.hpp"
#include "content/sfx/wave.hpp"

namespace studio::views
{
  std::filesystem::path sfx_view::export_path = std::filesystem::path();

  sfx_view::sfx_view(studio::resources::file_info info, std::istream& image_stream, const studio::resources::resource_explorer& explorer)
    : explorer(explorer), info(info)
  {
    if (export_path == std::filesystem::path())
    {
      export_path = explorer.get_search_path() / "exported";
    }

    if (studio::content::sfx::is_sfx_file(image_stream))
    {
      std::stringstream mem_buffer;
      std::vector<std::byte> data(info.size);
      image_stream.read(reinterpret_cast<char*>(data.data()), info.size);

      content::sfx::write_wav_data(mem_buffer, data);

      original_data = mem_buffer.str();

      buffer.loadFromMemory(original_data.data(), original_data.size());
    }
    else if (studio::content::sfx::is_wav_file(image_stream) || studio::content::sfx::is_ogg_file(image_stream) || studio::content::sfx::is_flac_file(image_stream))
    {
      std::stringstream mem_buffer;

      std::copy_n(std::istreambuf_iterator(image_stream),
        info.size,
        std::ostreambuf_iterator(mem_buffer));

      original_data = mem_buffer.str();
      buffer.loadFromMemory(original_data.data(), original_data.size());
    }
  }

  void sfx_view::setup_view(wxWindow& parent, sf::RenderWindow& window, ImGuiContext& guiContext)
  {
    sound.setBuffer(buffer);
  }

  void sfx_view::render_ui(wxWindow& parent, sf::RenderWindow& window, ImGuiContext& guiContext)
  {
    window.clear();

    ImGui::Begin("File Info");

    ImGui::LabelText("", "Duration (in seconds): %f", buffer.getDuration().asSeconds());
    ImGui::LabelText("", "Sample Rate: %u", buffer.getSampleRate());


    ImGui::End();

    ImGui::Begin("Export Options");

    if (ImGui::Button("Set Export Directory"))
    {
      auto dialog = std::make_unique<wxDirDialog>(nullptr, "Open a folder to export files to");

      if (dialog->ShowModal() == wxID_OK)
      {
        export_path = dialog->GetPath().c_str().AsChar();
      }
    }

    ImGui::SameLine();
    ImGui::Text("%s", export_path.string().c_str());

    if (ImGui::Button("Export to WAV"))
    {
      auto new_file_name = info.filename.replace_extension(".wav");
      std::filesystem::create_directories(export_path);

      std::ofstream output(export_path / new_file_name, std::ios::binary);
      output.write(original_data.data(), original_data.size());
      if (!opened_folder)
      {
        wxLaunchDefaultApplication(export_path.string());
        opened_folder = true;
      }
    }

    if (ImGui::Button("Export All SFX files to WAV"))
    {
      auto files = explorer.find_files({ ".sfx" });

      if (!opened_folder && !files.empty())
      {
        wxLaunchDefaultApplication(export_path.string());
        opened_folder = true;
      }

      std::for_each(std::execution::par_unseq, files.begin(), files.end(), [=](const auto& snd_info) {
        auto sound_stream = explorer.load_file(snd_info);

        if (content::sfx::is_sfx_file(*sound_stream.second))
        {
          const auto new_path = export_path / std::filesystem::relative(snd_info.folder_path, explorer.get_search_path());
          auto new_file_name = std::filesystem::path(snd_info.filename).replace_extension(".wav");

          std::filesystem::create_directories(new_path);
          std::ofstream output(new_path / new_file_name, std::ios::binary);

          content::sfx::write_wav_header(output, snd_info.size);
          std::copy_n(std::istreambuf_iterator(*sound_stream.second),
                      snd_info.size,
                      std::ostreambuf_iterator(output));
        }
      });
    }

    ImGui::End();

    ImGui::Begin("Playback");

    if (ImGui::Button("Play"))
    {
      sound.play();
    }

    ImGui::SameLine();

    if (ImGui::Button("Pause"))
    {
      sound.pause();
    }

    ImGui::SameLine();

    auto loop = sound.getLoop();

    if (ImGui::Checkbox("Loop", &loop))
    {
      sound.setLoop(loop);
    }

    ImGui::End();
  }
}// namespace studio::views