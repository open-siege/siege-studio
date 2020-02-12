import sharedInfo
from functools import partial

def createExecContext():
    result = {
        "armors": [],
        "currentArmor": {},
        "Armor": "armor",
        "A": "armor",
        "X": "xeno",
        "s": "small",
        "X": "xl",
        "x": "xl",
        "S": "small",
        "L": "large",
        "M": "medium",
        "m": "medium"
    }

    result["newArmor"] = partial(newArmor, result)
    result["armorInfo1"] = partial(armorInfo1, result)
    result["armorInfo2"] = partial(armorInfo2, result)
    result["armorInfospecial"] = partial(armorInfospecial, result)

    return result

def newArmor(context, armorId, type, conShrug, elecShrug, thrmShrug, conEff, elecEff, thrmEff, rcsMod, reallocRate, regenRate):
    armor = {
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
    context["armors"].append(armor)
    context["currentArmor"] = armor


def armorInfo1(context, shortNameTag, longNameTag, smallBmp, smallDisBmp, largeBmp, largeDisBmp, description):
    context["currentArmor"]["displayInfo"] = sharedInfo.createGenericInfo(shortNameTag, longNameTag, smallBmp, smallDisBmp, largeBmp, largeDisBmp, description)

def armorInfo2(context, techLevel, techBase, combatValue, density, mountPoint, sizeDamage):
    context["currentArmor"]["techInfo"] = {
        "techLevel": techLevel,
        "techBase": techBase,
        "combatValue": combatValue,
        "density": density,
        "mountPoint": mountPoint,
        "sizeDamage": sizeDamage
    }

def armorInfospecial(context, projectileId, shrug, effective):
    context["currentArmor"]["specialOverrides"].append({
        "projectileId": projectileId,
        "shrug": shrug,
        "effective": effective
    })