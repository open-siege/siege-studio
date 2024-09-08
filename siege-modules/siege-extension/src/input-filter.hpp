#ifndef INPUT_FILTER_HPP
#define INPUT_FILTER_HPP

#include <array>
#include <optional>
#include <type_traits>
#include <cstdint>

namespace siege
{
  struct active_input_state
  {
    struct group
    {
      std::uint32_t id = 0;
      std::uint32_t enabled = true;
      DWORD process_id = 0;
      DWORD thread_id = 0;
    };

    struct device
    {
      std::uint32_t type = -1;// keyboard, mouse, controller
      std::uint32_t id = 0;
      std::uint32_t group_id = 0;
      std::uint32_t enabled = true;
      HANDLE device_handle;
    };

    std::uint32_t size = sizeof(active_input_state);// could change in future versions

    std::array<group, 16> groups;// in real-life, most games only have hotseat or split-screen for 8 players. 16 should be enough.
    std::array<device, 64> devices;// you can then have 16 of each plus extra.
  };

  static_assert(std::is_trivially_copyable_v<active_input_state>);

  void init_active_input_state();

  bool update_device_id(const RAWINPUTDEVICELIST&);

  bool remove_device(HANDLE);

  std::optional<std::uint32_t> find_device_id(HANDLE);

  active_input_state& get_active_input_state();
}

#endif