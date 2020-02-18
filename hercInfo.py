from functools import partial
import intMountsInfo

def createExecContext():
    contextToOverwrite = intMountsInfo.createExecContext()
    result = {
        "hercs": [],
        "currentHerc": {},
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
        "RightPod": "rightPod",
        "TopPod": "topPod",
        "LeftServos": "leftServos",
        "RightServos": "rightServos",
        "Pelvis": "pelvis",
        "none": None,
        "LeftLeg": "leftLeg",
        "RightLeg": "rightLeg",
        "LeftCalf": "leftCalf",
        "RightCalf": "rightCalf",
        "LeftFoot": "leftFoot",
        "RightFoot": "rightFoot",
        "LeftThigh": "leftThigh",
        "RighThigh": "rightThigh",
        "RightThigh": "rightThigh",
        "Pilot": "pilot",
        "Engine": "engine",
        "Armor": "armor",
        "Reactor": "reactor",
        "Head": "head",
        "Computer": "computer",
        "Shield": "shield",
        "Sensors": "sensor",
        "true": True,
        "false": False,
        "vehicleIsPilotable": True,
        "BASL": "BASL",
        "PROM": "PROM"
    }
    result["hercBase"] = partial(hercBase, result)
    result["hercPos"] = partial(hercPos, result)
    result["hercRot"] = partial(hercRot, result)
    result["hercAnim"] = partial(hercAnim, result)
    result["hercCpit"] = partial(hercCpit, result)
    result["hercColl"] = partial(hercColl, result)
    result["hercAI"] = partial(hercAI, result)
    result["newHardPoint"] = partial(newHardPoint, result)
    result["newMountPoint"] = partial(newMountPoint, result)
    result["newComponent"] = partial(newComponent, result)
    result["newConfiguration"] = partial(newConfiguration, result)
    result["defaultWeapons"] = partial(defaultWeapons, result)
    result["defaultMountables"] = partial(defaultMountables, result)
    result["vehiclePilotable"] = partial(vehiclePilotable, result)
    result["translucentCockpit"] = partial(translucentCockpit, result)
    result["hercFootprint"] = partial(hercFootprint, result)
    result["HardPointDamage"] = partial(HardPointDamage, result)

    for key in result:
        contextToOverwrite[key] = result[key]

    return contextToOverwrite

def vehiclePilotable(context, value):
    context["vehicleIsPilotable"] = value

def hercBase(context, identityTag, abbreviation, shape, mass, maxMass, radarCrossSection, techLevel, combatValue):
    herc = {
        "vehiclePilotable": context["vehicleIsPilotable"],
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
    context["currentHerc"] = herc
    context["hercs"].append(herc)

def hercPos(context, maxPosAcc, minPosVel, maxForPosVel, maxRevPosVel):
    context["currentHerc"]["pos"] = {
        "maxPosAcc": maxPosAcc,
        "minPosVel": minPosVel,
        "maxForPosVel": maxForPosVel,
        "maxRevPosVel": maxRevPosVel
    }

def hercRot(context,  minRotVel,    maxRVSlow,        maxRVFast):
    context["currentHerc"]["rot"] = {
        "minRotVel": minRotVel,
        "maxRVSlow": maxRVSlow,
        "maxRVFast": maxRVFast
    }

def hercAnim(context, toStandVel, toRunVel, toFastRunVel, toFastTurnVel):
    context["currentHerc"]["anim"] = {
        "toStandVel": toStandVel,
        "toRunVel": toRunVel,
        "toFastRunVel": toFastRunVel,
        "toFastTurnVel": toFastTurnVel
    }

def hercCpit(context, offsetX, offsetY, offsetZ):
    context["currentHerc"]["cpit"] = {
        "offsetX": offsetX,
        "offsetY": offsetY,
        "offsetZ": offsetZ
    }

def hercColl(context, sphOffstX, sphOffstY, sphOffstZ, sphereRad):
    context["currentHerc"]["coll"] = {
        "sphOffstX": sphOffstX,
        "sphOffstY": sphOffstY,
        "sphOffstZ": sphOffstZ,
        "sphereRad": sphereRad
    }

def hercAI(context, aiName1, aiName2, aiName3, aiName4 = None):
    context["currentHerc"]["ai"] = {
        "aiName1": aiName1,
        "aiName2": aiName2,
        "aiName3": aiName3,
        "aiName4": aiName4
    }

def hercFootprint(context, footprintType):
    context["currentHerc"]["footprintType"] = footprintType

def translucentCockpit(context):
    context["currentHerc"]["cpit"]["translucent"] = True

def newHardPoint(context, hardpointId, size, side, dmgParent, offsetFromNodeX, offsetFromNodeY, offsetFromNodeZ, xRotationRangeMin, xRotationRangeMax, zRotationRangeMin, zRotationRangeMax):
    context["currentHerc"]["hardPoints"].append({
        "hardpointId": hardpointId,
        "size": size,
        "side": side,
        "dmgParent": dmgParent,
        "offsetFromNode": [offsetFromNodeX, offsetFromNodeY, offsetFromNodeZ],
        "xRotationRange": [xRotationRangeMin, xRotationRangeMax],
        "zRotationRange": [zRotationRangeMin, zRotationRangeMax]
    })

def HardPointDamage(context, damageValue):
    context["currentHerc"]["hardPoints"][-1]["damage"] = damageValue

def newMountPoint(context, mountPointId, size, dmgParent, *allowedMountables):
    mountPoint = {
        "mountPointId": mountPointId,
        "size": size,
        "dmgParent": dmgParent,
        "allowedMountables": []
    }

    for mountable in allowedMountables:
        mountPoint["allowedMountables"].append(mountable)
    context["currentHerc"]["mountPoints"].append(mountPoint)


def newComponent(context, componentId, componentType, parent, maxDamage, identityTag):
    context["currentHerc"]["components"].append({
        "componentId": componentId,
        "componentType": componentType,
        "parent": parent,
        "maxDamage": maxDamage,
        "identityTag": identityTag
    })

def newConfiguration(context, containee, containter, internalPercentage):
    context["currentHerc"]["configurations"].append({
        "containee": containee,
        "container": containter,
        "internalPercentage": internalPercentage
    })

def defaultWeapons(context, *weapons):
    for weapon in weapons:
        context["currentHerc"]["defaultWeapons"].append(weapon)

def defaultMountables(context, *mountables):
    for mountable in mountables:
        context["currentHerc"]["defaultMountables"].append(mountable)