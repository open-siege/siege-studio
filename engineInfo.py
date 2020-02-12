import sharedEquipment
from functools import partial

def createExecContext():
    result = {
        "H": "human",
        "C": "cybrid",
        "X": "xl",
        "x": "xl",
        "S": "small",
        "L": "large",
        "M": "medium",
        "engines": {},
        "finalEngines": [],
        "currentEngineId": {}
    }

    result["newEngine"] = partial(newEngine, result)
    result["engineInfo1"] = partial(engineInfo1, result)
    result["engineInfo2"] = partial(engineInfo2, result)

    return result

def newEngine(context, engineId, velocityRating, accelerationRating, accelerationAtMaxVelocity):
    engines = context["engines"]
    currentEngineId = context["currentEngineId"]
    finalEngines = context["finalEngines"]
    currentEngineId["current"] = engineId
    if engineId not in engines:
        engines[engineId] = {
            "engineId": engineId,
            "velocityRating": velocityRating,
            "accelerationRating": accelerationRating,
            "accelerationAtMaxVelocity": accelerationAtMaxVelocity
        }
        finalEngines.append(engines[engineId])


def engineInfo1(context, shortNameTag, longNameTag, smallBmp, smallDisBmp, largeBmp, largeDisBmp, description):
    engines = context["engines"]
    currentEngineId = context["currentEngineId"]
    engines[currentEngineId["current"]]["displayInfo"] = sharedEquipment.createGenericInfo(shortNameTag, longNameTag, smallBmp, smallDisBmp, largeBmp, largeDisBmp, description)

def engineInfo2(context, techLevel, techBase, combatValue, mass, mountPoint, sizeDamage):
    engines = context["engines"]
    currentEngineId = context["currentEngineId"]
    engines[currentEngineId["current"]]["techInfo"] = {
        "techLevel": techLevel,
        "techBase": techBase,
        "combatValue": combatValue,
        "mass": mass,
        "mountPoint": mountPoint,
        "sizeDamage": sizeDamage
    }
