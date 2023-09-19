#include <limits>
#include "configurations/jedi.hpp"
#include "endian_arithmetic.hpp"

namespace studio::configurations::jedi
{
    namespace endian = boost::endian;
    namespace dark_forces
    {
        
        constexpr auto static expected_header = std::array<char, 4> {'C', 'F', 'G', 'd'};
        constexpr auto static checksum = std::array<endian::little_int32_t, 4> = {
                60,
                37,
                84,
                57
            };
 
        constexpr static auto unused = std::string_view{"Unused"};

            constexpr static auto keyboardActions = std::array<std::string_view, 25>
            {
                std::string_view{"Keyboard/TurnRight"},
                std::string_view{"Keyboard/TurnLeft"},
                std::string_view{"Keyboard/MoveForward"},
                std::string_view{"Keyboard/MoveBackward"},
                std::string_view{"Keyboard/SlowMode"},
                std::string_view{"Keyboard/FastMode"},
                std::string_view{"Keyboard/PrimaryFire"},
                std::string_view{"Keyboard/SecondaryFire"},
                std::string_view{"Keyboard/StrafeLeft"},
                std::string_view{"Keyboard/StrafeRight"},
                unused,
                unused,
                std::string_view{"Keyboard/Crouch"},
                std::string_view{"Keyboard/Use"},
                std::string_view{"Keyboard/Jump"},
                unused,
                std::string_view{"Keyboard/StrafeMode"},
                unused,
                unused,
                unused,
                unused,
                std::string_view{"Keyboard/ViewUp"},
                std::string_view{"Keyboard/ViewDown"},
                unused,
                unused
            };

            
            constexpr static auto joystickActions = std::array<std::string_view, 16>
            {
                std::string_view{"Joystick/Axis/TurningLeftRight"},
                std::string_view{"Joystick/Axis/MovingForwardBackward"},
                unused,
                std::string_view{"Joystick/Axis/StrafingLeftRight"},
                std::string_view{"Joystick/Button/Crouch"},
                std::string_view{"Joystick/Button/Nudge"},
                std::string_view{"Joystick/Button/Jump"},
                unused,
                std::string_view{"Joystick/Button/Strafe"},
                unused,
                std::string_view{"Joystick/Button/FirePrimary"},
                std::string_view{"Joystick/Button/FireSecondary"},
                std::string_view{"Joystick/Button/SpeedMode"},
                std::string_view{"Joystick/Button/SlowMode"},
                std::string_view{"Joystick/Axis/ViewUpDown"},
                unused
            };


            constexpr static auto mouseActions = std::array<std::string_view, 16>
            {
                std::string_view{"Mouse/Axis/TurningLeftRight"},
                std::string_view{"Mouse/Axis/MovingForwardBackward"},
                std::string_view{"Mouse/Button/Forward"},
                std::string_view{"Mouse/Button/Backward"},
                unused,
                std::string_view{"Mouse/Button/Crouch"},
                std::string_view{"Mouse/Button/Nudge"},
                std::string_view{"Mouse/Button/Jump"},
                unused,
                std::string_view{"Mouse/Button/StrafeMode"},
                unused,
                std::string_view{"Mouse/Button/PrimaryFire"},
                std::string_view{"Mouse/Button/SpeedMode"},
                std::string_view{"Mouse/Button/SlowMode"},
                unused,
                unused
            };

        std::optional<game_config> load_config(std::istream& raw_data, std::size_t stream_size)
        {
            std::array<char, 4> header{};

            raw_data.read(header.data(), sizeof(header));
            raw_data.seekg(-int(sizeof(header)), std::ios::cur);

            if (header != expected_header)
            {
                return std::nullopt;
            }

            game_config result{};
            result.raw_data.reserve(stream_size);
            auto current_pos = raw_data.tellg();
            raw_data.read(result.raw_data.data(), result.raw_data.size());
            raw_data.seekg(current_pos, std::ios::beg);

            result.add("__ConfigType", "dark_forces");
            
            // space we don't use
            raw_data.seekg(header.size(), std::ios::cur);
            raw_data.seekg(checksum.size(), std::ios::cur);
            raw_data.seekg(5, std::ios::cur);

            little_endian_stream_reader reader{ raw_data };

            result.add("ShowHud", reader.read_uint32() == std::numeric_limits<std::uint32_t>::max());
            result.add("InternalResolution", reader.read_uint32());
            result.add("GeometryDetail", reader.read_uint32());
            result.add("ScreenSize", reader.read_uint32());
            raw_data.seekg(2, std::ios::cur);

            result.add("JoystickEnabled", reader.read_uint8());
            result.add("EffectsVolume", reader.read_uint32());
            result.add("MusicVolume", reader.read_uint32());
            result.add("MouseSensitivity", reader.read_uint32());
            result.add("GammaCorrection", reader.read_uint32());
            result.add("SuperShield", reader.read_uint32() == 1);
            result.add("NumChannels", reader.read_uint32());
            result.add("AutoSwapWeapon", reader.read_uint32() == 1);
            result.add("CutsceneEffectsVolume", reader.read_uint32());
            result.add("CutsceneMusicVolume", reader.read_uint32());
            raw_data.seekg(1, std::ios::cur);   // DB
            raw_data.seekg(24, std::ios::cur);  // FFx24
            raw_data.seekg(sizeof(std::array<std::int32_t, 3>), std::ios::cur);

            result.add("JoystickType", reader.read_uint32());
            result.add("HatInfo", reader.read_uint32());            
            raw_data.seekg(4, std::ios::cur);
            result.add("Joy1CalibrationCentre", reader.read_uint32());
            result.add("Joy1TopLeft", reader.read_uint32());
            result.add("Joy1BottomRight", reader.read_uint32());
            raw_data.seekg(sizeof(std::array<std::int32_t, 3>), std::ios::cur);

            result.add("Joy2CalibrationCentre", reader.read_uint32());
            result.add("Joy2TopLeft", reader.read_uint32());
            result.add("Joy2BottomRight", reader.read_uint32());
            raw_data.seekg(32, std::ios::cur);
            result.add("IsCalibrated", reader.read_uint32() == std::numeric_limits<std::uint32_t>::max());

            for (auto action : keyboardActions)
            {
                if (action == unused)
                {
                    raw_data.seekg(1, std::ios::cur); 
                }
                else
                {
                    result.add(action, reader.read_uint8());
                }
            }


            for (auto action : joystickActions)
            {
                if (action == unused)
                {
                    raw_data.seekg(1, std::ios::cur); 
                }
                else
                {
                    result.add(action, reader.read_uint8());
                }
            }

            for (auto action : mouseActions)
            {
                if (action == unused)
                {
                    raw_data.seekg(1, std::ios::cur); 
                }
                else
                {
                    result.add(action, reader.read_uint8());
                }
            }

            return result;
        }

        void save_config(std::ostream& raw_data, const game_config& config)
        {
            auto type = config.find_to_string("ConfigType");

            if (type != "DarkForces")
            {
                return;    
            }

            little_endian_stream_writer writer{raw_data};

            writer.write(std::string_view(expected_header.data(), expected_header.size()));
            raw_data.write(reinterpret_cast<char*>(checksum.data()), sizeof(checksum));

            writer.write(std::string(5, '\0'));
            writer.write(config.find_bool("ShowHud").value_or(true) ? std::numeric_limits<std::uint32_t>::max() : 0u);
            writer.write(config.find_uint32_t("InternalResolution").value_or(0));
            writer.write(config.find_uint32_t("GeometryDetail").value_or(0));
            writer.write(config.find_uint32_t("ScreenSize").value_or(0));
            writer.write(std::string(2, '\0'));
            writer.write(config.find_uint8_t("JoystickEnabled").value_or(0));
            writer.write(config.find_uint32_t("EffectsVolume").value_or(0));
            writer.write(config.find_uint32_t("MusicVolume").value_or(0));
            writer.write(config.find_uint32_t("MouseSensitivity").value_or(0));
            writer.write(config.find_uint32_t("GammaCorrection").value_or(0));
            writer.write(config.find_bool("SuperShield").value_or(false) ? std::uint32_t(1) : std::uint32_t(0));
            writer.write(config.find_uint32_t("NumChannels").value_or(0));
            writer.write(config.find_bool("AutoSwapWeapon").value_or(false) ? std::uint32_t(1) : std::uint32_t(0));
            writer.write(config.find_uint32_t("CutsceneEffectsVolume").value_or(0));
            writer.write(config.find_uint32_t("CutsceneMusicVolume").value_or(0));

            writer.write(std::uint8_t(0xDB));
            writer.write(std::string(24, 0xFF));
            writer.write(std::string(sizeof(std::array<std::int32_t, 3>), '\0'));          
            writer.write(config.find_uint32_t("JoystickType").value_or(0));
            writer.write(config.find_uint32_t("HatInfo").value_or(0));
            writer.write(std::uint32_t(0));
            writer.write(config.find_uint32_t("Joy1CalibrationCentre").value_or(0));
            writer.write(config.find_uint32_t("Joy1TopLeft").value_or(0));
            writer.write(config.find_uint32_t("Joy1BottomRight").value_or(0));
            writer.write(std::string(sizeof(std::array<std::int32_t, 3>), '\0'));
            writer.write(config.find_uint32_t("Joy2CalibrationCentre").value_or(0));
            writer.write(config.find_uint32_t("Joy2TopLeft").value_or(0));
            writer.write(config.find_uint32_t("Joy2BottomRight").value_or(0));
            writer.write(std::string(32, 0xFF));
            writer.write(config.find_bool("IsCalibrated").value_or(false) ? std::numeric_limits<std::uint32_t>::max() : 0u);
        
            for (auto action : keyboardActions)
            {
                if (action == unused)
                {
                    writer.write(std::uint8_t(0x00));
                }
                else
                {
                    writer.write(config.find_uint8_t(action).value_or(0));
                }
            }

            for (auto action : joystickActions)
            {
                if (action == unused)
                {
                    writer.write(std::uint8_t(0x00));
                }
                else
                {
                    writer.write(config.find_uint8_t(action).value_or(0));
                }
            }

            for (auto action : mouseActions)
            {
                if (action == unused)
                {
                    writer.write(std::uint8_t(0x00));
                }
                else
                {
                    writer.write(config.find_uint8_t(action).value_or(0));
                }
            }
        }
    }
}