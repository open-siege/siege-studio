from functools import partial
import parseFiles
import intMountsInfo

def createExecContext():
    contextToOverwrite = intMountsInfo.createExecContext()
    sfxStrings = parseFiles.parseStringsFromFile("sfx.strings.cs")
    result = {
        "vehicles": [],
        "H": "human",
        "C": "cybrid",
        "X": "xl",
        "x": "xl",
        "S": "small",
        "L": "large",
        "M": "medium",
        "T": "top",
        "B": "bottom",
        "L": "left",
        "R": "right",
        "I": "internal",
        "LeftPod": "leftPod",
        "Left_WingA": "leftWingA",
        "Right_WingA": "rightWingA",
        "tankLeftPod": "tankLeftPod",
        "tankRightPod": "tankRightPod",
        "TankLeftPod": "tankLeftPod",
        "TankRightPod": "tankRightPod",
        "Bumper": "bumper",
        "RightPod": "rightPod",
        "TopPod": "topPod",
        "TopPodA": "topPodA",
        "TopPodB": "topPodB",
        "LeftServos": "leftServos",
        "RightServos": "rightServos",
        "Pelvis": "pelvis",
        "none": None,
        "LeftWing1": "leftWing1",
        "RightWing1": "RightWing1",
        "LeftWing2": "leftWing2",
        "RightWing2": "RightWing2",
        "LeftEngine1": "leftEngine1",
        "RightEngine1": "rightEngine1",
        "LeftEngine2": "leftEngine2",
        "RightEngine2": "rightEngine2",
        "FlyerLeftFoot1": "flyerLeftFoot1",
        "FlyerLeftFoot2": "flyerLeftFoot2",
        "FlyerRightFoot1": "flyerRightFoot1",
        "FlyerRightFoot2": "flyerRightFoot2",
        "Tail": "tail",
        "LeftLeg": "leftLeg",
        "RightLeg": "rightLeg",
        "LeftLegA": "leftLegA",
        "RightLegA": "rightLegA",
        "LeftLegB": "leftLegB",
        "RightLegB": "rightLegB",
        "LeftLegC": "leftLegC",
        "RightLegC": "rightLegC",
        "LeftCalf": "leftCalf",
        "RightCalf": "rightCalf",
        "LeftFoot": "leftFoot",
        "RightFoot": "rightFoot",
        "LeftThigh": "leftThigh",
        "RighThigh": "rightThigh",
        "RightThigh": "rightThigh",
        "TankHead": "tankHead",
        "FlyerHead": "flyerHead",
        "Body": "body",
        "FlyerBody": "flyerBody",
        "LeftTread": "leftTread",
        "RightTread": "rightTread",
        "LeftTreadA": "leftTreadA",
        "RightTreadA": "rightTreadA",
        "LeftTreadB": "leftTreadB",
        "RightTreadB": "rightTreadB",
        "thermalDiffuser": "thermalDiffuser",
        "RearTread": "readTread",
        "CenterTread": "centerTread",
        "Pilot": "pilot",
        "Engine": "engine",
        "Armor": "armor",
        "Reactor": "reactor",
        "Head": "head",
        "Computer": "computer",
        "ElectroHull": "electroHull",
        "Shield": "shield",
        "Sensors": "sensor",
        "true": True,
        "false": False,
        "TRUE": True,
        "FALSE": False,
        "vehicleIsPilotable": True,
        "vehicleIsArtillery": False,
        "BASL": "BASL",
        "PROM": "PROM",
        "RBARTL": "RBARTL",
        "TRNIKE": "TRNIKE",
        "TRUPSR": "TRUPSR"
    }

    result["hercBase"] = partial(hercBase, contextToOverwrite)
    result["tankBase"] = partial(hercBase, contextToOverwrite)
    result["flyerBase"] = partial(hercBase, contextToOverwrite)
    result["droneBase"] = partial(hercBase, contextToOverwrite)

    result["hercPos"] = partial(hercPos, contextToOverwrite)
    result["tankPos"] = partial(hercPos, contextToOverwrite)
    result["dronePos"] = partial(hercPos, contextToOverwrite)
    result["flyerPos"] = partial(flyerPos, contextToOverwrite)

    result["hercRot"] = partial(hercRot, contextToOverwrite)
    result["tankRot"] = partial(hercRot, contextToOverwrite)
    result["droneRot"] = partial(hercRot, contextToOverwrite)
    result["flyerRot"] = partial(flyerRot, contextToOverwrite)

    result["hercAnim"] = partial(hercAnim, contextToOverwrite)
    result["tankAnim"] = partial(tankAnim, contextToOverwrite)
    result["droneAnim"] = partial(tankAnim, contextToOverwrite)
    result["hercAnim"] = partial(hercAnim, contextToOverwrite)

    result["hercCpit"] = partial(hercCpit, contextToOverwrite)
    result["tankCpit"] = partial(hercCpit, contextToOverwrite)
    result["flyerCpit"] = partial(hercCpit, contextToOverwrite)

    result["hercColl"] = partial(hercColl, contextToOverwrite)
    result["droneColl"] = partial(hercColl, contextToOverwrite)
    result["tankColl"] = partial(hercColl, contextToOverwrite)
    result["flyerColl"] = partial(hercColl, contextToOverwrite)

    result["hercAI"] = partial(hercAI, contextToOverwrite)
    result["tankAI"] = partial(hercAI, contextToOverwrite)
    result["flyerAI"] = partial(hercAI, contextToOverwrite)

    result["flyerExhaust"] = partial(flyerExhaust, contextToOverwrite)
    result["flyerExhaustOffset"] = partial(flyerExhaustOffset, contextToOverwrite)
    result["flyerNav"] = partial(flyerNav, contextToOverwrite)
    result["flyerSound"] = partial(flyerSound, contextToOverwrite)

    result["tankSound"] = partial(tankSound, contextToOverwrite)
    result["droneSound"] = partial(tankSound, contextToOverwrite)
    result["tankSlide"] = partial(tankSlide, contextToOverwrite)

    result["newHardPoint"] = partial(newHardPoint, contextToOverwrite)
    result["newMountPoint"] = partial(newMountPoint, contextToOverwrite)
    result["newComponent"] = partial(newComponent, contextToOverwrite)
    result["newConfiguration"] = partial(newConfiguration, contextToOverwrite)
    result["defaultWeapons"] = partial(defaultWeapons, contextToOverwrite)
    result["defaultMountables"] = partial(defaultMountables, contextToOverwrite)
    result["vehiclePilotable"] = partial(vehiclePilotable, contextToOverwrite)
    result["vehicleArtillery"] = partial(vehicleArtillery, contextToOverwrite)
    result["translucentCockpit"] = partial(translucentCockpit, contextToOverwrite)
    result["hercFootprint"] = partial(hercFootprint, contextToOverwrite)
    result["HardPointDamage"] = partial(HardPointDamage, contextToOverwrite)
    result["HardPointSpecial"] = partial(HardPointSpecial, contextToOverwrite)
    result["hardPointSpecial"] = partial(HardPointSpecial, contextToOverwrite)
    result["droneExplosion"] = partial(droneExplosion, contextToOverwrite)
    result["genericDrone"] = partial(droneExplosion, contextToOverwrite)

    for key in sfxStrings:
        contextToOverwrite[key] = sfxStrings[key]

    for key in result:
        contextToOverwrite[key] = result[key]

    return contextToOverwrite

def vehiclePilotable(context, value):
    context["vehicleIsPilotable"] = value

def vehicleArtillery(context, value):
    context["vehicleIsArtillery"] = value

def hercBase(context, identityTag, abbreviation, shape, mass, maxMass, radarCrossSection, techLevel, combatValue):
    herc = {
        "vehiclePilotable": context["vehicleIsPilotable"],
        "vehicleArtillery": context["vehicleIsArtillery"],
        "identityTag": identityTag,
        "abbreviation": abbreviation,
        "shape": shape,
        "mass": mass,
        "maxMass": maxMass,
        "radarCrossSection": radarCrossSection,
        "techLevel": techLevel,
        "combatValue": combatValue,
        "hardPoints": [],
        "mountPoints": [],
        "components": [],
        "configurations": [],
        "defaultWeapons": [],
        "defaultMountables": []
    }
    context["vehicleIsPilotable"] = True
    context["vehicleIsArtillery"] = False
    context["currentVehicle"].update(herc)


def hercPos(context, maxPosAcc, minPosVel, maxForPosVel, maxRevPosVel):
    context["currentVehicle"]["pos"] = {
        "maxPosAcc": maxPosAcc,
        "minPosVel": minPosVel,
        "maxForPosVel": maxForPosVel,
        "maxRevPosVel": maxRevPosVel
    }

def flyerPos(context, maxPosAcc, thrustMultiple, maxLiftVel, maxFallVel, maxFlyVel, fastLean):
    context["currentVehicle"]["pos"] = {
        "maxPosAcc": maxPosAcc,
        "thrustMultiple": thrustMultiple,
        "maxLiftVel": maxLiftVel,
        "maxFallVel": maxFallVel,
        "maxFlyVel": maxFlyVel,
        "fastLean": fastLean
    }

def hercRot(context,minRotVel, maxRVSlow, maxRVFast, maxRVTurret = None):
    context["currentVehicle"]["rot"] = {
        "minRotVel": minRotVel,
        "maxRVSlow": maxRVSlow,
        "maxRVFast": maxRVFast,
        "maxRVTurret": maxRVTurret
    }

def flyerRot(context, maxRotXVel, maxRotYVel, maxRotZVel):
    context["currentVehicle"]["rot"] = {
        "maxRotXVel": maxRotXVel,
        "maxRotYVel": maxRotYVel,
        "maxRotZVel": maxRotZVel,
    }

def hercAnim(context, toStandVel, toRunVel, toFastRunVel, toFastTurnVel):
    context["currentVehicle"]["anim"] = {
        "toStandVel": toStandVel,
        "toRunVel": toRunVel,
        "toFastRunVel": toFastRunVel,
        "toFastTurnVel": toFastTurnVel
    }

def tankAnim(context, treadAnimRotCoefficient, treadAnimPosCooefficent):
    context["currentVehicle"]["anim"] = {
        "treadAnimRotCoefficient": treadAnimRotCoefficient,
        "treadAnimPosCooefficent": treadAnimPosCooefficent
    }

def hercCpit(context, offsetX, offsetY, offsetZ):
    context["currentVehicle"]["cpit"] = {
        "offsetX": offsetX,
        "offsetY": offsetY,
        "offsetZ": offsetZ
    }

def hercColl(context, sphOffstX, sphOffstY, sphOffstZ, sphereRad):
    context["currentVehicle"]["coll"] = {
        "sphOffstX": sphOffstX,
        "sphOffstY": sphOffstY,
        "sphOffstZ": sphOffstZ,
        "sphereRad": sphereRad
    }

def hercAI(context, aiName1 = None, aiName2 = None, aiName3 = None, aiName4 = None):
    context["currentVehicle"]["ai"] = {
        "aiName1": aiName1,
        "aiName2": aiName2,
        "aiName3": aiName3,
        "aiName4": aiName4
    }

def hercFootprint(context, footprintType):
    context["currentVehicle"]["footprintType"] = footprintType

def tankSound(context, engineSoundTag, hasThrusters):
    context["currentVehicle"]["sound"] = {
        "engineSoundTag": engineSoundTag,
        "hasThrusters": hasThrusters
    }

def flyerSound(context, startupTag, shutdownTag, flyTag, damagedFlyTag):
    context["currentVehicle"]["sound"] = {
        "startupTag": startupTag,
        "shutdownTag": shutdownTag,
        "flyTag": flyTag,
        "damagedFlyTag": damagedFlyTag
    }

def tankSlide(context, slideCoefficient):
    context["currentVehicle"]["slide"] = {
        "slideCoefficient": slideCoefficient
    }

def flyerExhaust(context, exhaustShapeT, exhaustShapeNT, numberOfSources):
    context["currentVehicle"]["exhaust"] = {
        "exhaustShapeT": exhaustShapeT,
        "exhaustShapeNT": exhaustShapeNT,
        "numberOfSources": numberOfSources,
        "offsets": []
    }

def flyerExhaustOffset(context, *values):
    context["currentVehicle"]["exhaust"]["offsets"].append(values)

def flyerNav(context, maxLean, maxBank, taxiRange, shortRange, mediumRange):
    context["currentVehicle"]["nav"] = {
        "maxLean": maxLean,
        "maxBank": maxBank,
        "taxiRange": taxiRange,
        "shortRange": shortRange,
        "mediumRange": mediumRange
    }

def droneExplosion(context, *values):
    context["currentVehicle"]["droneExplosion"] = values

def translucentCockpit(context):
    context["currentVehicle"]["cpit"]["translucent"] = True

def newHardPoint(context, hardpointId, size, side, dmgParent, offsetFromNodeX, offsetFromNodeY, offsetFromNodeZ, xRotationRangeMin, xRotationRangeMax, zRotationRangeMin, zRotationRangeMax):
    context["currentVehicle"]["hardPoints"].append({
        "hardpointId": hardpointId,
        "size": size,
        "side": side,
        "dmgParent": dmgParent,
        "offsetFromNode": [offsetFromNodeX, offsetFromNodeY, offsetFromNodeZ],
        "xRotationRange": [xRotationRangeMin, xRotationRangeMax],
        "zRotationRange": [zRotationRangeMin, zRotationRangeMax]
    })

def HardPointDamage(context, damageValue):
    context["currentVehicle"]["hardPoints"][-1]["damage"] = damageValue

def HardPointSpecial(context, specialValue):
    context["currentVehicle"]["hardPoints"][-1]["special"] = specialValue

def newMountPoint(context, mountPointId, size, dmgParent, *allowedMountables):
    mountPoint = {
        "mountPointId": mountPointId,
        "size": size,
        "dmgParent": dmgParent,
        "allowedMountables": []
    }

    for mountable in allowedMountables:
        mountPoint["allowedMountables"].append(mountable)
    context["currentVehicle"]["mountPoints"].append(mountPoint)


def newComponent(context, componentId, componentType, parent, maxDamage, identityTag):
    context["currentVehicle"]["components"].append({
        "componentId": componentId,
        "componentType": componentType,
        "parent": parent,
        "maxDamage": maxDamage,
        "identityTag": identityTag
    })

def newConfiguration(context, containee, containter, internalPercentage):
    context["currentVehicle"]["configurations"].append({
        "containee": containee,
        "container": containter,
        "internalPercentage": internalPercentage
    })

def defaultWeapons(context, *weapons):
    for weapon in weapons:
        context["currentVehicle"]["defaultWeapons"].append(weapon)

def defaultMountables(context, *mountables):
    for mountable in mountables:
        context["currentVehicle"]["defaultMountables"].append(mountable)

def genericDrone(context, cargoCount):
    context["currentVehicle"]["cargoCount"] = cargoCount