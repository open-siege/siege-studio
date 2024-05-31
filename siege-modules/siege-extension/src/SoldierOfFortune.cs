using System;
using System.IO;
using System.Collections.Generic;
using System.Collections;
using System.Runtime.InteropServices;

namespace Siege.Extension.SoldierOfFortune
{
    public static class Guids
    {
        public const string IDispatch = "00020400-0000-0000-C000-000000000046";
    }

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
        void on_moveup();
        void off_moveup();
        void on_movedown();
        void off_movedown();

        void on_left();
        void off_left();

        void on_right();
        void off_right();

        void on_forward();
        void off_forward();

        void on_back();
        void off_back();

        void on_lookup();
        void off_lookup();

        void on_lookdown();
        void off_lookdown();

        void on_strafe();
        void off_strafe();

        void on_moveleft();
        void off_moveleft();

        void on_leanleft();
        void off_leanleft();

        void on_leanright();
        void off_leanright();

        void on_speed();
        void off_speed();

        void on_attack();
        void off_attack();

        void on_altattack();
        void off_altattack();

        void on_weaponExtra1();
        void off_weaponExtra1();

        void on_weaponExtra2();
        void off_weaponExtra2();

        void on_use();
        void off_use();

        void on_klook();
        void off_klook();
    }

    [ComImport]
    [InterfaceType(ComInterfaceType.InterfaceIsIDispatch)]
    [Guid(Guids.IDispatch)]
    public interface MouseAndJoystickPlugin
    {
        void on_mlook();
        void off_mlook();
        void joy_advancedupdate();
    }
}