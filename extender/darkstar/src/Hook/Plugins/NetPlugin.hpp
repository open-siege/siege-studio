//
// Created by Matthew on 2020/06/28.
//

#ifndef DARKSTARHOOK_NETPLUGIN_HPP
#define DARKSTARHOOK_NETPLUGIN_HPP

#include "Hook/Plugins/Shared.hpp"

namespace Hook::Plugins
{
    struct NetPlugin : GamePluginWrapper
    {
        void netStats()
        {
            execute(plugin, 0, { "netStats" });
        }

        void playDemo(std::string demoFilename)
        {
            execute(plugin, 1, { "playDemo", demoFilename.c_str() });
        }

        void timeDemo(std::string demoFilename)
        {
            execute(plugin, 2, { "timeDemo", demoFilename.c_str() });
        }

        void connect(std::string netAddress)
        {
            execute(plugin, 3, { "connect", netAddress.c_str() });
        }

        void disconnect()
        {
            execute(plugin, 4, { "disconnect" });
        }

        void kick(int playerId, std::optional<std::string> reason = std::nullopt)
        {
            std::string playerIdStr = std::to_string(playerId);

            if (reason)
            {
                execute(plugin, 5, { "net::kick", playerIdStr.c_str(), reason->c_str() });
                return;
            }

            execute(plugin, 5, { "net::kick", playerIdStr.c_str() });
        }

        void rateChanged()
        {
            execute(plugin, 6, { "pref::PacketRate" });
        }

        void logPacketStats()
        {
            execute(plugin, 7, { "logPacketStats" });
        }

        std::string translateAddress(std::string address, std::optional<int> timeout = std::nullopt)
        {
            if (timeout)
            {
                std::string timeoutStr = std::to_string(*timeout);
                return execute(plugin, 8, { "DNet::TranslateAddress", address.c_str(), timeoutStr.c_str() });
            }

            return execute(plugin, 8, { "DNet::TranslateAddress", address.c_str(), "0" });
        }
    };
}

#endif //DARKSTARHOOK_NETPLUGIN_HPP
