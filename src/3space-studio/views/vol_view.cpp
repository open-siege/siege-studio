#include "vol_view.hpp"
#include "archives/file_system_archive.hpp"

vol_view::vol_view(const shared::archive::file_info& info, const studio::fs::file_system_archive& archive)
{
  files = archive.find_files(info.folder_path / info.filename, { "ALL" });
}

void vol_view::setup_gl(wxWindow* parent, sf::RenderWindow* window, ImGuiContext* guiContext)
{
}