
import sharedInfo
from functools import partial

def createExecContext():
    result = {
        "internalMounts": [],
        "currentMount": {},
        "f": False,
        "t": True,
        "Computer": "computer",
        "ECM": "ecm",
        "Cloak": "cloak",
        "ThermalDiffuser": "thermalDiffuser",
        "ShieldModulator": "shieldModulator",
        "ShieldCapacitor": "shieldCapacitor",
        "ShieldAmplifier": "shieldAmplifier",
        "LaserTargeting": "laserTargeting",
        "Battery": "battery",
        "Capacitor": "capacitor",
        "FieldStabilizer": "fieldStabilizer",
        "RocketBooster": "rocketBooster",
        "TurbineBoost": "turbineBoost",
        "NanoRepair": "nanoRepair",
        "LifeSupport": "lifeSupport",
        "AgravGenerator": "agravGenerator",
        "electroHull": "electroHull",
        "ammoPack": "ammoPack",
        "A": "all",
        "H": "human",
        "C": "cybrid",
        "X": "xeno",
        "x": "xl",
        "S": "small",
        "L": "large",
        "M": "medium"
    }

    result["newComputer"] = partial(newComputer, result)
    result["newECM"] = partial(newECM, result)
    result["newThermal"] = partial(newThermal, result)
    result["newCloak"] = partial(newCloak, result)
    result["newModulator"] = partial(newModulator, result)
    result["newCapacitor"] = partial(newCapacitor, result)
    result["newAmplifier"] = partial(newAmplifier, result)
    result["newMountable"] = partial(newMountable, result)
    result["newBattery"] = partial(newBattery, result)
    result["newBooster"] = partial(newBooster, result)
    result["newRepair"] = partial(newRepair, result)
    result["mountInfo1"] = partial(mountInfo1, result)
    result["mountInfo2"] = partial(mountInfo2, result)

    return result

def newComputer(context, mountId, type, zoom, scanRange, leadIndicator, targetLabels, targetClosest, autoTarget):
    computer = {
        "internalMountId": mountId,
        "componentType": type,
        "zoom": zoom,
        "scanRange": scanRange,
        "leadIndicator": leadIndicator,
        "targetLabels": targetLabels,
        "targetClosest": targetClosest,
        "autoTarget": autoTarget
    }
    context["currentMount"] = computer
    context["internalMounts"].append(computer)
    

def newECM(context, mountId, type, ecmRating, chargeRate, jammingDistance, jammingChance):
    ecm = {
        "internalMountId": mountId,
        "componentType": type,
        "ecmRating": ecmRating,
        "chargeRate": chargeRate,
        "jammingDistance": jammingDistance,
        "jammingChance": jammingChance
    }
    context["currentMount"] = ecm
    context["internalMounts"].append(ecm)

def newThermal(context, mountId, type, rating, chargeRate, jammingDistance, jammingChance):
    thermal = {
        "internalMountId": mountId,
        "componentType": type,
        "rating": rating,
        "chargeRate": chargeRate,
        "jammingDistance": jammingDistance,
        "jammingChance": jammingChance
    }
    context["currentMount"] = thermal
    context["internalMounts"].append(thermal)

def newCloak(context, mountId, rating, dmgAmountGlitch, glitchCoefficient, dmgAmountFail, failCoefficient):
    clock = {
        "internalMountId": mountId,
        "componentType": "cloak",
        "rating": rating,
        "dmgAmountGlitch": dmgAmountGlitch,
        "glitchCoefficient": glitchCoefficient,
        "dmgAmountFail": dmgAmountFail,
        "failCoefficient": failCoefficient
    }
    context["currentMount"] = clock
    context["internalMounts"].append(clock)

def newModulator(context, mountId, type, focusBoost):
    computer = {
        "internalMountId": mountId,
        "componentType": type,
        "focusBoost": focusBoost
    }
    context["currentMount"] = computer
    context["internalMounts"].append(computer)

def newCapacitor(context, mountId, type, capacity, chargeRate, selfDamage):
    capacitor = {
        "internalMountId": mountId,
        "componentType": type,
        "capacity": capacity,
        "chargeRate": chargeRate,
        "selfDamage": selfDamage
    }
    context["currentMount"] = capacitor
    context["internalMounts"].append(capacitor)

def newAmplifier(context, mountId, type, multiplier):
    amplifier = {
        "internalMountId": mountId,
        "componentType": type,
        "multiplier": multiplier
    }
    context["currentMount"] = amplifier
    context["internalMounts"].append(amplifier)

def newMountable(context, mountId, type):
    mountable = {
        "internalMountId": mountId,
        "componentType": type
    }
    context["currentMount"] = mountable
    context["internalMounts"].append(mountable)

def newBattery(context, mountId, type, capacity):
    battery = {
        "internalMountId": mountId,
        "componentType": type,
        "capacity": capacity
    }
    context["currentMount"] = battery
    context["internalMounts"].append(battery)

def newBooster(context, mountId, type, boostRatio, energyCapacity, burnRate, chargeRate):
    computer = {
        "internalMountId": mountId,
        "componentType": type,
        "boostRatio": boostRatio,
        "energyCapacity": energyCapacity,
        "burnRate": burnRate,
        "chargeRate": chargeRate
    }
    context["currentMount"] = computer
    context["internalMounts"].append(computer)

def newRepair(context, mountId, type, repairRate, energyDrain):
    repair = {
        "internalMountId": mountId,
        "componentType": type,
        "repairRate": repairRate,
        "energyDrain": energyDrain
    }
    context["currentMount"] = repair
    context["internalMounts"].append(repair)

def mountInfo1(context, shortNameTag, longNameTag, smallBmp, smallDisBmp, largeBmp, largeDisBmp, description):
    context["currentMount"]["displayInfo"] = sharedInfo.createGenericInfo(shortNameTag, longNameTag, smallBmp, smallDisBmp, largeBmp, largeDisBmp, description)

def mountInfo2(context, techLevel, techBase, combatValue, mass, mountPoint, sizeDamage):
    context["currentMount"]["techInfo"] = {
        "techLevel": techLevel,
        "techBase": techBase,
        "combatValue": combatValue,
        "mass": mass,
        "mountPoint": mountPoint,
        "sizeDamage": sizeDamage
    }