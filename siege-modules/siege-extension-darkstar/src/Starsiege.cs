using System;
using System.IO;
using System.Collections.Generic;
using System.Collections;
using System.Runtime.InteropServices;

namespace Siege.Extension.Starsiege
{
    [ComImport]
    [InterfaceType(ComInterfaceType.InterfaceIsIDispatch)]
    [Guid(Guids.IDispatch)]
    public interface GameConsole
    {
        /// <summary>
        /// Returns only the integer portion of a number.
        /// </summary>
        float floor(float x);

        /// <summary>
        /// Returns the square root of a number.
        /// </summary>
        float sqrt(float x);

        /// <summary>
        /// Writes a message to the console
        /// </summary>
        void echo(string s);

        /// <summary>
        /// 
        /// </summary>
        void dbecho(string s);

        /// <summary>
        /// Joins two strings into one.
        /// </summary>
        string strcat(string a, string b);

        /// <summary>
        /// Quits the game.
        /// </summary>
        void quit();

        /// <summary>
        /// 
        /// </summary>
        void export();

        /// <summary>
        /// Deletes variables based on a filter string
        /// </summary>
        void deleteVariables();

        /// <summary>
        /// Exports registered functions to a file.
        /// </summary>
        void exportFunctions();

        /// <summary>
        /// Deletes functions based on a filter string.
        /// </summary>
        void deleteFunctions();

        /// <summary>
        /// Executes a script file.
        /// </summary>
        void exec(string filename);

        /// <summary>
        /// Executes a script string.
        /// </summary>
        string eval(string code);

        /// <summary>
        /// Triggers a manual breakpoint if a debugger is attached.
        /// </summary>
        void debug();

        /// <summary>
        /// 
        /// </summary>
        void trace();

        [RealExportName("Console::logBufferEnabled")]
        bool Console_logBufferEnabled
        {
            get;
        }

        [RealExportName("Console::printLevel")]
        int Console_printLevel
        {
            get;
        }

        [RealExportName("Console::updateMetrics")]
        int Console_updateMetrics
        {
            get;
        }

        [RealExportName("Console::logMode")]
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
        /// <summary>
        /// 
        /// </summary>
        void dynDataWriteObject();

        /// <summary>
        /// 
        /// </summary>
        void dynDataReadObject();

        /// <summary>
        /// 
        /// </summary>
        void dynDataWriteClassType();

        /// <summary>
        /// 
        /// </summary>
        void dynDataReadClassType();

        /// <summary>
        /// 
        /// </summary>
        void scanXLoad();

        /// <summary>
        /// 
        /// </summary>
        void scanXWrite();

        /// <summary>
        /// 
        /// </summary>
        void scanXFlush();

        /// <summary>
        /// 
        /// </summary>
        void scanXEcho();

        /// <summary>
        /// 
        /// </summary>
        void EncyclopediaLoad();

        /// <summary>
        /// 
        /// </summary>
        void EncyclopediaWrite();

        /// <summary>
        /// 
        /// </summary>
        void EncyclopediaFlush();

        /// <summary>
        /// 
        /// </summary>
        void EncyclopediaEcho();

        /// <summary>
        /// 
        /// </summary>
        void MissionBriefLoad();

        /// <summary>
        /// 
        /// </summary>
        void MissionBriefFlush();

        /// <summary>
        /// 
        /// </summary>
        void MissionBriefEcho();

        /// <summary>
        /// 
        /// </summary>
        void CampaignLoad();

        /// <summary>
        /// 
        /// </summary>
        void CampaignEcho();

        /// <summary>
        /// 
        /// </summary>
        void GameLoad();

        /// <summary>
        /// 
        /// </summary>
        void GameSave();

        /// <summary>
        /// 
        /// </summary>
        void GameSetSquadMate();

        /// <summary>
        /// 
        /// </summary>
        void GameSetVehicle();

        /// <summary>
        /// 
        /// </summary>
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

        [RealExportName("DNet::TranslateAddress")]
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