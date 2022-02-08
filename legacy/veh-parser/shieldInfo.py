import sharedInfo
from functools import partial

def createExecContext():
    result = {
        "shields": [],
        "currentShield": {},
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

    result["newShield"] = partial(newShield, result)
    result["shieldInfo1"] = partial(shieldInfo1, result)
    result["shieldInfo2"] = partial(shieldInfo2, result)

    return result

def newShield(context, shieldId, chargeLimit, chargeRate, decayTime, constant):
    shield = {
        "shieldId": shieldId,
        "chargeLimit": chargeLimit,
        "chargeRate": chargeRate,
        "decayTime": decayTime,
        "constant": constant
    }
    context["shields"].append(shield)
    context["currentShield"] = shield

def shieldInfo1(context, shortNameTag, longNameTag, smallBmp, smallDisBmp, largeBmp, largeDisBmp, description):
    context["currentShield"]["displayInfo"] = sharedInfo.createGenericInfo(shortNameTag, longNameTag, smallBmp,
                                                                           smallDisBmp, largeBmp, largeDisBmp,
                                                                           description)

def shieldInfo2(context, techLevel, techBase, combatValue, mass, mountPoint, sizeDamage):
    context["currentShield"]["techInfo"] = {
        "techLevel": techLevel,
        "techBase": techBase,
        "combatValue": combatValue,
        "mass": mass,
        "mountPoint": mountPoint,
        "sizeDamage": sizeDamage
    }