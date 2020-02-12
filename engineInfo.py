import sharedInfo
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
        "engines": [],
        "currentEngine": {}
    }

    result["newEngine"] = partial(newEngine, result)
    result["engineInfo1"] = partial(engineInfo1, result)
    result["engineInfo2"] = partial(engineInfo2, result)

    return result

def newEngine(context, engineId, velocityRating, accelerationRating, accelerationAtMaxVelocity):
    engine = {
            "engineId": engineId,
            "velocityRating": velocityRating,
            "accelerationRating": accelerationRating,
            "accelerationAtMaxVelocity": accelerationAtMaxVelocity
    }
    context["currentEngine"] = engine
    context["engines"].append(engine)


def engineInfo1(context, shortNameTag, longNameTag, smallBmp, smallDisBmp, largeBmp, largeDisBmp, description):
    context["currentEngine"]["displayInfo"] = sharedInfo.createGenericInfo(shortNameTag, longNameTag, smallBmp, smallDisBmp, largeBmp, largeDisBmp, description)

def engineInfo2(context, techLevel, techBase, combatValue, mass, mountPoint, sizeDamage):
    context["currentEngine"]["techInfo"] = {
        "techLevel": techLevel,
        "techBase": techBase,
        "combatValue": combatValue,
        "mass": mass,
        "mountPoint": mountPoint,
        "sizeDamage": sizeDamage
    }
