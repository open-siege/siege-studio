#include <limits>
#include <siege/configuration/jedi.hpp>
#include <siege/platform/endian_arithmetic.hpp>

namespace siege::configuration::jedi
{
    using config_entry = binary_game_config::config_entry;
    namespace endian = siege::platform;

    namespace jedi_knight
    {
        std::optional<text_game_config> load_config(std::istream&, std::size_t)
        {
            return std::nullopt;
        }

        void save_config(const std::vector<text_game_config::config_line>&, std::ostream&)
        {

        }
    }

    namespace racer
    {
        std::optional<text_game_config> load_config(std::istream&, std::size_t)
        {
            return std::nullopt;
        }

        void save_config(const std::vector<text_game_config::config_line>&, std::ostream&)
        {

        }
    }

    namespace dark_forces
    {
        
        constexpr static auto expected_header = std::array<char, 4> {'C', 'F', 'G', 'd'};
        const static auto checksum = std::array<endian::little_int32_t, 4> {
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

        std::optional<binary_game_config> load_config(std::istream& raw_data, std::size_t stream_size)
        {
            std::array<char, 4> header{};

            raw_data.read(header.data(), sizeof(header));
            raw_data.seekg(-int(sizeof(header)), std::ios::cur);

            if (header != expected_header)
            {
                return std::nullopt;
            }

            auto current_pos = raw_data.tellg();

            std::vector<config_entry> results;
            results.push_back({"__ConfigType", config_value("dark_forces")});
            
            // space we don't use
            raw_data.seekg(header.size(), std::ios::cur);
            raw_data.seekg(checksum.size(), std::ios::cur);
            raw_data.seekg(5, std::ios::cur);

            little_endian_stream_reader reader{ raw_data };

            results.push_back({"ShowHud", config_value(reader.read_uint32() == std::numeric_limits<std::uint32_t>::max())});
            results.push_back({"InternalResolution", config_value(reader.read_uint32())});
            results.push_back({"GeometryDetail", config_value(reader.read_uint32())});
            results.push_back({"ScreenSize", config_value(reader.read_uint32())});
            raw_data.seekg(2, std::ios::cur);

            results.push_back({"JoystickEnabled", config_value(reader.read_uint8())});
            results.push_back({"EffectsVolume", config_value(reader.read_uint32())});
            results.push_back({"MusicVolume", config_value(reader.read_uint32())});
            results.push_back({"MouseSensitivity", config_value(reader.read_uint32())});
            results.push_back({"GammaCorrection", config_value(reader.read_uint32())});
            results.push_back({"SuperShield", config_value(reader.read_uint32() == 1)});
            results.push_back({"NumChannels", config_value(reader.read_uint32())});
            results.push_back({"AutoSwapWeapon", config_value(reader.read_uint32() == 1)});
            results.push_back({"CutsceneEffectsVolume", config_value(reader.read_uint32())});
            results.push_back({"CutsceneMusicVolume", config_value(reader.read_uint32())});
            raw_data.seekg(1, std::ios::cur);   // DB
            raw_data.seekg(24, std::ios::cur);  // FFx24
            raw_data.seekg(sizeof(std::array<std::int32_t, 3>), std::ios::cur);

            results.push_back({"JoystickType", config_value(reader.read_uint32())});
            results.push_back({"HatInfo", config_value(reader.read_uint32())});         
            raw_data.seekg(4, std::ios::cur);
            results.push_back({"Joy1CalibrationCentre", config_value(reader.read_uint32())});
            results.push_back({"Joy1TopLeft", config_value(reader.read_uint32())});
            results.push_back({"Joy1BottomRight", config_value(reader.read_uint32())});
            raw_data.seekg(sizeof(std::array<std::int32_t, 3>), std::ios::cur);

            results.push_back({"Joy2CalibrationCentre", config_value(reader.read_uint32())});
            results.push_back({"Joy2TopLeft", config_value(reader.read_uint32())});
            results.push_back({"Joy2BottomRight", config_value(reader.read_uint32())});
            raw_data.seekg(32, std::ios::cur);
            results.push_back({"IsCalibrated", config_value(reader.read_uint32() == std::numeric_limits<std::uint32_t>::max())});

            for (auto action : keyboardActions)
            {
                if (action == unused)
                {
                    raw_data.seekg(1, std::ios::cur); 
                }
                else
                {
                    results.push_back({action, config_value(reader.read_uint8())});
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
                    results.push_back({action, config_value(reader.read_uint8())});
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
                    results.push_back({action, config_value(reader.read_uint8())});
                }
            }


            return std::make_optional<binary_game_config>(std::move(results), save_config);
        }

        void save_config(const std::vector<binary_game_config::config_entry>& entries, std::ostream& raw_data)
        {
            // auto type = config.find("ConfigType");

            // if (type != config_value("DarkForces"))
            // {
            //     return;    
            // }

            // config_value zero(std::uint32_t(0u));
            // little_endian_stream_writer writer{raw_data};

            // writer.write(std::string_view(expected_header.data(), expected_header.size()));
            // raw_data.write(reinterpret_cast<const char*>(checksum.data()), sizeof(checksum));

            // writer.write(std::string(5, '\0'));
            // write(writer, config.find("ShowHud").value_or(config_value(true)) ? config_value(std::numeric_limits<std::uint32_t>::max()) : zero);
            
            

            // write(writer, config.find("InternalResolution").value_or(zero));
            // write(writer, config.find("GeometryDetail").value_or(zero));
            // write(writer, config.find("ScreenSize").value_or(zero));
            // writer.write(std::string(2, '\0'));
            // write(writer, config.find("JoystickEnabled").value_or(zero));
            // write(writer, config.find("EffectsVolume").value_or(zero));
            // write(writer, config.find("MusicVolume").value_or(zero));
            // write(writer, config.find("MouseSensitivity").value_or(zero));
            // write(writer, config.find("GammaCorrection").value_or(zero));
            // write(writer, config.find("SuperShield").value_or(config_value(false)) ? config_value(std::uint32_t(1)) : zero);
            // write(writer, config.find("NumChannels").value_or(zero));
            // write(writer, config.find("AutoSwapWeapon").value_or(config_value(false)) ? config_value(std::uint32_t(1)) : zero);
            // write(writer, config.find("CutsceneEffectsVolume").value_or(zero));
            // write(writer, config.find("CutsceneMusicVolume").value_or(zero));

            // writer.write(std::uint8_t(0xDB));
            // writer.write(std::string(24, 0xFF));
            // writer.write(std::string(sizeof(std::array<std::int32_t, 3>), '\0'));          
            // write(writer, config.find("JoystickType").value_or(zero));
            // write(writer, config.find("HatInfo").value_or(zero));
            // writer.write(std::uint32_t(0));
            // write(writer, config.find("Joy1CalibrationCentre").value_or(zero));
            // write(writer, config.find("Joy1TopLeft").value_or(zero));
            // write(writer, config.find("Joy1BottomRight").value_or(zero));
            // writer.write(std::string(sizeof(std::array<std::int32_t, 3>), '\0'));
            // write(writer, config.find("Joy2CalibrationCentre").value_or(zero));
            // write(writer, config.find("Joy2TopLeft").value_or(zero));
            // write(writer, config.find("Joy2BottomRight").value_or(zero));
            // writer.write(std::string(32, 0xFF));
            // write(writer, config.find("IsCalibrated").value_or(config_value(false)) ? config_value(std::numeric_limits<std::uint32_t>::max()) : zero);
        
            // for (auto action : keyboardActions)
            // {
            //     if (action == unused)
            //     {
            //         writer.write(std::uint8_t(0x00));
            //     }
            //     else
            //     {
            //         write(writer, config.find(action).value_or(config_value(std::uint8_t(0u))));
            //     }
            // }

            // for (auto action : joystickActions)
            // {
            //     if (action == unused)
            //     {
            //         writer.write(std::uint8_t(0x00));
            //     }
            //     else
            //     {
            //         write(writer, config.find(action).value_or(config_value(std::uint8_t(0u))));
            //     }
            // }

            // for (auto action : mouseActions)
            // {
            //     if (action == unused)
            //     {
            //         writer.write(std::uint8_t(0x00));
            //     }
            //     else
            //     {
            //         write(writer, config.find(action).value_or(config_value(std::uint8_t(0u))));
            //     }
            // }
        }
    }
}