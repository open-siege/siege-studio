### Format Background

Despite each version of the engine having some of the same extensions for some of their formats, each iteration of the engine has completely different structures and binary layouts for said formats.

In other words, 3Space 2.0 DTS files are fundamentally different to 3Space 3.0 DTS files, which are in turn completely different to Torque DTS files.

They do share a similar high level structure and some features, but the overall format changes over time and even between games there is a big difference between the format used.

For example, while Earthsiege and Red Baron 2 might share a 3Space 2.0 core (of sorts), the DTS files themselves have different version tags for each entity and need different code to handle them. Depending on how different they are, they may need completely separate implementations for parsing and viewing.

### Game Support

Because the 3Space engine has a long history, and has morphed into engines with new names, here is a matrix of the games, most of which were made by Dynamix, (focusing specifically on DOS or Windows) which are intended to be supported or are supported:

# 3Space 1.0

### Planned file support:

* [PAL](wiki/PAL.md)
* [SCR](wiki/SCR.md)
* [BMP](wiki/BMP.md)
* [FNT](wiki/FNT.md)
* [TBL](wiki/TBL.md)

## Games

<div class="row">
		<div class="col-6">
		<h3>Arcticfox - 1987 (for DOS)</h3>
		No support. Unknown file formats.
		</div>
		<div class="col-6">
		<h3>A-10 Tank Killer - 1989 (for DOS)</h3>
		No support. 
		</div>
</div>

<div class="row">
		<div class="col-6">
		<h3>Abrams Battle Tank - 1989 (for DOS)</h3>
		No support.
		</div>
		<div class="col-6">
		<h3>David Wolf Secret Agent - 1989 (for DOS)</h3>
		No support. 
		</div>
</div>

<div class="row">
		<div class="col-6">
		<h3>DeathTrack - 1989</h3>
		No support.
		</div>
		<div class="col-6">
		<h3>Die Hard - 1989</h3>
		No support. 
		</div>
</div>

<div class="row">
		<div class="col-6">
		<h3>MechWarrior - 1989 (for DOS)</h3>
		No support.
		</div>
		<div class="col-6">
		<h3>F-14 Tomcat - 1990 (for DOS)</h3>
		No support. 
		</div>
</div>

# 3Space 1.5

## Planned file support:

* [RMF](wiki/RMF.md)
* [DYN](wiki/DYN.md)
* [TBL](wiki/TBL.md)
* [PAL](wiki/PAL.md)
* [SCR](wiki/SCR.md)
* [TTM](wiki/TTM.md)

## Games

<div class="row">
		<div class="col-6">
		<h3>Red Baron - 1990</h3>
        <ul>
            <li><a href="wiki/RMF.md">RMF</a></li>
        </ul>
		</div>
		<div class="col-6">
		<h3>Stellar 7 (re-release) - 1990</h3>
		No support. 
		</div>
</div>

<div class="row">
		<div class="col-6">
		<h3>A-10 Tank Killer 1.5 - 1991</h3>
        <ul>
            <li><a href="wiki/RMF.md">RMF</a></li>
        </ul>
		</div>
		<div class="col-6">
		<h3>Nova 9: The Return of Gir Draxon - 1991</h3>
		<ul>
            <li><a href="wiki/RMF.md">RMF</a></li>
        </ul> 
		</div>
</div>

<div class="row">
		<div class="col-6">
		<h3>Aces of the Pacific - 1992</h3>
        <ul>
            <li><a href="wiki/DYN.md">DYN</a></li>
        </ul>
		</div>
		<div class="col-6">
		<h3>Aces Over Europe - 1993</h3>
		<ul>
            <li><a href="wiki/DYN.md">DYN</a></li>
        </ul> 
		</div>
</div>

<div class="row">
		<div class="col-6">
		<h3>Betrayal at Krondor - 1993</h3>
        <ul>
            <li><a href="wiki/RMF.md">RMF</a></li>
        </ul>
		</div>
</div>

# 3Space 2.0

## Planned file support:

* [VOL](wiki/VOL.md)
* [DYN](wiki/DYN.md)
* [PAL](wiki/PAL.md)
* [DPL](wiki/DPL.md)
* [BMP](wiki/BMP.md)
* [SCR](wiki/SCR.md)
* [DBM](wiki/DBM.md)
* [DBA](wiki/DBA.md)
* [DTS](wiki/DTS.md)
* [CAR](wiki/CAR.md)
* [MEC](wiki/MEC.md)

## Games

<div class="row">
		<div class="col-6">
		<h3>Aces of the Deep - 1994</h3>
        <ul>
          <li><a href="wiki/VOL.md">VOL</a></li>
          <li><a href="wiki/DYN.md">DYN</a></li>
          <li><a href="wiki/DTS.md">DTS</a> support in progress (feature/earthsiege-files)</li>
        </ul>
		</div>
		<div class="col-6">
		<h3>Metaltech: Battledrome - 1994</h3>
        <ul>
          <li><a href="wiki/VOL.md">VOL</a></li>
          <li><a href="wiki/DTS.md">DTS</a> support in progress (feature/earthsiege-files)</li>
        </ul>
		</div>
</div>

<div class="row">
		<div class="col-6">
		<h3>Metaltech: Earthsiege - 1994</h3>
        <ul>
          <li><a href="wiki/VOL.md">VOL</a></li>
          <li><a href="wiki/DTS.md">DTS</a> support in progress (feature/earthsiege-files)</li>
        </ul>
		</div>
		<div class="col-6">
		<h3>Command: Aces of the Deep - 1995</h3>
        <ul>
          <li><a href="wiki/VOL.md">VOL</a></li>
          <li><a href="wiki/DYN.md">DYN</a></li>
          <li><a href="wiki/DTS.md">DTS</a> support in progress (feature/earthsiege-files)</li>
        </ul>
		</div>
</div>

<div class="row">
		<div class="col-6">
		<h3>Earthsiege 2 - 1996</h3>
        <ul>
          <li><a href="wiki/VOL.md">VOL</a></li>
          <li><a href="wiki/DTS.md">DTS</a> support in progress (feature/earthsiege-files)</li>
        </ul>
		</div>
</div>

# 3Space 2.5

## Planned file support:

* [VOL](wiki/VOL.md)
* [DTS](wiki/DTS.md)
* [DT2](wiki/DT2.md)
* [DML](wiki/DML.md)
* [BMP](wiki/BMP.md)
* [PBM](wiki/PBM.md)
* [PBA](wiki/PBA.md)
* [PAL](wiki/PAL.md)
* [IPL](wiki/IPL.md)
* [PPL](wiki/PPL.md)

## Games

<div class="row">
		<div class="col-6">
		<h3>Silent Thunder: A-10 Tank Killer 2 - 1996</h3>
        <ul>
          <li><a href="wiki/VOL.md">VOL</a></li>
          <li><a href="wiki/DT2.md">DT2</a> support in progress (feature/earthsiege-files)</li>
          <li><a href="wiki/DML.md">DML</a> support in progress (feature/earthsiege-files)</li>
          <li><a href="wiki/PAL.md">PAL</a></li>
          <li><a href="wiki/PPL.md">PPL</a></li>
        </ul>
		</div>
		<div class="col-6">
		<h3>Red Baron 2 - 1997</h3>
        <ul>
          <li><a href="wiki/VOL.md">VOL</a> (partial support. Needs more investigation to support compression)</li>
          <li><a href="wiki/DTS.md">DTS</a> support in progress (feature/earthsiege-files)</li>
          <li><a href="wiki/DML.md">DML</a> support in progress (feature/earthsiege-files)</li>
        </ul>
		</div>
</div>

<div class="row">
		<div class="col-6">
		<h3>Pro Pilot '98 - 1997</h3>
    <ul>
      <li><a href="wiki/VOL.md">VOL</a> (partial support. Needs more investigation to support compression)</li>
      <li><a href="wiki/DTS.md">DTS</a> support in progress (feature/earthsiege-files)</li>
      <li><a href="wiki/DML.md">DML</a> support in progress (feature/earthsiege-files)</li>
    </ul>
		</div>
		<div class="col-6">
		<h3>Red Baron 3D - 1998</h3>
    <ul>
      <li><a href="wiki/VOL.md">VOL</a> (partial support. Needs more investigation to support compression)</li>
      <li><a href="wiki/DTS.md">DTS</a> support in progress (feature/earthsiege-files)</li>
      <li><a href="wiki/DML.md">DML</a> support in progress (feature/earthsiege-files)</li>
    </ul>
		</div>
</div>

<div class="row">
		<div class="col-6">
		<h3>Pro Pilot '99 - 1998</h3>
        <ul>
          <li><a href="wiki/VOL.md">VOL</a> (partial support. Needs more investigation to support compression)</li>
          <li><a href="wiki/DTS.md">DTS</a> support in progress (feature/earthsiege-files)</li>
          <li><a href="wiki/DML.md">DML</a> support in progress (feature/earthsiege-files)</li>
        </ul>
		</div>
		<div class="col-6">
		<h3>Kid Pilot - 1998</h3>
        <ul>
          <li><a href="wiki/VOL.md">VOL</a> (partial support. Needs more investigation to support compression)</li>
          <li><a href="wiki/DTS.md">DTS</a> support in progress (feature/earthsiege-files)</li>
          <li><a href="wiki/DML.md">DML</a> support in progress (feature/earthsiege-files)</li>
        </ul>
		</div>
</div>

<div class="row">
		<div class="col-6">
		<h3>Curse You! Red Baron - 1999</h3>
        <ul>
          <li><a href="wiki/VOL.md">VOL</a> (partial support. Needs more investigation to support compression)</li>
          <li><a href="wiki/DTS.md">DTS</a> support in progress (feature/earthsiege-files)</li>
          <li><a href="wiki/DML.md">DML</a> support in progress (feature/earthsiege-files)</li>
        </ul>
		</div>
</div>

# 3Space 3.0 aka Darkstar

## Planned file support:

* [VOL](wiki/VOL.md)
* [DTS](wiki/DTS.md)
* [DML](wiki/DML.md)
* [KQS](wiki/KQS.md)
* [BMP](wiki/BMP.md)
* [PBM](wiki/PBM.md)
* [PBA](wiki/PBA.md)
* [PFT](wiki/PFT.md)
* [PAL](wiki/PAL.md)
* [IPL](wiki/IPL.md)
* [PPL](wiki/PPL.md)
* [CS](wiki/CS.md)

## Games

<div class="row">
		<div class="col-6">
		<h3>Front Page Sports: Ski Racing - 1997</h3>
        <ul>
                  <li><a href="wiki/VOL.md">VOL</a></li>
                  <li><a href="wiki/TBV.md">TBV</a></li>
                  <li><a href="wiki/DML.md">DML</a></li>
                  <li><a href="wiki/PAL">PAL</a></li>
                  <li><a href="wiki/PPL">PPL</a></li>
                  <li><a href="wiki/BMP">BMP</a></li>
                  <li><a href="wiki/PBA">PBA</a></li>
        </ul>
		</div>
		<div class="col-6">
		<h3>King's Quest: Mask of Eternity - 1998</h3>
        <ul>
                  <li><a href="wiki/VOL.md">VOL</a></li>
                  <li><a href="wiki/DML.md">DML</a></li>
                  <li><a href="wiki/PAL.md">PAL</a></li>
                  <li><a href="wiki/PPL.md">PPL</a></li>
                  <li><a href="wiki/BMP.md">BMP</a></li>
                  <li><a href="wiki/PBA.md">PBA</a></li>
        </ul>
		</div>
</div>

<div class="row">
		<div class="col-6">
		<h3>Driver's Education '98 - 1998</h3>
        <ul>
          <li><a href="wiki/VOL.md">VOL</a></li>
          <li><a href="wiki/DTS.md">DTS</a></li>
          <li><a href="wiki/DML.md">DML</a></li>
          <li><a href="wiki/PAL.md">PAL</a></li>
          <li><a href="wiki/PPL.md">PPL</a></li>
          <li><a href="wiki/BMP.md">BMP</a></li>
          <li><a href="wiki/PBA.md">PBA</a></li>
        </ul>
		</div>
		<div class="col-6">
		<h3>Starsiege - 1999</h3>
        <ul>
          <li><a href="wiki/VOL.md">VOL</a></li>
          <li><a href="wiki/DTS.md">DTS</a></li>
          <li><a href="wiki/DML.md">DML</a></li>
          <li><a href="wiki/PAL.md">PAL</a></li>
          <li><a href="wiki/PPL.md">PPL</a></li>
          <li><a href="wiki/BMP.md">BMP</a></li>
          <li><a href="wiki/PBA.md">PBA</a></li>
          <li><a href="wiki/CS.md">CS</a> (partial grammar implemented)</li>
        </ul>
		</div>
</div>

<div class="row">
		<div class="col-6">
		<h3>Starsiege: Tribes - 1999</h3>
        <ul>
          <li><a href="wiki/VOL.md">VOL</a></li>
          <li><a href="wiki/DTS.md">DTS</a></li>
          <li><a href="wiki/DML.md">DML</a></li>
          <li><a href="wiki/PAL.md">PAL</a></li>
          <li><a href="wiki/PPL.md">PPL</a></li>
          <li><a href="wiki/BMP.md">BMP</a></li>
          <li><a href="wiki/PBA.md">PBA</a></li>
          <li><a href="wiki/CS.md">CS</a> (partial grammar implemented)</li>
        </ul>
		</div>
		<div class="col-6">
		<h3>Desert Fighters - 1999</h3>
        No support.
		</div>
</div>

<div class="row">
		<div class="col-6">
		<h3>Driver's Education '99 - 1999</h3>
        <ul>
          <li><a href="wiki/VOL.md">VOL</a></li>
          <li><a href="wiki/DTS.md">DTS</a></li>
          <li><a href="wiki/DML.md">DML</a></li>
          <li><a href="wiki/PAL.md">PAL</a></li>
          <li><a href="wiki/PPL.md">PPL</a></li>
          <li><a href="wiki/BMP.md">BMP</a></li>
          <li><a href="wiki/PBA.md">PBA</a></li>
        </ul>
		</div>
		<div class="col-6">
		<h3>Field & Stream: Trophy Bass 3D - 1999</h3>
        <ul>
          <li><a href="wiki/VOL.md">VOL</a></li>
          <li><a href="wiki/DTS.md">DTS</a></li>
          <li><a href="wiki/DML.md">DML</a></li>
          <li><a href="wiki/PAL.md">PAL</a></li>
          <li><a href="wiki/PPL.md">PPL</a></li>
          <li><a href="wiki/BMP.md">BMP</a></li>
          <li><a href="wiki/PBA.md">PBA</a></li>
        </ul>
		</div>
</div>

<div class="row">
		<div class="col-6">
		<h3>Field & Stream: Trophy Bass 4 - 2000</h3>
        <ul>
          <li><a href="wiki/VOL.md">VOL</a></li>
          <li><a href="wiki/DTS.md">DTS</a></li>
          <li><a href="wiki/DML.md">DML</a></li>
          <li><a href="wiki/PAL.md">PAL</a></li>
          <li><a href="wiki/PPL.md">PPL</a></li>
          <li><a href="wiki/BMP.md">BMP</a></li>
          <li><a href="wiki/PBA.md">PBA</a></li>
        </ul>
		</div>
</div>

# Torque

## Planned file support:

* [VL2](wiki/VL2.md)
* [VOL](wiki/VOL.md)
* [DTS](wiki/DTS.md)
* [DSQ](wiki/DSQ.md)
* [CS](wiki/CS.md)
* [DSO](wiki/DSO.md)

## Games

<div class="row">
		<div class="col-6">
		<h3>Field & Stream: Trophy Hunting 4 - 2000</h3>
        No support.
		</div>
		<div class="col-6">
		<h3>Field & Stream: Trophy Hunting 5 - 2001</h3>
        No support.
		</div>
</div>

<div class="row">
		<div class="col-6">
		<h3>Tribes 2 - 2001</h3>
        No support.
		</div>
</div>

# Dynamix Game Development System

## Planned file support:

* [RMF](wiki/RMF.md)
* [TBL](wiki/TBL.md)
* [PAL](wiki/PAL.md)
* [SCR](wiki/SCR.md)
* [BMP](wiki/BMP.md)
* [TTM](wiki/TTM.md)

## Games

<div class="row">
		<div class="col-6">
		<h3>Rise of the Dragon - 1990</h3>
        <ul>
          <li><a href="wiki/RMF.md">RMF</a></li>
        </ul>
		</div>
		<div class="col-6">
		<h3>Heart of China - 1991</h3>
        <ul>
          <li><a href="wiki/RMF.md">RMF</a></li>
        </ul>
		</div>
</div>

<div class="row">
		<div class="col-6">
		<h3>The Adventures of Willy Beamish - 1991</h3>
        <ul>
          <li><a href="wiki/RMF.md">RMF</a></li>
        </ul>
		</div>
		<div class="col-6">
		<h3>Quarky & Quaysoo's Turbo Science - 1992</h3>
        <ul>
          <li><a href="wiki/RMF">RMF.md</a></li>
        </ul>
		</div>
</div>

# Dynamix 2D/2.5D Game Engines

## Planned file support:

* [RMF](wiki/RMF.md)
* [RBX](wiki/RBX.md)
* [TBV](wiki/TBV.md)
* [VOL](wiki/VOL.md)
* [TBL](wiki/TBL.md)
* [PAL](wiki/PAL.md)
* [SCR](wiki/SCR.md)
* [BMP](wiki/BMP.md)
* [TBB](wiki/TBB.md)
* [PFT](wiki/PFT.md)
* [PBA](wiki/PBA.md)
* [TBA](wiki/TBA.md)

## Games

### The Incredible Machine - 1993

* [RMF](wiki/RMF.md)

### The Even More Incredible Machine - 1993

* [RMF](wiki/RMF.md)

### Sid & Al's Incredible Toons - 1993

* [RMF](wiki/RMF.md)

### The Incredible Machine 2 - 1994

* [RMF](wiki/RMF.md)

### The Incredible Machine 3 - 1995

* [RMF](wiki/RMF.md)

### 3-D Ultra Pinball - 1995

* [RMF](wiki/RMF.md)

### Trophy Bass - 1995

* [RMF](wiki/RMF.md)
* [PAL](wiki/PAL.md)

### 3-D Ultra Pinball: Creep Night - 1996

* [RMF](wiki/RMF.md)

### Hunter Hunted - 1996

* [RMF](wiki/RMF.md)
* [PAL](wiki/PAL.md)

### Front Page Sports: Trophy Bass 2 - 1996

* [RMF](wiki/RMF.md)
* [PAL](wiki/PAL.md)

### MissionForce: CyberStorm - 1997

* [RBX](wiki/RBX.md)

### Front Page Sports: Trophy Rivers - 1997

* [TBV](wiki/TBV.md)

### Outpost 2: Divided Destiny - 1997

* [VOL](wiki/VOL.md)

### 3-D Ultra Minigolf - 1997

* [RBX](wiki/RBX.md)

### 3-D Ultra Pinball: The Lost Continent - 1997

* [RMF](wiki/RMF.md)
* [PAL](wiki/PAL.md)

### 3-D Ultra NASCAR Pinball - 1998

* [TBV](wiki/TBV.md)
* [PAL](wiki/PAL.md)

### Cyberstorm 2: Corporate Wars

* [RBX](wiki/RBX.md)

### 3-D Ultra MiniGolf Deluxe - 1999

* [TBV](wiki/TBV.md)
* [RBX](wiki/RBX.md)
* [PAL](wiki/PAL.md)

### 3-D Ultra Radio Control Racers - 1999

* [TBV](wiki/TBV.md)
* [RBX](wiki/RBX.md)
* [PAL](wiki/PAL.md)

### 3-D Ultra Cool Pool - 1999

* [TBV](wiki/TBV.md)

### 3-D Ultra Lionel Train Town - 1999

* [TBV](wiki/TBV.md)

### 3-D Ultra Pinball: Thrillride - 2000

* [TBV](wiki/TBV.md)

### 3-D Ultra Lionel Train Town Deluxe - 2000

* [TBV](wiki/TBV.md)

### Maximum Pool - 2000

* [TBV](wiki/TBV.md)

### Return of the Incredible Machine: Contraptions - 2000

* [TBV](wiki/TBV.md)

### The Incredible Machine: Even More Contraptions - 2001

* [TBV](wiki/TBV.md)

### Minigolf Maniacs - 2001

No support.
