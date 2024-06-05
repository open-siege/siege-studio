using System;
using System.IO;
using System.Collections.Generic;
using System.Collections;
using System.Runtime.InteropServices;

namespace Siege.Extension.SoldierOfFortune
{
    [ComImport]
    [InterfaceType(ComInterfaceType.InterfaceIsIDispatch)]
    [Guid(Guids.IDispatch)]
    public interface ServerPlugin
    {
        void adddownload();
        void heartbeat();
        void kick();
        void status();
        void serverinfo();
        void dumpuser();
        void map();
        void newsave();
        void savesleft();
        void gserver();
        void demomap();
        void gamemap();
    }

    [ComImport]
    [InterfaceType(ComInterfaceType.InterfaceIsIDispatch)]
    [Guid(Guids.IDispatch)]
    public interface ConsolePlugin
    {
        void toggleconsole();
        void togglechat();
        void messagemode();
        void messagemode2();
        void clear();
        void condump();
        void conchars();
    }

    [ComImport]
    [InterfaceType(ComInterfaceType.InterfaceIsIDispatch)]
    [Guid(Guids.IDispatch)]
    public interface GameActionPlugin
    {
        void centerview();

        [RealExportName("+moveup")]
        void on_moveup();
        [RealExportName("-moveup")]
        void off_moveup();

        [RealExportName("+movedown")]
        void on_movedown();
        [RealExportName("-movedown")]
        void off_movedown();

        [RealExportName("+left")]
        void on_left();
        [RealExportName("-left")]
        void off_left();

        [RealExportName("+right")]
        void on_right();
        [RealExportName("-right")]
        void off_right();

        [RealExportName("+forward")]
        void on_forward();
        [RealExportName("-forward")]
        void off_forward();

        [RealExportName("+back")]
        void on_back();
        [RealExportName("-back")]
        void off_back();

        [RealExportName("+lookup")]
        void on_lookup();
        [RealExportName("-lookup")]
        void off_lookup();

        [RealExportName("+lookdown")]
        void on_lookdown();
        [RealExportName("-lookdown")]
        void off_lookdown();

        [RealExportName("+strafe")]
        void on_strafe();
        [RealExportName("-strafe")]
        void off_strafe();

        [RealExportName("+moveleft")]
        void on_moveleft();
        [RealExportName("-moveleft")]
        void off_moveleft();

        [RealExportName("+leanleft")]
        void on_leanleft();

        [RealExportName("+leanleft")]
        void off_leanleft();

        [RealExportName("+leanright")]
        void on_leanright();
        [RealExportName("-leanright")]
        void off_leanright();

        [RealExportName("+speed")]
        void on_speed();
        [RealExportName("-speed")]
        void off_speed();

        [RealExportName("+attack")]
        void on_attack();
        [RealExportName("-attack")]
        void off_attack();

        [RealExportName("+attack")]
        void on_altattack();
        [RealExportName("-attack")]
        void off_altattack();

        [RealExportName("+weaponExtra1")]
        void on_weaponExtra1();
        [RealExportName("-weaponExtra1")]
        void off_weaponExtra1();

        [RealExportName("+weaponExtra2")]
        void on_weaponExtra2();
        [RealExportName("-weaponExtra2")]
        void off_weaponExtra2();

        [RealExportName("+use")]
        void on_use();
        [RealExportName("-use")]
        void off_use();

        [RealExportName("+klook")]
        void on_klook();
        [RealExportName("-klook")]
        void off_klook();
    }

    [ComImport]
    [InterfaceType(ComInterfaceType.InterfaceIsIDispatch)]
    [Guid(Guids.IDispatch)]
    public interface MouseAndJoystickPlugin
    {
        [RealExportName("+mlook")]
        void on_mlook();
        [RealExportName("-mlook")]
        void off_mlook();

        void joy_advancedupdate();
    }
}