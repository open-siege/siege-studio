<link rel="stylesheet" href="https://simplegrid.io/grid/simple-grid.css" />
<style>
.project-name, .project-tagline 
{
    color: white;
}
</style>

### Format Background

Despite each version of the engine having some of the same extensions for some of their formats, each iteration of the engine has completely different structures and binary layouts for said formats.

In other words, 3Space 2.0 DTS files are fundamentally different to 3Space 3.0 DTS files, which are in turn completely different to Torque DTS files.

They do share a similar high level structure and some features, but the overall format changes over time and even between games there is a big difference between the format used.

For example, while Earthsiege and Red Baron 2 might share a 3Space 2.0 core (of sorts), the DTS files themselves have different version tags for each entity and need different code to handle them. Depending on how different they are, they may need completely separate implementations for parsing and viewing.

### Game Support

Because the 3Space engine has a long history, and has morphed into engines with new names, here is a matrix of the games, most of which were made by Dynamix, (focusing specifically on DOS or Windows) which are intended to be supported or are supported:

# 3Space 1.0

### Planned file support:

* [PAL](https://github.com/open-siege/open-siege/wiki/PAL)
* [SCR](https://github.com/open-siege/open-siege/wiki/SCR)
* [BMP](https://github.com/open-siege/open-siege/wiki/BMP)
* [FNT](https://github.com/open-siege/open-siege/wiki/FNT)
* [TBL](https://github.com/open-siege/open-siege/wiki/TBL)

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

* [RMF](https://github.com/open-siege/open-siege/wiki/RMF)
* [DYN](https://github.com/open-siege/open-siege/wiki/DYN)
* [TBL](https://github.com/open-siege/open-siege/wiki/TBL)
* [PAL](https://github.com/open-siege/open-siege/wiki/PAL)
* [SCR](https://github.com/open-siege/open-siege/wiki/SCR)
* [TTM](https://github.com/open-siege/open-siege/wiki/TTM)

## Games

<div class="row">
		<div class="col-6">
		<h3>Red Baron - 1990</h3>
        <ul>
            <li><a href="https://github.com/open-siege/open-siege/wiki/RMF">RMF</a></li>
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
            <li><a href="https://github.com/open-siege/open-siege/wiki/RMF">RMF</a></li>
        </ul>
		</div>
		<div class="col-6">
		<h3>Nova 9: The Return of Gir Draxon - 1991</h3>
		<ul>
            <li><a href="https://github.com/open-siege/open-siege/wiki/RMF">RMF</a></li>
        </ul> 
		</div>
</div>

<div class="row">
		<div class="col-6">
		<h3>Aces of the Pacific - 1992</h3>
        <ul>
            <li><a href="https://github.com/open-siege/open-siege/wiki/DYN">DYN</a></li>
        </ul>
		</div>
		<div class="col-6">
		<h3>Aces Over Europe - 1993</h3>
		<ul>
            <li><a href="https://github.com/open-siege/open-siege/wiki/DYN">DYN</a></li>
        </ul> 
		</div>
</div>

<div class="row">
		<div class="col-6">
		<h3>Betrayal at Krondor - 1993</h3>
        <ul>
            <li><a href="https://github.com/open-siege/open-siege/wiki/RMF">RMF</a></li>
        </ul>
		</div>
</div>

# 3Space 2.0

## Planned file support:

* [VOL](https://github.com/open-siege/open-siege/wiki/VOL)
* [DYN](https://github.com/open-siege/open-siege/wiki/DYN)
* [PAL](https://github.com/open-siege/open-siege/wiki/PAL)
* [DPL](https://github.com/open-siege/open-siege/wiki/DPL)
* [BMP](https://github.com/open-siege/open-siege/wiki/BMP)
* [SCR](https://github.com/open-siege/open-siege/wiki/SCR)
* [DBM](https://github.com/open-siege/open-siege/wiki/DBM)
* [DBA](https://github.com/open-siege/open-siege/wiki/DBA)
* [DTS](https://github.com/open-siege/open-siege/wiki/DTS)
* [CAR](https://github.com/open-siege/open-siege/wiki/CAR)
* [MEC](https://github.com/open-siege/open-siege/wiki/MEC)

## Games

<div class="row">
		<div class="col-6">
		<h3>Aces of the Deep - 1994</h3>
        <ul>
          <li><a href="https://github.com/open-siege/open-siege/wiki/VOL">VOL</a></li>
          <li><a href="https://github.com/open-siege/open-siege/wiki/DYN">DYN</a></li>
          <li><a href="https://github.com/open-siege/open-siege/wiki/DTS">DTS</a> support in progress (feature/earthsiege-files)</li>
        </ul>
		</div>
		<div class="col-6">
		<h3>Metaltech: Battledrome - 1994</h3>
        <ul>
          <li><a href="https://github.com/open-siege/open-siege/wiki/VOL">VOL</a></li>
          <li><a href="https://github.com/open-siege/open-siege/wiki/DTS">DTS</a> support in progress (feature/earthsiege-files)</li>
        </ul>
		</div>
</div>

<div class="row">
		<div class="col-6">
		<h3>Metaltech: Earthsiege - 1994</h3>
        <ul>
          <li><a href="https://github.com/open-siege/open-siege/wiki/VOL">VOL</a></li>
          <li><a href="https://github.com/open-siege/open-siege/wiki/DTS">DTS</a> support in progress (feature/earthsiege-files)</li>
        </ul>
		</div>
		<div class="col-6">
		<h3>Command: Aces of the Deep - 1995</h3>
        <ul>
          <li><a href="https://github.com/open-siege/open-siege/wiki/VOL">VOL</a></li>
          <li><a href="https://github.com/open-siege/open-siege/wiki/DYN">DYN</a></li>
          <li><a href="https://github.com/open-siege/open-siege/wiki/DTS">DTS</a> support in progress (feature/earthsiege-files)</li>
        </ul>
		</div>
</div>

<div class="row">
		<div class="col-6">
		<h3>Earthsiege 2 - 1996</h3>
        <ul>
          <li><a href="https://github.com/open-siege/open-siege/wiki/VOL">VOL</a></li>
          <li><a href="https://github.com/open-siege/open-siege/wiki/DTS">DTS</a> support in progress (feature/earthsiege-files)</li>
        </ul>
		</div>
</div>

# 3Space 2.5

## Planned file support:

* [VOL](https://github.com/open-siege/open-siege/wiki/VOL)
* [DTS](https://github.com/open-siege/open-siege/wiki/DTS)
* [DT2](https://github.com/open-siege/open-siege/wiki/DT2)
* [DML](https://github.com/open-siege/open-siege/wiki/DML)
* [BMP](https://github.com/open-siege/open-siege/wiki/BMP)
* [PBM](https://github.com/open-siege/open-siege/wiki/PBM)
* [PBA](https://github.com/open-siege/open-siege/wiki/PBA)
* [PAL](https://github.com/open-siege/open-siege/wiki/PAL)
* [IPL](https://github.com/open-siege/open-siege/wiki/IPL)
* [PPL](https://github.com/open-siege/open-siege/wiki/PPL)

## Games

<div class="row">
		<div class="col-6">
		<h3>Silent Thunder: A-10 Tank Killer 2 - 1996</h3>
        <ul>
          <li><a href="https://github.com/open-siege/open-siege/wiki/VOL">VOL</a></li>
          <li><a href="https://github.com/open-siege/open-siege/wiki/DT2">DT2</a> support in progress (feature/earthsiege-files)</li>
          <li><a href="https://github.com/open-siege/open-siege/wiki/DML">DML</a> support in progress (feature/earthsiege-files)</li>
          <li><a href="https://github.com/open-siege/open-siege/wiki/PAL">PAL</a></li>
          <li><a href="https://github.com/open-siege/open-siege/wiki/PPL">PPL</a></li>
        </ul>
		</div>
		<div class="col-6">
		<h3>Red Baron 2 - 1997</h3>
        <ul>
          <li><a href="https://github.com/open-siege/open-siege/wiki/VOL">VOL</a> (partial support. Needs more investigation to support compression)</li>
          <li><a href="https://github.com/open-siege/open-siege/wiki/DTS">DTS</a> support in progress (feature/earthsiege-files)</li>
          <li><a href="https://github.com/open-siege/open-siege/wiki/DML">DML</a> support in progress (feature/earthsiege-files)</li>
        </ul>
		</div>
</div>

<div class="row">
		<div class="col-6">
		<h3>Pro Pilot '98 - 1997</h3>
    <ul>
      <li><a href="https://github.com/open-siege/open-siege/wiki/VOL">VOL</a> (partial support. Needs more investigation to support compression)</li>
      <li><a href="https://github.com/open-siege/open-siege/wiki/DTS">DTS</a> support in progress (feature/earthsiege-files)</li>
      <li><a href="https://github.com/open-siege/open-siege/wiki/DML">DML</a> support in progress (feature/earthsiege-files)</li>
    </ul>
		</div>
		<div class="col-6">
		<h3>Red Baron 3D - 1998</h3>
    <ul>
      <li><a href="https://github.com/open-siege/open-siege/wiki/VOL">VOL</a> (partial support. Needs more investigation to support compression)</li>
      <li><a href="https://github.com/open-siege/open-siege/wiki/DTS">DTS</a> support in progress (feature/earthsiege-files)</li>
      <li><a href="https://github.com/open-siege/open-siege/wiki/DML">DML</a> support in progress (feature/earthsiege-files)</li>
    </ul>
		</div>
</div>

<div class="row">
		<div class="col-6">
		<h3>Pro Pilot '99 - 1998</h3>
        <ul>
          <li><a href="https://github.com/open-siege/open-siege/wiki/VOL">VOL</a> (partial support. Needs more investigation to support compression)</li>
          <li><a href="https://github.com/open-siege/open-siege/wiki/DTS">DTS</a> support in progress (feature/earthsiege-files)</li>
          <li><a href="https://github.com/open-siege/open-siege/wiki/DML">DML</a> support in progress (feature/earthsiege-files)</li>
        </ul>
		</div>
		<div class="col-6">
		<h3>Kid Pilot - 1998</h3>
        <ul>
          <li><a href="https://github.com/open-siege/open-siege/wiki/VOL">VOL</a> (partial support. Needs more investigation to support compression)</li>
          <li><a href="https://github.com/open-siege/open-siege/wiki/DTS">DTS</a> support in progress (feature/earthsiege-files)</li>
          <li><a href="https://github.com/open-siege/open-siege/wiki/DML">DML</a> support in progress (feature/earthsiege-files)</li>
        </ul>
		</div>
</div>

<div class="row">
		<div class="col-6">
		<h3>Curse You! Red Baron - 1999</h3>
        <ul>
          <li><a href="https://github.com/open-siege/open-siege/wiki/VOL">VOL</a> (partial support. Needs more investigation to support compression)</li>
          <li><a href="https://github.com/open-siege/open-siege/wiki/DTS">DTS</a> support in progress (feature/earthsiege-files)</li>
          <li><a href="https://github.com/open-siege/open-siege/wiki/DML">DML</a> support in progress (feature/earthsiege-files)</li>
        </ul>
		</div>
</div>

# 3Space 3.0 aka Darkstar

## Planned file support:

* [VOL](https://github.com/open-siege/open-siege/wiki/VOL)
* [DTS](https://github.com/open-siege/open-siege/wiki/DTS)
* [DML](https://github.com/open-siege/open-siege/wiki/DML)
* [KQS](https://github.com/open-siege/open-siege/wiki/KQS)
* [BMP](https://github.com/open-siege/open-siege/wiki/BMP)
* [PBM](https://github.com/open-siege/open-siege/wiki/PBM)
* [PBA](https://github.com/open-siege/open-siege/wiki/PBA)
* [PFT](https://github.com/open-siege/open-siege/wiki/PFT)
* [PAL](https://github.com/open-siege/open-siege/wiki/PAL)
* [IPL](https://github.com/open-siege/open-siege/wiki/IPL)
* [PPL](https://github.com/open-siege/open-siege/wiki/PPL)
* [CS](https://github.com/open-siege/open-siege/wiki/CS)

## Games

<div class="row">
		<div class="col-6">
		<h3>Front Page Sports: Ski Racing - 1997</h3>
        <ul>
                  <li><a href="https://github.com/open-siege/open-siege/wiki/VOL">VOL</a></li>
                  <li><a href="https://github.com/open-siege/open-siege/wiki/TBV">TBV</a></li>
                  <li><a href="https://github.com/open-siege/open-siege/wiki/DML">DML</a></li>
                  <li><a href="https://github.com/open-siege/open-siege/wiki/PAL">PAL</a></li>
                  <li><a href="https://github.com/open-siege/open-siege/wiki/PPL">PPL</a></li>
                  <li><a href="https://github.com/open-siege/open-siege/wiki/BMP">BMP</a></li>
                  <li><a href="https://github.com/open-siege/open-siege/wiki/PBA">PBA</a></li>
        </ul>
		</div>
		<div class="col-6">
		<h3>King's Quest: Mask of Eternity - 1998</h3>
        <ul>
                  <li><a href="https://github.com/open-siege/open-siege/wiki/VOL">VOL</a></li>
                  <li><a href="https://github.com/open-siege/open-siege/wiki/DML">DML</a></li>
                  <li><a href="https://github.com/open-siege/open-siege/wiki/PAL">PAL</a></li>
                  <li><a href="https://github.com/open-siege/open-siege/wiki/PPL">PPL</a></li>
                  <li><a href="https://github.com/open-siege/open-siege/wiki/BMP">BMP</a></li>
                  <li><a href="https://github.com/open-siege/open-siege/wiki/PBA">PBA</a></li>
        </ul>
		</div>
</div>

<div class="row">
		<div class="col-6">
		<h3>Driver's Education '98 - 1998</h3>
        <ul>
          <li><a href="https://github.com/open-siege/open-siege/wiki/VOL">VOL</a></li>
          <li><a href="https://github.com/open-siege/open-siege/wiki/DTS">DTS</a></li>
          <li><a href="https://github.com/open-siege/open-siege/wiki/DML">DML</a></li>
          <li><a href="https://github.com/open-siege/open-siege/wiki/PAL">PAL</a></li>
          <li><a href="https://github.com/open-siege/open-siege/wiki/PPL">PPL</a></li>
          <li><a href="https://github.com/open-siege/open-siege/wiki/BMP">BMP</a></li>
          <li><a href="https://github.com/open-siege/open-siege/wiki/PBA">PBA</a></li>
        </ul>
		</div>
		<div class="col-6">
		<h3>Starsiege - 1999</h3>
        <ul>
          <li><a href="https://github.com/open-siege/open-siege/wiki/VOL">VOL</a></li>
          <li><a href="https://github.com/open-siege/open-siege/wiki/DTS">DTS</a></li>
          <li><a href="https://github.com/open-siege/open-siege/wiki/DML">DML</a></li>
          <li><a href="https://github.com/open-siege/open-siege/wiki/PAL">PAL</a></li>
          <li><a href="https://github.com/open-siege/open-siege/wiki/PPL">PPL</a></li>
          <li><a href="https://github.com/open-siege/open-siege/wiki/BMP">BMP</a></li>
          <li><a href="https://github.com/open-siege/open-siege/wiki/PBA">PBA</a></li>
          <li><a href="https://github.com/open-siege/open-siege/wiki/CS">CS</a> (partial grammar implemented)</li>
        </ul>
		</div>
</div>

<div class="row">
		<div class="col-6">
		<h3>Starsiege: Tribes - 1999</h3>
        <ul>
          <li><a href="https://github.com/open-siege/open-siege/wiki/VOL">VOL</a></li>
          <li><a href="https://github.com/open-siege/open-siege/wiki/DTS">DTS</a></li>
          <li><a href="https://github.com/open-siege/open-siege/wiki/DML">DML</a></li>
          <li><a href="https://github.com/open-siege/open-siege/wiki/PAL">PAL</a></li>
          <li><a href="https://github.com/open-siege/open-siege/wiki/PPL">PPL</a></li>
          <li><a href="https://github.com/open-siege/open-siege/wiki/BMP">BMP</a></li>
          <li><a href="https://github.com/open-siege/open-siege/wiki/PBA">PBA</a></li>
          <li><a href="https://github.com/open-siege/open-siege/wiki/CS">CS</a> (partial grammar implemented)</li>
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
          <li><a href="https://github.com/open-siege/open-siege/wiki/VOL">VOL</a></li>
          <li><a href="https://github.com/open-siege/open-siege/wiki/DTS">DTS</a></li>
          <li><a href="https://github.com/open-siege/open-siege/wiki/DML">DML</a></li>
          <li><a href="https://github.com/open-siege/open-siege/wiki/PAL">PAL</a></li>
          <li><a href="https://github.com/open-siege/open-siege/wiki/PPL">PPL</a></li>
          <li><a href="https://github.com/open-siege/open-siege/wiki/BMP">BMP</a></li>
          <li><a href="https://github.com/open-siege/open-siege/wiki/PBA">PBA</a></li>
        </ul>
		</div>
		<div class="col-6">
		<h3>Field & Stream: Trophy Bass 3D - 1999</h3>
        <ul>
          <li><a href="https://github.com/open-siege/open-siege/wiki/VOL">VOL</a></li>
          <li><a href="https://github.com/open-siege/open-siege/wiki/DTS">DTS</a></li>
          <li><a href="https://github.com/open-siege/open-siege/wiki/DML">DML</a></li>
          <li><a href="https://github.com/open-siege/open-siege/wiki/PAL">PAL</a></li>
          <li><a href="https://github.com/open-siege/open-siege/wiki/PPL">PPL</a></li>
          <li><a href="https://github.com/open-siege/open-siege/wiki/BMP">BMP</a></li>
          <li><a href="https://github.com/open-siege/open-siege/wiki/PBA">PBA</a></li>
        </ul>
		</div>
</div>

<div class="row">
		<div class="col-6">
		<h3>Field & Stream: Trophy Bass 4 - 2000</h3>
        <ul>
          <li><a href="https://github.com/open-siege/open-siege/wiki/VOL">VOL</a></li>
          <li><a href="https://github.com/open-siege/open-siege/wiki/DTS">DTS</a></li>
          <li><a href="https://github.com/open-siege/open-siege/wiki/DML">DML</a></li>
          <li><a href="https://github.com/open-siege/open-siege/wiki/PAL">PAL</a></li>
          <li><a href="https://github.com/open-siege/open-siege/wiki/PPL">PPL</a></li>
          <li><a href="https://github.com/open-siege/open-siege/wiki/BMP">BMP</a></li>
          <li><a href="https://github.com/open-siege/open-siege/wiki/PBA">PBA</a></li>
        </ul>
		</div>
</div>

# Torque

## Planned file support:

* [VL2](https://github.com/open-siege/open-siege/wiki/VL2)
* [VOL](https://github.com/open-siege/open-siege/wiki/VOL)
* [DTS](https://github.com/open-siege/open-siege/wiki/DTS)
* [DSQ](https://github.com/open-siege/open-siege/wiki/DSQ)
* [CS](https://github.com/open-siege/open-siege/wiki/CS)
* [DSO](https://github.com/open-siege/open-siege/wiki/DSO)

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

* [RMF](https://github.com/open-siege/open-siege/wiki/RMF)
* [TBL](https://github.com/open-siege/open-siege/wiki/TBL)
* [PAL](https://github.com/open-siege/open-siege/wiki/PAL)
* [SCR](https://github.com/open-siege/open-siege/wiki/SCR)
* [BMP](https://github.com/open-siege/open-siege/wiki/BMP)
* [TTM](https://github.com/open-siege/open-siege/wiki/TTM)

## Games

<div class="row">
		<div class="col-6">
		<h3>Rise of the Dragon - 1990</h3>
        <ul>
          <li><a href="https://github.com/open-siege/open-siege/wiki/RMF">RMF</a></li>
        </ul>
		</div>
		<div class="col-6">
		<h3>Heart of China - 1991</h3>
        <ul>
          <li><a href="https://github.com/open-siege/open-siege/wiki/RMF">RMF</a></li>
        </ul>
		</div>
</div>

<div class="row">
		<div class="col-6">
		<h3>The Adventures of Willy Beamish - 1991</h3>
        <ul>
          <li><a href="https://github.com/open-siege/open-siege/wiki/RMF">RMF</a></li>
        </ul>
		</div>
		<div class="col-6">
		<h3>Quarky & Quaysoo's Turbo Science - 1992</h3>
        <ul>
          <li><a href="https://github.com/open-siege/open-siege/wiki/RMF">RMF</a></li>
        </ul>
		</div>
</div>

# Dynamix 2D/2.5D Game Engines

## Planned file support:

* [RMF](https://github.com/open-siege/open-siege/wiki/RMF)
* [RBX](https://github.com/open-siege/open-siege/wiki/RBX)
* [TBV](https://github.com/open-siege/open-siege/wiki/TBV)
* [VOL](https://github.com/open-siege/open-siege/wiki/VOL)
* [TBL](https://github.com/open-siege/open-siege/wiki/TBL)
* [PAL](https://github.com/open-siege/open-siege/wiki/PAL)
* [SCR](https://github.com/open-siege/open-siege/wiki/SCR)
* [BMP](https://github.com/open-siege/open-siege/wiki/BMP)
* [TBB](https://github.com/open-siege/open-siege/wiki/TBB)
* [PFT](https://github.com/open-siege/open-siege/wiki/PFT)
* [PBA](https://github.com/open-siege/open-siege/wiki/PBA)
* [TBA](https://github.com/open-siege/open-siege/wiki/TBA)

## Games

### The Incredible Machine - 1993

* [RMF](https://github.com/open-siege/open-siege/wiki/RMF)

### The Even More Incredible Machine - 1993

* [RMF](https://github.com/open-siege/open-siege/wiki/RMF)

### Sid & Al's Incredible Toons - 1993

* [RMF](https://github.com/open-siege/open-siege/wiki/RMF)

### The Incredible Machine 2 - 1994

* [RMF](https://github.com/open-siege/open-siege/wiki/RMF)

### The Incredible Machine 3 - 1995

* [RMF](https://github.com/open-siege/open-siege/wiki/RMF)

### 3-D Ultra Pinball - 1995

* [RMF](https://github.com/open-siege/open-siege/wiki/RMF)

### Trophy Bass - 1995

* [RMF](https://github.com/open-siege/open-siege/wiki/RMF)
* [PAL](https://github.com/open-siege/open-siege/wiki/PAL)

### 3-D Ultra Pinball: Creep Night - 1996

* [RMF](https://github.com/open-siege/open-siege/wiki/RMF)

### Hunter Hunted - 1996

* [RMF](https://github.com/open-siege/open-siege/wiki/RMF)
* [PAL](https://github.com/open-siege/open-siege/wiki/PAL)

### Front Page Sports: Trophy Bass 2 - 1996

* [RMF](https://github.com/open-siege/open-siege/wiki/RMF)
* [PAL](https://github.com/open-siege/open-siege/wiki/PAL)

### MissionForce: CyberStorm - 1997

* [RBX](https://github.com/open-siege/open-siege/wiki/RBX)

### Front Page Sports: Trophy Rivers - 1997

* [TBV](https://github.com/open-siege/open-siege/wiki/TBV)

### Outpost 2: Divided Destiny - 1997

* [VOL](https://github.com/open-siege/open-siege/wiki/VOL)

### 3-D Ultra Minigolf - 1997

* [RBX](https://github.com/open-siege/open-siege/wiki/RBX)

### 3-D Ultra Pinball: The Lost Continent - 1997

* [RMF](https://github.com/open-siege/open-siege/wiki/RMF)
* [PAL](https://github.com/open-siege/open-siege/wiki/PAL)

### 3-D Ultra NASCAR Pinball - 1998

* [TBV](https://github.com/open-siege/open-siege/wiki/TBV)
* [PAL](https://github.com/open-siege/open-siege/wiki/PAL)

### Cyberstorm 2: Corporate Wars

* [RBX](https://github.com/open-siege/open-siege/wiki/RBX)

### 3-D Ultra MiniGolf Deluxe - 1999

* [TBV](https://github.com/open-siege/open-siege/wiki/TBV)
* [RBX](https://github.com/open-siege/open-siege/wiki/RBX)
* [PAL](https://github.com/open-siege/open-siege/wiki/PAL)

### 3-D Ultra Radio Control Racers - 1999

* [TBV](https://github.com/open-siege/open-siege/wiki/TBV)
* [RBX](https://github.com/open-siege/open-siege/wiki/RBX)
* [PAL](https://github.com/open-siege/open-siege/wiki/PAL)

### 3-D Ultra Cool Pool - 1999

* [TBV](https://github.com/open-siege/open-siege/wiki/TBV)

### 3-D Ultra Lionel Train Town - 1999

* [TBV](https://github.com/open-siege/open-siege/wiki/TBV)

### 3-D Ultra Pinball: Thrillride - 2000

* [TBV](https://github.com/open-siege/open-siege/wiki/TBV)

### 3-D Ultra Lionel Train Town Deluxe - 2000

* [TBV](https://github.com/open-siege/open-siege/wiki/TBV)

### Maximum Pool - 2000

* [TBV](https://github.com/open-siege/open-siege/wiki/TBV)

### Return of the Incredible Machine: Contraptions - 2000

* [TBV](https://github.com/open-siege/open-siege/wiki/TBV)

### The Incredible Machine: Even More Contraptions - 2001

* [TBV](https://github.com/open-siege/open-siege/wiki/TBV)

### Minigolf Maniacs - 2001

No support.
