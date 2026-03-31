#ifndef SIEGE_GAME_EXTENSION_MODULE_HPP
#define SIEGE_GAME_EXTENSION_MODULE_HPP

#include <siege/platform/extension_module.hpp>

namespace siege::platform
{
  using executable_is_supported = std::errc(const siege::fs_char* filename) noexcept;
  using get_function_name_ranges = std::errc(std::size_t, std::array<const char*, 2>*, std::size_t*) noexcept;
  using get_variable_name_ranges = std::errc(std::size_t, std::array<const char*, 2>*, std::size_t*) noexcept;
  using predefined_int = siege::platform::game_command_line_predefined_setting<int>;
  using predefined_string = siege::platform::game_command_line_predefined_setting<const wchar_t*>;
  using get_predefined_string_command_line_settings = predefined_string*(const siege::fs_char* name) noexcept;
  using get_predefined_int_command_line_settings = predefined_int*(const siege::fs_char* name) noexcept;
  using init_keyboard_inputs = std::errc(keyboard_binding* binding) noexcept;
  using init_mouse_inputs = std::errc(mouse_binding* binding) noexcept;
  using init_controller_inputs = std::errc(controller_binding* binding) noexcept;
  using default_controller_inputs = std::errc(controller_binding* binding, std::uint32_t layout_index) noexcept;

  using apply_prelaunch_settings = std::errc(const siege::fs_char* exe_path_str, siege::platform::game_command_line_args*);
  using format_command_line = const siege::fs_char**(const siege::platform::game_command_line_args*, std::uint32_t* new_size);

  using apply_prelaunch_settings_ex = std::errc(const siege::fs_char* exe_path_str, siege::platform::game_command_line_args_ex*);
  using format_command_line_ex = const siege::fs_char**(const siege::platform::game_command_line_args_ex*, std::uint32_t* new_size);
  using is_input_mapping_valid = std::errc(const siege::platform::hardware_context_caps* caps, const siege::platform::input_mapping_ex* mapping);

  // TODO Port this code to linux using a "platform::module" instead of a "win32::module"
  class game_extension_module : public win32::module
  {
    using base = win32::module;
    executable_is_supported* executable_is_supported_proc = nullptr;
    get_function_name_ranges* get_function_name_ranges_proc = nullptr;
    get_variable_name_ranges* get_variable_name_ranges_proc = nullptr;
    init_keyboard_inputs* init_keyboard_inputs_proc = nullptr;
    init_mouse_inputs* init_mouse_inputs_proc = nullptr;
    init_controller_inputs* init_controller_inputs_proc = nullptr;
    default_controller_inputs* default_controller_inputs_proc = nullptr;
    is_input_mapping_valid* is_input_mapping_valid_proc = nullptr;

    std::variant<std::monostate, apply_prelaunch_settings*, apply_prelaunch_settings_ex*> apply_prelaunch_settings_proc;
    std::variant<std::monostate, format_command_line*, format_command_line_ex*> format_command_line_proc;

  public:
    get_predefined_string_command_line_settings* get_predefined_string_command_line_settings_proc = nullptr;
    get_predefined_int_command_line_settings* get_predefined_int_command_line_settings_proc = nullptr;

    const std::span<game_action> game_actions;
    const std::span<const wchar_t*> controller_input_backends;

    const std::optional<game_command_line_caps> caps = std::nullopt;

    using base::base;

    game_extension_module(std::filesystem::path module_path) : base(module_path),
                                                               game_actions([this] {
                                                                 auto* actions = GetProcAddress<game_action*>("game_actions");

                                                                 if (!actions)
                                                                 {
                                                                   return std::span<game_action>();
                                                                 }

                                                                 auto size = 0;
                                                                 for (auto i = 0; i < 64; ++i)
                                                                 {
                                                                   if (actions[i].type == game_action::unknown)
                                                                   {
                                                                     size = i;
                                                                     break;
                                                                   }
                                                                 }

                                                                 return std::span(actions, size);
                                                               }()),
                                                               controller_input_backends(create_span("controller_input_backends")),
                                                               caps([this]() -> std::optional<game_command_line_caps> {
                                                                 auto raw_caps = GetProcAddress<game_command_line_caps*>("command_line_caps");

                                                                 if (raw_caps)
                                                                 {
                                                                   game_command_line_caps results{};
                                                                   auto size = raw_caps->caps_size < results.caps_size ? raw_caps->caps_size : results.caps_size;
                                                                   std::memcpy(&results, raw_caps, size);
                                                                   results.caps_size = size;
                                                                   return results;
                                                                 }

                                                                 return std::nullopt;
                                                               }())
    {
      executable_is_supported_proc = GetProcAddress<decltype(executable_is_supported_proc)>("executable_is_supported");
      get_function_name_ranges_proc = GetProcAddress<decltype(get_function_name_ranges_proc)>("get_function_name_ranges");
      get_variable_name_ranges_proc = GetProcAddress<decltype(get_variable_name_ranges_proc)>("get_variable_name_ranges");
      get_predefined_string_command_line_settings_proc = GetProcAddress<decltype(get_predefined_string_command_line_settings_proc)>("get_predefined_string_command_line_settings");
      get_predefined_int_command_line_settings_proc = GetProcAddress<decltype(get_predefined_int_command_line_settings_proc)>("get_predefined_int_command_line_settings");
      init_keyboard_inputs_proc = GetProcAddress<decltype(init_keyboard_inputs_proc)>("init_keyboard_inputs");
      init_mouse_inputs_proc = GetProcAddress<decltype(init_mouse_inputs_proc)>("init_mouse_inputs");
      init_controller_inputs_proc = GetProcAddress<decltype(init_controller_inputs_proc)>("init_controller_inputs");
      default_controller_inputs_proc = GetProcAddress<decltype(default_controller_inputs_proc)>("default_controller_inputs");

      is_input_mapping_valid_proc = GetProcAddress<decltype(is_input_mapping_valid_proc)>("is_input_mapping_valid");

      apply_prelaunch_settings_proc = [this] -> decltype(apply_prelaunch_settings_proc) {
        auto* ex_proc = GetProcAddress<apply_prelaunch_settings_ex*>("apply_prelaunch_settings_ex");

        if (ex_proc)
        {
          return ex_proc;
        }

        auto* proc = GetProcAddress<siege::platform::apply_prelaunch_settings*>("apply_prelaunch_settings");

        if (proc)
        {
          return proc;
        }

        return {};
      }();

      format_command_line_proc = [this] -> decltype(format_command_line_proc) {
        auto* ex_proc = GetProcAddress<format_command_line_ex*>("format_command_line_ex");

        if (ex_proc)
        {
          return ex_proc;
        }

        auto* proc = GetProcAddress<siege::platform::format_command_line*>("format_command_line");

        if (proc)
        {
          return proc;
        }

        return {};
      }();

      if (!this->executable_is_supported_proc)
      {
        throw std::runtime_error("Could not find module functions");
      }
    }

    inline bool is_input_mapping_valid(const siege::platform::hardware_context_caps& caps, const siege::platform::input_mapping_ex& mapping) const
    {
      if (!is_input_mapping_valid_proc)
      {
        return true;
      }

      return is_input_mapping_valid_proc(&caps, &mapping) == std::errc{};
    }

    inline std::optional<std::unique_ptr<siege::platform::controller_binding>> init_controller_inputs() const
    {
      if (init_controller_inputs_proc)
      {
        auto binding = std::make_unique<siege::platform::controller_binding>();

        if (init_controller_inputs_proc(binding.get()) != std::errc{})
        {
          return std::nullopt;
        }
        return binding;
      }

      if (default_controller_inputs_proc)
      {
        auto binding = std::make_unique<siege::platform::controller_binding>();

        if (default_controller_inputs_proc(binding.get(), 0) != std::errc{})
        {
          return std::nullopt;
        }
        return binding;
      }

      return std::nullopt;
    }

    inline std::optional<std::unique_ptr<siege::platform::keyboard_binding>>
      init_keyboard_inputs() const
    {
      if (!init_keyboard_inputs_proc)
      {
        return std::nullopt;
      }

      auto binding = std::make_unique<siege::platform::keyboard_binding>();

      if (init_keyboard_inputs_proc(binding.get()) != std::errc{})
      {
        return std::nullopt;
      }

      return binding;
    }

    inline std::optional<std::unique_ptr<siege::platform::mouse_binding>> init_mouse_inputs() const
    {
      if (!init_mouse_inputs_proc)
      {
        return std::nullopt;
      }

      auto binding = std::make_unique<siege::platform::mouse_binding>();

      if (init_mouse_inputs_proc(binding.get()) != std::errc{})
      {
        return std::nullopt;
      }

      return binding;
    }

    inline std::optional<bool> executable_is_supported(std::filesystem::path exe_path) const noexcept
    {
      if (executable_is_supported_proc)
      {
        return executable_is_supported_proc(exe_path.c_str()) == std::errc{};
      }

      return std::nullopt;
    }

    inline std::optional<bool> supports_format_command_line_ex() const noexcept
    {
      if (format_command_line_proc.index() == 0)
      {
        return std::nullopt;
      }

      return format_command_line_proc.index() == 2;
    }

    inline std::span<const siege::fs_char*> format_command_line(const siege::platform::game_command_line_args_ex& args) const noexcept
    {
      if (supports_format_command_line_ex() != true)
      {
        return {};
      }

      auto* proc = std::get<siege::platform::format_command_line_ex*>(format_command_line_proc);

      std::uint32_t size = 0;
      auto** result = proc(&args, &size);

      if (result == nullptr || size == 0)
      {
        return {};
      }

      return std::span<const siege::fs_char*>(result, size);
    }

    inline std::span<const siege::fs_char*> format_command_line(const siege::platform::game_command_line_args& args) const noexcept
    {
      if (supports_format_command_line_ex() != false)
      {
        return {};
      }

      auto* proc = std::get<siege::platform::format_command_line*>(format_command_line_proc);

      std::uint32_t size = 0;
      auto** result = proc(&args, &size);

      if (result == nullptr || size == 0)
      {
        return {};
      }

      return std::span<const siege::fs_char*>(result, size);
    }

    inline std::optional<bool> supports_apply_prelaunch_settings_ex() const noexcept
    {
      if (apply_prelaunch_settings_proc.index() == 0)
      {
        return std::nullopt;
      }

      return apply_prelaunch_settings_proc.index() == 2;
    }

    inline std::errc apply_prelaunch_settings(std::filesystem::path exe_path, siege::platform::game_command_line_args_ex args) const noexcept
    {
      if (supports_apply_prelaunch_settings_ex() != true)
      {
        return std::errc::not_supported;
      }

      auto* proc = std::get<siege::platform::apply_prelaunch_settings_ex*>(apply_prelaunch_settings_proc);
      return proc(exe_path.c_str(), &args);
    }

    inline std::errc apply_prelaunch_settings(std::filesystem::path exe_path, siege::platform::game_command_line_args& args) const noexcept
    {
      if (supports_apply_prelaunch_settings_ex() != false)
      {
        return std::errc::not_supported;
      }

      auto* proc = std::get<siege::platform::apply_prelaunch_settings*>(apply_prelaunch_settings_proc);
      return proc(exe_path.c_str(), &args);
    }

    static std::list<game_extension_module> load_modules(std::filesystem::path search_path, std::move_only_function<bool(const std::filesystem::path&)> sort_condition = nullptr, std::move_only_function<bool(const game_extension_module&)> condition = nullptr)
    {
      std::list<game_extension_module> loaded_modules;

      std::vector<std::filesystem::path> dll_paths;

      for (auto const& dir_entry : std::filesystem::directory_iterator{ search_path })
      {
        if ((dir_entry.path().extension() == ".dll" || dir_entry.path().extension() == ".DLL")
            && dir_entry.path().stem().wstring().find(L"siege-extension") != std::wstring::npos)
        {
          dll_paths.emplace_back(dir_entry.path());
        }
      }

      if (sort_condition)
      {
        std::stable_partition(dll_paths.begin(), dll_paths.end(), std::move(sort_condition));
      }

      std::for_each(dll_paths.begin(), dll_paths.end(), [&loaded_modules, condition = std::move(condition)](auto path) mutable {
        try
        {
          game_extension_module temp{ path };
          if (condition && !condition(temp))
          {
            return;
          }

          loaded_modules.emplace_back(std::move(temp));
        }
        catch (...)
        {
        }
      });

      return loaded_modules;
    }

    inline bool is_named_export(void* export_ptr) const
    {
      if (!export_ptr)
      {
        return false;
      }

      return export_ptr == executable_is_supported_proc || export_ptr == get_function_name_ranges_proc || export_ptr == get_variable_name_ranges_proc || export_ptr == init_keyboard_inputs_proc || export_ptr == init_mouse_inputs_proc || export_ptr == get_predefined_string_command_line_settings_proc || export_ptr == get_predefined_int_command_line_settings_proc || export_ptr == init_controller_inputs_proc || export_ptr == GetProcAddress<game_command_line_caps*>("command_line_caps") || export_ptr == GetProcAddress<game_command_line_caps*>("controller_input_backends") || export_ptr == GetProcAddress<game_command_line_caps*>("apply_prelaunch_settings") || export_ptr == GetProcAddress<game_command_line_caps*>("game_actions") || export_ptr == GetProcAddress<game_command_line_caps*>("format_command_line");
    }

    inline std::vector<std::pair<std::string, std::string>> get_function_name_ranges() const
    {
      std::vector<std::pair<std::string, std::string>> results;

      if (get_function_name_ranges_proc)
      {
        std::size_t count;

        if (auto hresult = get_function_name_ranges_proc(0, nullptr, &count); hresult == std::errc{})
        {
          std::vector<std::array<const char*, 2>> raw;
          raw.resize(count);

          if (hresult = get_function_name_ranges_proc(raw.size(), raw.data(), &count); hresult == std::errc{})
          {
            results.reserve(raw.size());

            std::transform(raw.begin(), raw.end(), std::back_inserter(results), [](auto& item) {
              return std::make_pair<std::string, std::string>(item[0] ? item[0] : "", item[1] ? item[1] : "");
            });
          }
        }
      }
      return results;
    }

    inline std::vector<std::pair<std::string, std::string>> get_variable_name_ranges() const
    {
      std::vector<std::pair<std::string, std::string>> results;

      if (get_variable_name_ranges_proc)
      {
        std::size_t count;

        if (auto hresult = get_variable_name_ranges_proc(0, nullptr, &count); hresult == std::errc{})
        {
          std::vector<std::array<const char*, 2>> raw;
          raw.resize(count);

          if (hresult = get_variable_name_ranges_proc(raw.size(), raw.data(), &count); hresult == std::errc{})
          {
            results.reserve(raw.size());

            for (auto& item : raw)
            {
              if (!(item[0] || item[1]))
              {
                break;
              }
              results.emplace_back(std::make_pair<std::string, std::string>(item[0], item[1]));
            }
          }
        }
      }
      return results;
    }

  private:
    inline std::span<const wchar_t*> create_span(const char* key) const
    {
      auto* storage = this->GetProcAddress<const wchar_t**>(key);

      if (storage)
      {
        int end = 0;

        for (auto i = 0; i < 64; ++i)
        {
          if (storage[i] == nullptr)
          {
            end = i;
            break;
          }
        }

        return std::span<const wchar_t*>(storage, end);
      }
      return std::span<const wchar_t*>();
    }
  };

}// namespace siege::platform

#endif// !SIEGE_EXTENSION_MODULE_HPP
