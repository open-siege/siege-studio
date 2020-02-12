import sharedEquipment
from functools import partial

def createExecContext():
    result = {
        "newReactor": newReactor,
        "reactors": {},
        "finalReactors": [],
        "currentReactorId": {},
        "H": "human",
        "C": "cybrid",
        "X": "xl",
        "x": "xl",
        "m": "medium",
        "S": "small",
        "s": "small",
        "L": "large",
        "M": "medium"
    }

    result["newReactor"] = partial(newReactor, result)
    result["reactorInfo1"] = partial(reactorInfo1, result)
    result["reactorInfo2"] = partial(reactorInfo2, result)

    return result


def newReactor(context, reactorId, output, battery, meltdown):
    reactors = context["reactors"]
    finalReactors = context["finalReactors"]
    currentReactorId = context["currentReactorId"]
    if reactorId not in reactors:
        currentReactorId["current"] = reactorId
        reactors[reactorId] = {
            "reactorId": reactorId,
            "output": output,
            "battery": battery,
            "meltdown": meltdown
        }
        finalReactors.append(reactors[reactorId])


def reactorInfo1(context, shortNameTag, longNameTag, smallBmp, smallDisBmp, largeBmp, largeDisBmp, description):
    reactors = context["reactors"]
    currentReactorId = context["currentReactorId"]
    reactors[currentReactorId["current"]]["displayInfo"] = sharedEquipment.createGenericInfo(shortNameTag, longNameTag, smallBmp, smallDisBmp, largeBmp, largeDisBmp, description)

def reactorInfo2(context, techLevel, techBase, combatValue, mass, mountPoint, sizeDamage):
    reactors = context["reactors"]
    currentReactorId = context["currentReactorId"]
    reactors[currentReactorId["current"]]["techInfo"] = {
        "techLevel": techLevel,
        "techBase": techBase,
        "combatValue": combatValue,
        "mass": mass,
        "mountPoint": mountPoint,
        "sizeDamage": sizeDamage
    }
