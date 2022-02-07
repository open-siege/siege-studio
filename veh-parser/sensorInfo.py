import sharedInfo
from functools import partial

def createExecContext():
    result = {
        "sensors": [],
        "currentSensor": {},
        "a": "active",
        "p": "passive",
        "H": "human",
        "C": "cybrid",
        "s": "small",
        "X": "xl",
        "x": "xl",
        "S": "small",
        "L": "large",
        "M": "medium",
        "m": "medium"
    }

    result["newSensor"] = partial(newSensor, result)
    result["sensorInfo1"] = partial(sensorInfo1, result)
    result["sensorInfo2"] = partial(sensorInfo2, result)
    result["sensorMode"] = partial(sensorMode, result)

    return result

def newSensor(context, sensorId, sweepTime):
    shield = {
        "sensorId": sensorId,
        "sweepTime": sweepTime,
        "modes": []
    }
    context["sensors"].append(shield)
    context["currentSensor"] = shield

def sensorMode(context, modeType, base, range, shutdown, squat, stop, slow, fast, active, camo, jam, shld, tjam):
    context["currentSensor"]["modes"].append({
        "modeType": modeType,
        "base": base,
        "range": range,
        "shutdown": shutdown,
        "squat": squat,
        "stop": stop,
        "slow": slow,
        "fast": fast,
        "active": active,
        "camo": camo,
        "jam": jam,
        "shld": shld,
        "tjam": tjam
    })

def sensorInfo1(context, shortNameTag, longNameTag, smallBmp, smallDisBmp, largeBmp, largeDisBmp, description):
    context["currentSensor"]["displayInfo"] = sharedInfo.createGenericInfo(shortNameTag, longNameTag, smallBmp,
                                                                           smallDisBmp, largeBmp, largeDisBmp,
                                                                           description)

def sensorInfo2(context, techLevel, techBase, combatValue, mass, mountPoint, sizeDamage):
    context["currentSensor"]["techInfo"] = {
        "techLevel": techLevel,
        "techBase": techBase,
        "combatValue": combatValue,
        "mass": mass,
        "mountPoint": mountPoint,
        "sizeDamage": sizeDamage
    }