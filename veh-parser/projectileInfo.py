import parseFiles
from functools import partial

def createExecContext():
    result = {
        "projectiles": [],
        "F": False,
        "T": True,
        "H": "heat",
        "R": "radar",
        "none": None,
        "ART": "art",
        "VIP": "vip",
        "SPR": "spr",
        "ARAC": "arac",
        "PROX": "prox"
    }

    globalStrings = parseFiles.parseStringsFromFile("Sim.Strings.cs")

    for key in globalStrings:
        if key.startswith("IDWEA_"):
            newKey: str = key.replace("IDWEA_", "")
            result[newKey] = newKey.lower()

    result["newBullet"] = partial(newBullet, result)
    result["newMissile"] = partial(newMissile, result)
    result["NewEnergy"] = partial(NewEnergy, result)
    result["NewBeam"] = partial(NewBeam, result)
    result["newMine"] = partial(newMine, result)
    result["newBomb"] = partial(newBomb, result)

    return result

def newBullet(context, abbr, projectileId, velocity, range, psuedoMass, glowRange, glowColorR, glowColorG, glowColorB, damage, blast, concussionPercentage, elecPercentage, thermalPercentage,
              specialPercentage, passthroughShield, passthroughArmor, effectVsShield, effectVsArmor, shape, transparentShape, impactId, shieldImpactId, terrainImpactId = None):
    context["projectiles"].append({
        "projectileId": projectileId,
        "projectileType": "bullet",
        "abbreviation": abbr,
        "velocity": velocity,
        "range": range,
        "psuedoMass": psuedoMass,
        "glowRange": glowRange,
        "glowColor": [glowColorR, glowColorG, glowColorB],
        "damage": damage,
        "blastRadius": blast,
        "concussionPercentage": concussionPercentage,
        "elecPercentage": elecPercentage,
        "thermalPercentage": thermalPercentage,
        "specialPercentage": specialPercentage,
        "passthroughShield": passthroughShield,
        "passthroughArmor": passthroughArmor,
        "effectVsShield": effectVsShield,
        "effectVsArmor": effectVsArmor,
        "shape": shape,
        "transparentShape": transparentShape,
        "impactId": impactId,
        "shieldImpactId": shieldImpactId,
        "terrainImpactId": terrainImpactId
    })

def newMissile(context, abbr, projectileId, startVelocity, endVelocity, acceleration, range, minRange, psuedoMass,
               glowRange, glowColorR, glowColorG, glowColorB, damage, blast, concussionPercentage, elecPercentage, thermalPercentage,
              specialPercentage, passthroughShield, passthroughArmor,
               effectVsShield, effectVsAmor, tackType, turnRate1, rate1Timeout, turnRate2, cruising, cruiseHugTerrain,
               cruiseEnvelope, burnFuelWhileUnarmed, explodeOnMiss, shape, transparentShape, impactId, shieldImpactId, terrainImpactId = None):
    context["projectiles"].append({
        "projectileId": projectileId,
        "projectileType": "missile",
        "abbreviation": abbr,
        "startVelocity": startVelocity,
        "endVelocity": endVelocity,
        "range": range,
        "psuedoMass": psuedoMass,
        "glowRange": glowRange,
        "glowColor": [glowColorR, glowColorG, glowColorB],
        "damage": damage,
        "blastRadius": blast,
        "concussionPercentage": concussionPercentage,
        "elecPercentage": elecPercentage,
        "thermalPercentage": thermalPercentage,
        "specialPercentage": specialPercentage,
        "passthroughShield": passthroughShield,
        "passthroughArmor": passthroughArmor,
        "effectVsShield": effectVsShield,
        "shape": shape,
        "transparentShape": transparentShape,
        "impactId": impactId,
        "shieldImpactId": shieldImpactId,
        "terrainImpactId": terrainImpactId
    })

def NewEnergy(context, abbr, projectileId, velocity, range, psuedoMass, glowRange, glowColorR, glowColorG, glowColorB,
            damage, blast, concussionPercentage, elecPercentage, thermalPercentage,
              specialPercentage, passthroughShield, passthroughArmor, effectVsShield, effectVsAmror, faceCamera, shape, transparentShape, impactId, shieldImpactId, terrainImpactId = None):
    context["projectiles"].append({
        "projectileId": projectileId,
        "projectileType": "energy",
        "abbreviation": abbr,
        "velocity": velocity,
        "range": range,
        "psuedoMass": psuedoMass,
        "glowRange": glowRange,
        "glowColor": [glowColorR, glowColorG, glowColorB],
        "damage": damage,
        "blastRadius": blast,
        "concussionPercentage": concussionPercentage,
        "elecPercentage": elecPercentage,
        "thermalPercentage": thermalPercentage,
        "specialPercentage": specialPercentage,
        "passthroughShield": passthroughShield,
        "passthroughArmor": passthroughArmor,
        "effectVsShield": effectVsShield,
        "effectVsAmror": effectVsAmror,
        "faceCamera": faceCamera,
        "shape": shape,
        "transparentShape": transparentShape,
        "impactId": impactId,
        "shieldImpactId": shieldImpactId,
        "terrainImpactId": terrainImpactId
    })

def NewBeam(context, abbr, projectileId, velocity, range, beamLength, segLength, targetTrack, turn, jitterVelocity, psuedoMass, damage,
            varPercentage, blast, concussionPercentage, elecPercentage, thermalPercentage,
              specialPercentage, passthroughShield, passthroughArmor, effectVsShield, effectVsAmor, faceCamera, nearWidth, farWidth,
            bitmap, transparentBitmap, glowColorR, glowColorG, glowColorB, glowColorAlpha, impactId, shieldImpactId, terrainImpactId = None):
    context["projectiles"].append({
        "projectileId": projectileId,
        "projectileType": "beam",
        "abbreviation": abbr,
        "velocity": velocity,
        "range": range,
        "psuedoMass": psuedoMass,
        "glowColor": [glowColorR, glowColorG, glowColorB],
        "damage": damage,
        "blastRadius": blast,
        "concussionPercentage": concussionPercentage,
        "elecPercentage": elecPercentage,
        "thermalPercentage": thermalPercentage,
        "specialPercentage": specialPercentage,
        "passthroughShield": passthroughShield,
        "passthroughArmor": passthroughArmor,
        "effectVsShield": effectVsShield,
        "bitmap": bitmap,
        "transparentBitmap": transparentBitmap,
        "impactId": impactId,
        "shieldImpactId": shieldImpactId,
        "terrainImpactId": terrainImpactId
    })

def newMine(context, abbr, projectileId, startVelocity, endVelocity, acceleration, duration, armingDelay, psuedoMass, proximity, damage, blast, concussionPercentage, elecPercentage, thermalPercentage,
              specialPercentage, passthroughShield, passthroughArmor, effectVsShield, effectVsAmor,
            turnRate, cruiseEnvelope, shape, transparentShape, impactId, shieldImpactId):
    context["projectiles"].append({
        "projectileId": projectileId,
        "projectileType": "mine",
        "abbreviation": abbr,
        "startVelocity": startVelocity,
        "endVelocity": endVelocity,
        "psuedoMass": psuedoMass,
        "damage": damage,
        "blastRadius": blast,
        "concussionPercentage": concussionPercentage,
        "elecPercentage": elecPercentage,
        "thermalPercentage": thermalPercentage,
        "specialPercentage": specialPercentage,
        "passthroughShield": passthroughShield,
        "passthroughArmor": passthroughArmor,
        "effectVsShield": effectVsShield,
        "shape": shape,
        "transparentShape": transparentShape,
        "impactId": impactId,
        "shieldImpactId": shieldImpactId
    })

def newBomb(context, abbr, projectileId, endVelocity, psuedoMass, damage, blast, concussionPercentage, elecPercentage, thermalPercentage,
              specialPercentage, passthroughShield, passthroughArmor, effectVsShield, effectVsArmor, turnRate, shape, transparentShape, impactId, shieldImpactId):
    context["projectiles"].append({
        "projectileId": projectileId,
        "projectileType": "bomb",
        "abbreviation": abbr,
        "endVelocity": endVelocity,
        "psuedoMass": psuedoMass,
        "damage": damage,
        "blastRadius": blast,
        "concussionPercentage": concussionPercentage,
        "elecPercentage": elecPercentage,
        "thermalPercentage": thermalPercentage,
        "specialPercentage": specialPercentage,
        "passthroughShield": passthroughShield,
        "passthroughArmor": passthroughArmor,
        "effectVsShield": effectVsShield,
        "shape": shape,
        "transparentShape": transparentShape,
        "impactId": impactId,
        "shieldImpactId": shieldImpactId
    })