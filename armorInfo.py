import sharedEquipment
from functools import partial

def createExecContext():
    result = {
        "armors": {},
        "currentArmorId": {},
        "finalArmors": []
    }

    result["newArmor"] = partial(newArmor, result)
    result["armorInfo1"] = partial(armorInfo1, result)
    result["armorInfo2"] = partial(armorInfo2, result)
    result["armorInfospecial"] = partial(armorInfospecial, result)

    return result

def newArmor(context, armorId, type, conShrug, elecShrug, thrmShrug, conEff, elecEff, thrmEff, rcsMod, reallocRate, regenRate):
    armors = context["armors"]
    currentArmorId = context["currentArmorId"]
    finalArmors = context["finalArmors"]
    if armorId not in armors:
        currentArmorId["current"] = armorId
        armors[armorId] = {
            "armorId": armorId,
            "type": type,
            "conShrug": conShrug,
            "elecShrug": elecShrug,
            "thrmShrug": thrmShrug,
            "conEff": conEff,
            "elecEff": elecEff,
            "thrmEff": thrmEff,
            "rscMod": rcsMod,
            "reallocRate": reallocRate,
            "regenRate": regenRate,
            "specialOverrides": []
        }
        finalArmors.append(armorId[armorId])


def armorInfo1(context, shortNameTag, longNameTag, smallBmp, smallDisBmp, largeBmp, largeDisBmp, description):
    armors = context["armors"]
    currentArmorId = context["currentArmorId"]
    armors[currentArmorId["current"]]["displayInfo"] = sharedEquipment.createGenericInfo(shortNameTag, longNameTag, smallBmp, smallDisBmp, largeBmp, largeDisBmp, description)

def armorInfo2(context, techLevel, techBase, combatValue, density, mountPoint, sizeDamage):
    armors = context["armors"]
    currentArmorId = context["currentArmorId"]
    armors[currentArmorId["current"]]["techInfo"] = {
        "techLevel": techLevel,
        "techBase": techBase,
        "combatValue": combatValue,
        "density": density,
        "mountPoint": mountPoint,
        "sizeDamage": sizeDamage
    }

def armorInfospecial(context, projectileId, shrug, effective):
    armors = context["armors"]
    currentArmorId = context["currentArmorId"]
    armors[currentArmorId["current"]]["specialOverrides"].append({
        "projectileId": projectileId,
        "shrug": shrug,
        "effective": effective
    })