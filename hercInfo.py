
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

def hercPos(context, maxPosAcc, minPosVel, maxForPosVel, maxRevPosVel):
    context["currentHerc"]["pos"] = {
        "maxPosAcc": maxPosAcc,
        "minPosVel": minPosVel,
        "maxForPosVel": maxForPosVel,
        "maxRevPosVel": maxRevPosVel
    }

def hercRot(context,  minRotVel,    maxRVSlow,        maxRVFast):
    context["currentHerc"]["rot"] = {
        "maxPosAcc": maxPosAcc,
        "minPosVel": minPosVel,
        "maxForPosVel": maxForPosVel,
        "maxRevPosVel": maxRevPosVel
    }

def hercAnim(context, toStandVel, toRunVel, toFastRunVel, toFastTurnVel):
    context["currentHerc"]["anim"] = {
        "maxPosAcc": maxPosAcc,
        "minPosVel": minPosVel,
        "maxForPosVel": maxForPosVel,
        "maxRevPosVel": maxRevPosVel
    }

def hercCpit(context, offsetX, offsetY, offsetZ):
    context["currentHerc"]["anim"] = {
        "maxPosAcc": maxPosAcc,
        "minPosVel": minPosVel,
        "maxForPosVel": maxForPosVel,
        "maxRevPosVel": maxRevPosVel
    }

def hercColl(context, sphOffstX, sphOffstY, sphOffstZ, sphereRad):
    context["currentHerc"]["coll"] = {
        "maxPosAcc": maxPosAcc,
        "minPosVel": minPosVel,
        "maxForPosVel": maxForPosVel,
        "maxRevPosVel": maxRevPosVel
    }

def hercAI(context, aiName1, aiName2, aiName3, aiName4):
    context["currentHerc"]["ai"] = {
        "maxPosAcc": maxPosAcc,
        "minPosVel": minPosVel,
        "maxForPosVel": maxForPosVel,
        "maxRevPosVel": maxRevPosVel
    }

def hercAI(context, aiName1, aiName2, aiName3, aiName4):
    context["currentHerc"]["ai"] = {
        "maxPosAcc": maxPosAcc,
        "minPosVel": minPosVel,
        "maxForPosVel": maxForPosVel,
        "maxRevPosVel": maxRevPosVel
    }

def newHardPoint(context, hardpointId, size, dmgParent, allowedMountables):
    context["currentHerc"]["hardpoints"].append({
        "maxPosAcc": maxPosAcc,
        "minPosVel": minPosVel,
        "maxForPosVel": maxForPosVel,
        "maxRevPosVel": maxRevPosVel
    })