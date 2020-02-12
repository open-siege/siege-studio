
def createExecContext():
    result = {
        "hercs": []
        "currentHerc": {}
    }

    return result

def hercBase(context, identityTag, abbreviation, shape, mass, maxMass, radarCrossSection, techLevel, combatValue):
    herc = {
        "identityTag": identityTag,
        "abbreviation": abbreviation,
        "shape": shape,
        "mass": mass,
        "maxMass": maxMass,
        "radarCrossSection": radarCrossSection,
        "techLevel": techLevel,
        "combatValue": combatValue
    }
    context["currentHerc"] = herc
    context["hercs"].append(herc)

def hercPos(maxPosAcc, minPosVel, maxForPosVel, maxRevPosVel):
    