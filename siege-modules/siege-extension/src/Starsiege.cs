using System;
using System.IO;
using System.Collections.Generic;
using System.Collections;
using System.Runtime.InteropServices;

namespace Siege.Extension.Starsiege
{
    public static class Guids
    {
        public const string IDispatch = "00020400-0000-0000-C000-000000000046";
    }

    [ComImport]
    [InterfaceType(ComInterfaceType.InterfaceIsIDispatch)]
    [Guid(Guids.IDispatch)]
    public interface GameConsole
    {
        /// <summary>
        /// This property always returns a value &lt; 1.
        /// </summary>
        float floor(float x);

        /// <summary>
        /// This property always returns a value &lt; 1.
        /// </summary>
        float sqrt(float x);

        /// <summary>
        /// This property always returns a value &lt; 1.
        /// </summary>
        void echo(string s);

        /// <summary>
        /// This property always returns a value &lt; 1.
        /// </summary>
        void dbecho(string s);

        /// <summary>
        /// This property always returns a value &lt; 1.
        /// </summary>
        string strcat(string a, string b);

        /// <summary>
        /// This property always returns a value &lt; 1.
        /// </summary>
        void quit();

        /// <summary>
        /// This property always returns a value &lt; 1.
        /// </summary>
        void export();
        void deleteVariables();

        /// <summary>
        /// This property always returns a value &lt; 1.
        /// </summary>
        void exportFunctions();

        /// <summary>
        /// This property always returns a value &lt; 1.
        /// </summary>
        void deleteFunctions();

        /// <summary>
        /// This property always returns a value &lt; 1.
        /// </summary>
        void exec(string filename);

        /// <summary>
        /// This property always returns a value &lt; 1.
        /// </summary>
        string eval(string code);

        /// <summary>
        /// This property always returns a value &lt; 1.
        /// </summary>
        void debug();

        /// <summary>
        /// This property always returns a value &lt; 1.
        /// </summary>
        void trace();

        bool Console_logBufferEnabled
        {
            get;
        }

        int Console_printLevel
        {
            get;
        }

        int Console_updateMetrics
        {
            get;
        }

        int Console_logMode
        {
            get;
        }
    }

    [ComImport]
    [InterfaceType(ComInterfaceType.InterfaceIsIDispatch)]
    [Guid(Guids.IDispatch)]
    public interface DynamicDataPlugin
    {
        void dynDataWriteObject();
        void dynDataReadObject();
        void dynDataWriteClassType();
        void dynDataReadClassType();
        void scanXLoad();
        void scanXWrite();
        void scanXFlush();
        void scanXEcho();
        void EncyclopediaLoad();
        void EncyclopediaWrite();
        void EncyclopediaFlush();
        void EncyclopediaEcho();
        void MissionBriefLoad();
        void MissionBriefFlush();
        void MissionBriefEcho();
        void CampaignLoad();
        void CampaignEcho();
        void GameLoad();
        void GameSave();
        void GameSetSquadMate();
        void GameSetVehicle();
        void FlushPilots();
    }

    [ComImport]
    [InterfaceType(ComInterfaceType.InterfaceIsIDispatch)]
    [Guid(Guids.IDispatch)]
    public interface ICPlugin
    {

    }

    [ComImport]
    [InterfaceType(ComInterfaceType.InterfaceIsIDispatch)]
    [Guid(Guids.IDispatch)]
    public interface DatabasePlugin
    {

    }

    [ComImport]
    [InterfaceType(ComInterfaceType.InterfaceIsIDispatch)]
    [Guid(Guids.IDispatch)]
    public interface HercInfoDataPlugin
    {

    }

    [ComImport]
    [InterfaceType(ComInterfaceType.InterfaceIsIDispatch)]
    [Guid(Guids.IDispatch)]
    public interface ESBasePlugin
    {

    }

    [ComImport]
    [InterfaceType(ComInterfaceType.InterfaceIsIDispatch)]
    [Guid(Guids.IDispatch)]
    public interface ESGameplayPlugin
    {

    }

    [ComImport]
    [InterfaceType(ComInterfaceType.InterfaceIsIDispatch)]
    [Guid(Guids.IDispatch)]
    public interface ESChatPlugin
    {

    }

    [ComImport]
    [InterfaceType(ComInterfaceType.InterfaceIsIDispatch)]
    [Guid(Guids.IDispatch)]
    public interface MissionPlugin
    {

    }

    [ComImport]
    [InterfaceType(ComInterfaceType.InterfaceIsIDispatch)]
    [Guid(Guids.IDispatch)]
    public interface SkyPlugin
    {

    }

    [ComImport]
    [InterfaceType(ComInterfaceType.InterfaceIsIDispatch)]
    [Guid(Guids.IDispatch)]
    public interface ESFPlugin
    {

    }

    [ComImport]
    [InterfaceType(ComInterfaceType.InterfaceIsIDispatch)]
    [Guid(Guids.IDispatch)]
    public interface IRCPlugin
    {

    }

    [ComImport]
    [InterfaceType(ComInterfaceType.InterfaceIsIDispatch)]
    [Guid(Guids.IDispatch)]
    public interface TedPlugin
    {

    }

    [ComImport]
    [InterfaceType(ComInterfaceType.InterfaceIsIDispatch)]
    [Guid(Guids.IDispatch)]
    public interface LSPLugin
    {

    }

    [ComImport]
    [InterfaceType(ComInterfaceType.InterfaceIsIDispatch)]
    [Guid(Guids.IDispatch)]
    public interface SimGuiPlugin
    {

    }

    [ComImport]
    [InterfaceType(ComInterfaceType.InterfaceIsIDispatch)]
    [Guid(Guids.IDispatch)]
    public interface TerrainPlugin
    {

    }

    [ComImport]
    [InterfaceType(ComInterfaceType.InterfaceIsIDispatch)]
    [Guid(Guids.IDispatch)]
    public interface ToolPlugin
    {

    }

    [ComImport]
    [InterfaceType(ComInterfaceType.InterfaceIsIDispatch)]
    [Guid(Guids.IDispatch)]
    public interface SoundFXPlugin
    {

    }

    [ComImport]
    [InterfaceType(ComInterfaceType.InterfaceIsIDispatch)]
    [Guid(Guids.IDispatch)]
    public interface SimTreePlugin
    {

    }

    [ComImport]
    [InterfaceType(ComInterfaceType.InterfaceIsIDispatch)]
    [Guid(Guids.IDispatch)]
    public interface SimInputPlugin
    {
        void newInputManager();
        void listInputDevices();
        void getInputDeviceInfo();
        void saveInputDeviceInfo();
        void inputOpen();
        void inputClose();
        void inputCapture();
        void inputRelease();
        void inputActivate();
        void inputDeactivate();
        void newActionMap();
        void bindAction();
        void bindCommand();
        void bind();
        void saveActionMap();
        void unbind();
        void defineKey();
    }

    [ComImport]
    [InterfaceType(ComInterfaceType.InterfaceIsIDispatch)]
    [Guid(Guids.IDispatch)]
    public interface RedbookPlugin
    {
        void newRedbook();
        void rbOpen();
        void rbClose();
        void rbEject();
        void rbRetract();
        void rbGetStatus();
        void rbGetTrackCount();
        void rbGetTrackInfo();
        void rbGetTrackPosition();
        void rbPlay();
        void rbStop();
        void rbPause();
        void rbResume();
        void rbSetVolume();
        void rbGetVolume();
        void rbSetPlayMode();
    }

    [ComImport]
    [InterfaceType(ComInterfaceType.InterfaceIsIDispatch)]
    [Guid(Guids.IDispatch)]
    public interface NetPlugin
    {
        void netStats();
        void logPacketStats();
        void DNet_TranslateAddress();
        void playDemo();
        void connect();
        void disconnect();
        void net_kick();
    }

    [ComImport]
    [InterfaceType(ComInterfaceType.InterfaceIsIDispatch)]
    [Guid(Guids.IDispatch)]
    public interface MovPlayPlugin
    {
        void newMovPlay();
        void openMovie();
        void closeMovie();
        void playMovie();
        void playMovieToComp();
        void stopMovie();
        void pauseMovie();
    }

    [ComImport]
    [InterfaceType(ComInterfaceType.InterfaceIsIDispatch)]
    [Guid(Guids.IDispatch)]
    public interface GFXPlugin
    {

    }

    [ComImport]
    [InterfaceType(ComInterfaceType.InterfaceIsIDispatch)]
    [Guid(Guids.IDispatch)]
    public interface MissionEditorPlugin
    {

    }
}