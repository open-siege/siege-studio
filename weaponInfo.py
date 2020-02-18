import parseStrings
from functools import partial

def createExecContext():
    sfxStrings = parseStrings.parseStringsFromFile("sfx.strings.cs")
    result = {
        "weapons": [],
        "currentWeapon": {},
        "A": "all",
        "H": "human",
        "C": "cybrid",
        "X": "xl",
        "x": "xl",
        "S": "small",
        "L": "large",
        "M": "medium",
        "I": "internal",
        "internal": "internal",
        "T": True,
        "F": False
    }

    for key in sfxStrings:
        result[key] = sfxStrings[key]

    result["newWeapon"] = partial(newWeapon, result)
    result["weaponInfo1"] = partial(weaponInfo1, result)
    result["weaponInfo2"] = partial(weaponInfo2, result)
    result["weaponMuzzle"] = partial(weaponMuzzle, result)
    result["weaponGeneral"] = partial(weaponGeneral, result)
    result["weaponShot"] = partial(weaponShot, result)
    result["weaponEnergy"] = partial(weaponEnergy, result)
    result["weaponAmmo"] = partial(weaponAmmo, result)

    return result

def newWeapon(context, weaponId, shape, size, soundTag, damage, techBase, descriptionTag = None):
    if techBase == context["X"]:
        techBase = "xeno"
    weapon = {
        "weaponId": weaponId,
        "shape": shape,
        "size": size,
        "soundTag": soundTag,
        "displayInfo": {
            "description": descriptionTag
        },
        "techInfo": {
            "techBase": techBase,
            "sizeDamage": damage
        },
        "muzzle": None,
        "general": None,
        "shots": [],
        "energy": None,
        "ammo": None
    }
    context["currentWeapon"] = weapon
    context["weapons"].append(weapon)

def weaponInfo1(context, shortNameTag, longNameTag, smallBmp, smallDisabledBmp, largeBmp, largeDisabledBmp):
    context["currentWeapon"]["displayInfo"]["shortName"] = shortNameTag
    context["currentWeapon"]["displayInfo"]["longName"] = longNameTag
    context["currentWeapon"]["displayInfo"]["smallBmp"] = smallBmp
    context["currentWeapon"]["displayInfo"]["smallDisBmp"] = smallDisabledBmp
    context["currentWeapon"]["displayInfo"]["largeBmp"] = largeBmp
    context["currentWeapon"]["displayInfo"]["largeDisBmp"] = largeDisabledBmp

def weaponInfo2(context, techLevel, combatValue, mass):
    context["currentWeapon"]["techInfo"]["techLevel"] = techLevel
    context["currentWeapon"]["techInfo"]["mass"] = mass
    context["currentWeapon"]["techInfo"]["combatValue"] = combatValue

def weaponMuzzle(context, muzzleShape, transMuzzleShape, faceCamera, flashColorR, flashColorG, flashColorB, flashRange):
    context["currentWeapon"]["muzzle"] = {
        "muzzleShape": muzzleShape,
        "transMuzzleShape": transMuzzleShape,
        "faceCamera": faceCamera,
        "flashColor": [flashColorR, flashColorG, flashColorB],
        "flashRange": flashRange
    }

def weaponGeneral(context, reloadAnimTime, lockTime, converge):
    context["currentWeapon"]["general"] = {
        "reloadAnimTime": reloadAnimTime,
        "lockTime": lockTime,
        "converge": converge
    }

def weaponShot(context, fireOffsetX, fireOffsetY, fireOffsetZ, fireTime = None):
    context["currentWeapon"]["shots"].append({
        "fireOffset": [fireOffsetX, fireOffsetY, fireOffsetZ],
        "fireTime": fireTime
    })

def weaponEnergy(context, projectileId, chargeLimit, chargeRate):
    context["currentWeapon"]["energy"] = {
        "projectileId": projectileId,
        "chargeLimit": chargeLimit,
        "chargeRate": chargeRate
    }

def weaponAmmo(context, projectileId, maxAmmo, startAmmo, roundsPerVolley):
    context["currentWeapon"]["energy"] = {
        "projectileId": projectileId,
        "maxAmmo": maxAmmo,
        "startAmmo": startAmmo,
        "roundsPerVolley": roundsPerVolley
    }