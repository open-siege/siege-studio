### Format Background

Despite each version of the engine having some of the same extensions for some of their formats, each iteration of the engine has completely different structures and binary layouts for said formats.

In other words, 3Space 2.0 DTS files are fundamentally different to 3Space 3.0 DTS files, which are in turn completely different to Torque DTS files.

They do share a similar high level structure and some features, but the overall format changes over time and even between games there is a big difference between the format used.

For example, while Earthsiege and Red Baron 2 might share a 3Space 2.0 core (of sorts), the DTS files themselves have different version tags for each entity and need different code to handle them. Depending on how different they are, they may need completely separate implementations for parsing and viewing.

### Game Support

Because the 3Space engine has a long history, and has morphed into engines with new names, here is a list of the games, most of which were made by Dynamix, (focusing specifically on DOS or Windows) which are intended to be supported or are supported:

# 3Space 1.0

## Planned file support:

* [PAL](/siege-modules/content/siege-content-3space/src/pal/PAL.md)
* [SCR](/siege-modules/content/siege-content-3space/src/bmp/SCR.md)
* [BMP](/siege-modules/content/siege-content-3space/src/bmp/BMP.md)
* [FNT](/siege-modules/content/siege-content-3space/src/fn/FNT.md)
* [TBL](/siege-modules/content/siege-content-3space/src/dts/TBL.md)

## Games

### Arcticfox - 1987 (for DOS)

No support. Unknown file formats.

### A-10 Tank Killer - 1989 (for DOS)

No support.

### Abrams Battle Tank - 1989 (for DOS)

No support.

### David Wolf Secret Agent - 1989 (for DOS)

No support.

### DeathTrack - 1989

No support.

### Die Hard - 1989

No support.

### MechWarrior - 1989 (for DOS)

No support.

### F-14 Tomcat - 1990 (for DOS)

No support.

# 3Space 1.5

## Planned file support:

* [RMF](/siege-modules/content/siege-content-3space/src/RMF.md)
* [DYN](/siege-modules/content/siege-content-3space/src/DYN.md)
* [TBL](/siege-modules/content/siege-content-3space/src/dts/TBL.md)
* [PAL](/siege-modules/content/siege-content-3space/src/pal/PAL.md)
* [SCR](/siege-modules/content/siege-content-3space/src/bmp/SCR.md)
* [TTM](/siege-modules/content/siege-content-3space/src/bmp/TTM.md)

## Games

### Red Baron - 1990

* [RMF](/siege-modules/content/siege-content-3space/src/RMF.md)

### Stellar 7 (re-release) - 1990

No support.

### A-10 Tank Killer 1.5 - 1991

* [RMF](/siege-modules/content/siege-content-3space/src/RMF.md)

### Nova 9: The Return of Gir Draxon - 1991

* [RMF](/siege-modules/content/siege-content-3space/src/RMF.md)

### Aces of the Pacific - 1992

* [DYN](/siege-modules/content/siege-content-3space/src/DYN.md)

### Aces Over Europe - 1993

* [DYN](/siege-modules/content/siege-content-3space/src/DYN.md)

### Betrayal at Krondor - 1993

* [RMF](/siege-modules/content/siege-content-3space/src/RMF.md)

# 3Space 2.0

## Planned file support:

* [VOL](/siege-modules/content/siege-content-3space/src/VOL.md)
* [DYN](/siege-modules/content/siege-content-3space/src/DYN.md)
* [PAL](/siege-modules/content/siege-content-3space/src/pal/PAL.md)
* [DPL](/siege-modules/content/siege-content-3space/src/pal/DPL.md)
* [BMP](/siege-modules/content/siege-content-3space/src/bmp/BMP.md)
* [SCR](/siege-modules/content/siege-content-3space/src/bmp/SCR.md)
* [DBM](/siege-modules/content/siege-content-3space/src/bmp/DBM.md)
* [DBA](/siege-modules/content/siege-content-3space/src/bmp/DBA.md)
* [DTS](/siege-modules/content/siege-content-3space/src/dts/DTS.md)
* CAR
* MEC

## Games

### Aces of the Deep - 1994

* [VOL](/siege-modules/content/siege-content-3space/src/VOL.md)
* [DYN](/siege-modules/content/siege-content-3space/src/DYN.md)
* [DTS](/siege-modules/content/siege-content-3space/src/dts/DTS.md) support in progress (feature/earthsiege-files)

### Metaltech: Battledrome - 1994

* [VOL](/siege-modules/content/siege-content-3space/src/VOL.md)
* [DTS](/siege-modules/content/siege-content-3space/src/dts/DTS.md) support in progress (feature/earthsiege-files)

### Metaltech: Earthsiege - 1994

* [VOL](/siege-modules/content/siege-content-3space/src/VOL.md)
* [DTS](/siege-modules/content/siege-content-3space/src/dts/DTS.md) support in progress (feature/earthsiege-files)

### Command: Aces of the Deep - 1995

* [VOL](/siege-modules/content/siege-content-3space/src/VOL.md)
* [DYN](/siege-modules/content/siege-content-3space/src/DYN.md)
* [DTS](/siege-modules/content/siege-content-3space/src/dts/DTS.md) support in progress (feature/earthsiege-files)

### Earthsiege 2 - 1996

* [VOL](/siege-modules/content/siege-content-3space/src/VOL.md)
* [DTS](/siege-modules/content/siege-content-3space/src/dts/DTS.md) support in progress (feature/earthsiege-files)

# 3Space 2.5

## Planned file support:

* [VOL](/siege-modules/content/siege-content-3space/src/VOL.md)
* [DTS](/siege-modules/content/siege-content-3space/src/dts/DTS.md)
* [DT2](/siege-modules/content/siege-content-3space/src/dts/DT2.md)
* [DML](/siege-modules/content/siege-content-3space/src/dts/DML.md)
* [BMP](/siege-modules/content/siege-content-3space/src/bmp/BMP.md)
* [PBM](/siege-modules/content/siege-content-3space/src/bmp/PBM.md)
* [PBA](/siege-modules/content/siege-content-3space/src/bmp/PBA.md)
* [PAL](/siege-modules/content/siege-content-3space/src/pal/PAL.md)
* [IPL](/siege-modules/content/siege-content-3space/src/pal/IPL.md)
* [PPL](/siege-modules/content/siege-content-3space/src/pal/PPL.md)

## Games

### Silent Thunder: A-10 Tank Killer 2 - 1996

* [VOL](/siege-modules/content/siege-content-3space/src/VOL.md)
* [DT2](/siege-modules/content/siege-content-3space/src/dts/DT2.md) support in progress (feature/earthsiege-files)
* [DML](/siege-modules/content/siege-content-3space/src/dts/DML.md) support in progress (feature/earthsiege-files)
* [PAL](/siege-modules/content/siege-content-3space/src/pal/PAL.md)
* [PPL](/siege-modules/content/siege-content-3space/src/pal/PPL.md)

### Red Baron 2 - 1997

* [VOL](/siege-modules/content/siege-content-3space/src/VOL.md) (partial support. Needs more investigation to support compression)
* [DTS](/siege-modules/content/siege-content-3space/src/dts/DTS.md) support in progress (feature/earthsiege-files)
* [DML](/siege-modules/content/siege-content-3space/src/dts/DML.md) support in progress (feature/earthsiege-files)

### Pro Pilot '98 - 1997

* [VOL](/siege-modules/content/siege-content-3space/src/VOL.md) (partial support. Needs more investigation to support compression)
* [DTS](/siege-modules/content/siege-content-3space/src/dts/DTS.md) support in progress (feature/earthsiege-files)
* [DML](/siege-modules/content/siege-content-3space/src/dts/DML.md) support in progress (feature/earthsiege-files)

### Red Baron 3D - 1998

* [VOL](/siege-modules/content/siege-content-3space/src/VOL.md) (partial support. Needs more investigation to support compression)
* [DTS](/siege-modules/content/siege-content-3space/src/dts/DTS.md) support in progress (feature/earthsiege-files)
* [DML](/siege-modules/content/siege-content-3space/src/dts/DML.md) support in progress (feature/earthsiege-files)

### Pro Pilot '99 - 1998

* [VOL](/siege-modules/content/siege-content-3space/src/VOL.md) (partial support. Needs more investigation to support compression)
* [DTS](/siege-modules/content/siege-content-3space/src/dts/DTS.md) support in progress (feature/earthsiege-files)
* [DML](/siege-modules/content/siege-content-3space/src/dts/DML.md) support in progress (feature/earthsiege-files)

### Kid Pilot - 1998

* [VOL](/siege-modules/content/siege-content-3space/src/VOL.md) (partial support. Needs more investigation to support compression)
* [DTS](/siege-modules/content/siege-content-3space/src/dts/DTS.md) support in progress (feature/earthsiege-files)
* [DML](/siege-modules/content/siege-content-3space/src/dts/DML.md) support in progress (feature/earthsiege-files)

### Curse You! Red Baron - 1999

* [VOL](/siege-modules/content/siege-content-3space/src/VOL.md) (partial support. Needs more investigation to support compression)
* [DTS](/siege-modules/content/siege-content-3space/src/dts/DTS.md) support in progress (feature/earthsiege-files)
* [DML](/siege-modules/content/siege-content-3space/src/dts/DML.md) support in progress (feature/earthsiege-files)

# 3Space 3.0

## Planned file support:

* [VOL](/siege-modules/content/siege-content-3space/src/VOL.md)
* [DTS](/siege-modules/content/siege-content-3space/src/dts/DTS.md)
* [DML](/siege-modules/content/siege-content-3space/src/dts/DML.md)
* [KQS](/siege-modules/content/siege-content-3space/src/dts/DTS.md)
* [BMP](/siege-modules/content/siege-content-3space/src/bmp/BMP.md)
* [PBM](/siege-modules/content/siege-content-3space/src/bmp/PBM.md)
* [PBA](/siege-modules/content/siege-content-3space/src/bmp/PBA.md)
* [PFT](/siege-modules/content/siege-content-3space/src/font/PFT.md)
* [PAL](/siege-modules/content/siege-content-3space/src/pal/PAL.md)
* [IPL](/siege-modules/content/siege-content-3space/src/pal/IPL.md)
* [PPL](/siege-modules/content/siege-content-3space/src/pal/PPL.md)
* [CS](/siege-modules/content/siege-content-3space/src/cscript/CS.md)

## Games

### Front Page Sports: Ski Racing - 1997

* [VOL](/siege-modules/content/siege-content-3space/src/VOL.md)
* [TBV](/siege-modules/content/siege-content-3space/src/TBV.md)
* [DML](/siege-modules/content/siege-content-3space/src/dts/DML.md)
* [PAL](/siege-modules/content/siege-content-3space/src/pal/PAL.md)
* [PPL](/siege-modules/content/siege-content-3space/src/pal/PPL.md)
* [BMP](/siege-modules/content/siege-content-3space/src/bmp/BMP.md)
* [PBA](/siege-modules/content/siege-content-3space/src/bmp/PBA.md)

### King's Quest: Mask of Eternity - 1998

* [VOL](/siege-modules/content/siege-content-3space/src/VOL.md)
* [DML](/siege-modules/content/siege-content-3space/src/dts/DML.md)
* [PAL](/siege-modules/content/siege-content-3space/src/pal/PAL.md)
* [PPL](/siege-modules/content/siege-content-3space/src/pal/PPL.md)
* [BMP](/siege-modules/content/siege-content-3space/src/bmp/BMP.md)
* [PBA](/siege-modules/content/siege-content-3space/src/bmp/PBA.md)

### Driver's Education '98 - 1998

* [VOL](/siege-modules/content/siege-content-3space/src/VOL.md)
* [DTS](/siege-modules/content/siege-content-3space/src/dts/DTS.md)
* [DML](/siege-modules/content/siege-content-3space/src/dts/DML.md)
* [PAL](/siege-modules/content/siege-content-3space/src/pal/PAL.md)
* [PPL](/siege-modules/content/siege-content-3space/src/pal/PPL.md)
* [BMP](/siege-modules/content/siege-content-3space/src/bmp/BMP.md)
* [PBA](/siege-modules/content/siege-content-3space/src/bmp/PBA.md)

### Driver's Education '99 - 1999

* [VOL](/siege-modules/content/siege-content-3space/src/VOL.md)
* [DTS](/siege-modules/content/siege-content-3space/src/dts/DTS.md)
* [DML](/siege-modules/content/siege-content-3space/src/dts/DML.md)
* [PAL](/siege-modules/content/siege-content-3space/src/pal/PAL.md)
* [PPL](/siege-modules/content/siege-content-3space/src/pal/PPL.md)
* [BMP](/siege-modules/content/siege-content-3space/src/bmp/BMP.md)
* [PBA](/siege-modules/content/siege-content-3space/src/bmp/PBA.md)

### Field & Stream: Trophy Bass 3D - 1999

* [VOL](/siege-modules/content/siege-content-3space/src/VOL.md)
* [DTS](/siege-modules/content/siege-content-3space/src/dts/DTS.md)
* [DML](/siege-modules/content/siege-content-3space/src/dts/DML.md)
* [PAL](/siege-modules/content/siege-content-3space/src/pal/PAL.md)
* [PPL](/siege-modules/content/siege-content-3space/src/pal/PPL.md)
* [BMP](/siege-modules/content/siege-content-3space/src/bmp/BMP.md)
* [PBA](/siege-modules/content/siege-content-3space/src/bmp/PBA.md)

### Field & Stream: Trophy Bass 4 - 2000

* [VOL](/siege-modules/content/siege-content-3space/src/VOL.md)
* [DTS](/siege-modules/content/siege-content-3space/src/dts/DTS.md)
* [DML](/siege-modules/content/siege-content-3space/src/dts/DML.md)
* [PAL](/siege-modules/content/siege-content-3space/src/pal/PAL.md)
* [PPL](/siege-modules/content/siege-content-3space/src/pal/PPL.md)
* [BMP](/siege-modules/content/siege-content-3space/src/bmp/BMP.md)
* [PBA](/siege-modules/content/siege-content-3space/src/bmp/PBA.md)

# 3Space 3.5

## Planned file support:

* [VOL](/siege-modules/content/siege-content-3space/src/VOL.md)
* [DTS](/siege-modules/content/siege-content-3space/src/dts/DTS.md)
* [DML](/siege-modules/content/siege-content-3space/src/dts/DML.md)
* [BMP](/siege-modules/content/siege-content-3space/src/bmp/BMP.md)
* [PBM](/siege-modules/content/siege-content-3space/src/bmp/PBM.md)
* [PBA](/siege-modules/content/siege-content-3space/src/bmp/PBA.md)
* [PFT](/siege-modules/content/siege-content-3space/src/font/PFT.md)
* [PAL](/siege-modules/content/siege-content-3space/src/pal/PAL.md)
* [IPL](/siege-modules/content/siege-content-3space/src/pal/IPL.md)
* [PPL](/siege-modules/content/siege-content-3space/src/pal/PPL.md)
* [CS](/siege-modules/content/siege-content-3space/src/cscript/CS.md)

## Games

### Starsiege - 1999

* [VOL](/siege-modules/content/siege-content-3space/src/VOL.md)
* [DTS](/siege-modules/content/siege-content-3space/src/dts/DTS.md)
* [DML](/siege-modules/content/siege-content-3space/src/dts/DML.md)
* [PAL](/siege-modules/content/siege-content-3space/src/pal/PAL.md)
* [PPL](/siege-modules/content/siege-content-3space/src/pal/PPL.md)
* [BMP](/siege-modules/content/siege-content-3space/src/bmp/BMP.md)
* [PBA](/siege-modules/content/siege-content-3space/src/bmp/PBA.md)
* [CS](/siege-modules/content/siege-content-3space/src/cscript/CS.md) (partial grammar implemented)

### Starsiege: Tribes - 1999

* [VOL](/siege-modules/content/siege-content-3space/src/VOL.md)
* [DTS](/siege-modules/content/siege-content-3space/src/dts/DTS.md)
* [DML](/siege-modules/content/siege-content-3space/src/dts/DML.md)
* [PAL](/siege-modules/content/siege-content-3space/src/pal/PAL.md)
* [PPL](/siege-modules/content/siege-content-3space/src/pal/PPL.md)
* [BMP](/siege-modules/content/siege-content-3space/src/bmp/BMP.md)
* [PBA](/siege-modules/content/siege-content-3space/src/bmp/PBA.md)
* [CS](/siege-modules/content/siege-content-3space/src/cscript/CS.md) (partial grammar implemented)

### Desert Fighters - 1999

No support.

# Torque

## Planned file support:

* VL2
* [VOL](/siege-modules/content/siege-content-3space/src/VOL.md)
* [DTS](/siege-modules/content/siege-content-3space/src/dts/DTS.md)
* [DSQ](/siege-modules/content/siege-content-3space/src/dts/DSQ.md)
* [CS](/siege-modules/content/siege-content-3space/src/cscript/CS.md)
* [DSO](/siege-modules/content/siege-content-3space/src/DSO.md)

## Games

### Field & Stream: Trophy Hunting 4 - 2000

No support.

### Field & Stream: Trophy Hunting 5 - 2001

No support.

### Tribes 2 - 2001

No support.

# Dynamix Game Development System

## Planned file support:

* [RMF](/siege-modules/content/siege-content-3space/src/RMF.md)
* [TBL](/siege-modules/content/siege-content-3space/src/dts/TBL.md)
* [PAL](/siege-modules/content/siege-content-3space/src/pal/PAL.md)
* [SCR](/siege-modules/content/siege-content-3space/src/bmp/SCR.md)
* [BMP](/siege-modules/content/siege-content-3space/src/bmp/BMP.md)
* [TTM](/siege-modules/content/siege-content-3space/src/bmp/TTM.md)

## Games

### Rise of the Dragon - 1990

* [RMF](/siege-modules/content/siege-content-3space/src/RMF.md)

### Heart of China - 1991

* [RMF](/siege-modules/content/siege-content-3space/src/RMF.md)

### The Adventures of Willy Beamish - 1991

* [RMF](/siege-modules/content/siege-content-3space/src/RMF.md)

### Quarky & Quaysoo's Turbo Science - 1992

* [RMF](/siege-modules/content/siege-content-3space/src/RMF.md)

# Dynamix 2D/2.5D Game Engines

## Planned file support:

* [RMF](/siege-modules/content/siege-content-3space/src/RMF.md)
* RBX
* [TBV](/siege-modules/content/siege-content-3space/src/TBV.md)
* [VOL](/siege-modules/content/siege-content-3space/src/VOL.md)
* [TBL](/siege-modules/content/siege-content-3space/src/dts/TBL.md)
* [PAL](/siege-modules/content/siege-content-3space/src/pal/PAL.md)
* [SCR](/siege-modules/content/siege-content-3space/src/bmp/SCR.md)
* [BMP](/siege-modules/content/siege-content-3space/src/bmp/BMP.md)
* TBB
* [PFT](/siege-modules/content/siege-content-3space/src/font/PFT.md)
* [PBA](/siege-modules/content/siege-content-3space/src/bmp/PBA.md)
* [TBA](wiki/TBA.md)

## Games

### The Incredible Machine - 1993

* [RMF](/siege-modules/content/siege-content-3space/src/RMF.md)

### The Even More Incredible Machine - 1993

* [RMF](/siege-modules/content/siege-content-3space/src/RMF.md)

### Sid & Al's Incredible Toons - 1993

* [RMF](/siege-modules/content/siege-content-3space/src/RMF.md)

### The Incredible Machine 2 - 1994

* [RMF](/siege-modules/content/siege-content-3space/src/RMF.md)

### The Incredible Machine 3 - 1995

* [RMF](/siege-modules/content/siege-content-3space/src/RMF.md)

### 3-D Ultra Pinball - 1995

* [RMF](/siege-modules/content/siege-content-3space/src/RMF.md)

### Trophy Bass - 1995

* [RMF](/siege-modules/content/siege-content-3space/src/RMF.md)
* [PAL](/siege-modules/content/siege-content-3space/src/pal/PAL.md)

### 3-D Ultra Pinball: Creep Night - 1996

* [RMF](/siege-modules/content/siege-content-3space/src/RMF.md)

### Hunter Hunted - 1996

* [RMF](/siege-modules/content/siege-content-3space/src/RMF.md)
* [PAL](/siege-modules/content/siege-content-3space/src/pal/PAL.md)

### Front Page Sports: Trophy Bass 2 - 1996

* [RMF](/siege-modules/content/siege-content-3space/src/RMF.md)
* [PAL](/siege-modules/content/siege-content-3space/src/pal/PAL.md)

### MissionForce: CyberStorm - 1997

* RBX

### Front Page Sports: Trophy Rivers - 1997

* [TBV](/siege-modules/content/siege-content-3space/src/TBV.md)

### Outpost 2: Divided Destiny - 1997

* [VOL](/siege-modules/content/siege-content-3space/src/VOL.md)

### 3-D Ultra Minigolf - 1997

* RBX

### 3-D Ultra Pinball: The Lost Continent - 1997

* [RMF](/siege-modules/content/siege-content-3space/src/RMF.md)
* [PAL](/siege-modules/content/siege-content-3space/src/pal/PAL.md)

### 3-D Ultra NASCAR Pinball - 1998

* [TBV](/siege-modules/content/siege-content-3space/src/TBV.md)
* [PAL](/siege-modules/content/siege-content-3space/src/pal/PAL.md)

### Cyberstorm 2: Corporate Wars

* RBX

### 3-D Ultra MiniGolf Deluxe - 1999

* [TBV](/siege-modules/content/siege-content-3space/src/TBV.md)
* RBX
* [PAL](/siege-modules/content/siege-content-3space/src/pal/PAL.md)

### 3-D Ultra Radio Control Racers - 1999

* [TBV](/siege-modules/content/siege-content-3space/src/TBV.md)
* RBX
* [PAL](/siege-modules/content/siege-content-3space/src/pal/PAL.md)

### 3-D Ultra Cool Pool - 1999

* [TBV](/siege-modules/content/siege-content-3space/src/TBV.md)

### 3-D Ultra Lionel Train Town - 1999

* [TBV](/siege-modules/content/siege-content-3space/src/TBV.md)

### 3-D Ultra Pinball: Thrillride - 2000

* [TBV](/siege-modules/content/siege-content-3space/src/TBV.md)

### 3-D Ultra Lionel Train Town Deluxe - 2000

* [TBV](/siege-modules/content/siege-content-3space/src/TBV.md)

### Maximum Pool - 2000

* [TBV](/siege-modules/content/siege-content-3space/src/TBV.md)

### Return of the Incredible Machine: Contraptions - 2000

* [TBV](/siege-modules/content/siege-content-3space/src/TBV.md)

### The Incredible Machine: Even More Contraptions - 2001

* [TBV](/siege-modules/content/siege-content-3space/src/TBV.md)

### Minigolf Maniacs - 2001

No support.
