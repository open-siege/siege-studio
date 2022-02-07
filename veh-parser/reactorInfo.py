import sharedInfo
from functools import partial

def createExecContext():
    result = {
        "reactors": [],
        "currentReactor": {},
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
    reactor = {
            "reactorId": reactorId,
            "output": output,
            "battery": battery,
            "meltdown": meltdown
        }
    context["currentReactor"] = reactor
    context["reactors"].append(reactor)


def reactorInfo1(context, shortNameTag, longNameTag, smallBmp, smallDisBmp, largeBmp, largeDisBmp, description):

    context["currentReactor"]["displayInfo"] = sharedInfo.createGenericInfo(shortNameTag, longNameTag, smallBmp, smallDisBmp, largeBmp, largeDisBmp, description)

def reactorInfo2(context, techLevel, techBase, combatValue, mass, mountPoint, sizeDamage):
    context["currentReactor"]["techInfo"] = {
        "techLevel": techLevel,
        "techBase": techBase,
        "combatValue": combatValue,
        "mass": mass,
        "mountPoint": mountPoint,
        "sizeDamage": sizeDamage
    }
