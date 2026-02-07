#ifndef ID_TECH_CONFIG_HPP
#define ID_TECH_CONFIG_HPP

#include <siege/configuration/shared.hpp>
#include <istream>
#include <any>

namespace siege::configuration::id_tech
{
  namespace id_tech_0_0
  {
    namespace wolf3d
    {
      std::optional<std::any> load_config(std::istream&, std::size_t);
      void save_config(const std::any&, std::ostream& raw_data);
    }// namespace wolf3d

    namespace blake_stone
    {
      std::optional<std::any> load_config(std::istream&, std::size_t);
      void save_config(const std::any&, std::ostream& raw_data);
    }// namespace blake_stone

    namespace corridor_7
    {
      std::optional<std::any> load_config(std::istream&, std::size_t);
      void save_config(const std::any&, std::ostream& raw_data);
    }// namespace corridor_7

    namespace body_count
    {
      std::optional<std::any> load_config(std::istream&, std::size_t);
      void save_config(const std::any&, std::ostream& raw_data);
    }// namespace body_count
  }// namespace id_tech_0_0

  namespace id_tech_0_5
  {
    namespace shadowcaster
    {
      std::optional<std::any> load_config(std::istream&, std::size_t);
      void save_config(const std::any&, std::ostream& raw_data);
    }// namespace shadowcaster

    namespace cyclones
    {
      std::optional<std::any> load_config(std::istream&, std::size_t);
      void save_config(const std::any&, std::ostream& raw_data);
    }// namespace cyclones

    namespace rott
    {
      std::optional<std::any> load_config(std::istream&, std::size_t);
      void save_config(const std::any&, std::ostream& raw_data);
    }// namespace rott

    namespace greed
    {
      std::optional<std::any> load_config(std::istream&, std::size_t);
      void save_config(const std::any&, std::ostream& raw_data);
    }// namespace greed
  }// namespace id_tech_0_5

  namespace id_tech_1
  {
    std::optional<text_game_config> load_config(std::istream&, std::size_t);
    void save_config(const std::vector<text_game_config::config_line>& entries, std::ostream& raw_data);
  }// namespace id_tech_1

  namespace id_tech_2
  {
    std::optional<text_game_config> load_config(std::istream&, std::size_t);
    void save_config(const std::vector<text_game_config::config_line>& entries, std::ostream& raw_data);
  }// namespace id_tech_2
}// namespace siege::configuration::id_tech

#endif