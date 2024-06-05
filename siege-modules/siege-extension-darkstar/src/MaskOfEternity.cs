using System;
using System.IO;
using System.Collections.Generic;
using System.Collections;
using System.Runtime.InteropServices;

namespace Siege.Extension.MaskOfEternity
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
        void alias();
        void activate();
        void cls();
        void debug();
        void bind();
        void echo();
        void eval();
        void exec();
        void @false();
        void history();
        void inc();
        void not();
        void set();
        void setcat();
        void test();
        void quit();
    }

    [ComImport]
    [InterfaceType(ComInterfaceType.InterfaceIsIDispatch)]
    [Guid(Guids.IDispatch)]
    public interface GamePlugin
    {
        void deleteObject();
        void listObjects();
        void loadPalette();
        void loadVolume();
        void newGroup();
        void addGroup();
        void purgeRsources();
        void purgeResource();
        void setTimedPurge();
        void preloadResource();
        void lockResource();
        void lockShape();
        void lockInterior();
        void preloadShape();
        void lightTerrain();
        void enableEdit();
        void loadKQ();
        void saveKQ();
        void loadObject();
        void loadAnim();
        void setDeleteWhenDone();
        void sendEvent();
        void setLoadProgress();
        void logging();
        void move();
        void handsOff();
        void listActive();
        void exist();
        void setWindowDevice();
        void winInfo();
        void getHaze();
        void setHaze();
        void checkKQ();
        void setShadowMask();
        void setBrightness();
        void set800Mode();
        void setActiveCamera();
        void setThirdPerson();
        void setFirstPerson();
        void daedHallTeleport();
        void seActivePoint();
        void pointAndCue();
        void useDynamicLighting();
        void pauseAnim();
        void setAnimPos();
        void deactivateLight();
        void resetHazeColor();
        void activatePersistMgr();
        void beep();
        void setCurrentTime();
        void getCurrentTime();
        void matchHeights();
        void terrAnimSwitch();
        void checkDistance();
        void assignGModeName();
        void setFullIntensity();
        void rocket();
        void testCast();
        void showResources();
        void makeActive();
        void getEndLoop();
        void addMapIcon();
        void removeMapIcon();
        void earnTeleport();
        void teleportersEarned();
        void onTeleport();
        void offTeleport();
        void doCredits();
        void makeTransluscent();
        void sendAction();
        void mouseAction();
        void guiKeyAction();
        void guiSelectAction();
        void record();
        void setLowerGui();
        void setUpperGui();
        void claudia();
        void setMapGui();
        void getGuiStatus();
        void getConInventory();
        void conClickItem();
        void setConnorFlag();
        void getConnorFlag();
        void oscar();
        void adam();
        void playAvi();
        void setCombatLevel();
        void applyCombatLevel();
        void doPurging();
        void leslie();
        void resetScreenSize();
        void goldCoinPersists();
        void checkMips();
        void ruth();
        void allowScreenDumps();
        void mem();
        void doPopup();
        void notStartingHelp();
        void checkSave();
        void welch_piel();
        void displayIcon();
        void undisplayIcon();
        void blackOut();
        void useAlerantiveLightmap();
        void jeffo();
        void allowShadows();
        void invClick();
        void killGame();
    }

    [ComImport]
    [InterfaceType(ComInterfaceType.InterfaceIsIDispatch)]
    [Guid(Guids.IDispatch)]
    public interface KQObjectPlugin
    {
        
    }

    [ComImport]
    [InterfaceType(ComInterfaceType.InterfaceIsIDispatch)]
    [Guid(Guids.IDispatch)]
    public interface KQCameraPlugin
    {

    }

    [ComImport]
    [InterfaceType(ComInterfaceType.InterfaceIsIDispatch)]
    [Guid(Guids.IDispatch)]
    public interface KQMonsterPlugin
    {

    }

    [ComImport]
    [InterfaceType(ComInterfaceType.InterfaceIsIDispatch)]
    [Guid(Guids.IDispatch)]
    public interface KQEmitterPlugin
    {

    }

    [ComImport]
    [InterfaceType(ComInterfaceType.InterfaceIsIDispatch)]
    [Guid(Guids.IDispatch)]
    public interface KQHeapPlugin
    {

    }

    [ComImport]
    [InterfaceType(ComInterfaceType.InterfaceIsIDispatch)]
    [Guid(Guids.IDispatch)]
    public interface KQMusicPlugin
    {
        void KQMusic_play();
        void KQMusic_push();
        void KQMusic_pop();
        void KQMusic_create();
        void KQMusic_delete();
    }

    [ComImport]
    [InterfaceType(ComInterfaceType.InterfaceIsIDispatch)]
    [Guid(Guids.IDispatch)]
    public interface KQSoundPlugin
    {
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
    public interface LightPlugin
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
    public interface InteriorPlugin
    {

    }
}