import parseStrings
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
        "SPR": "SPR"
    }

    globalStrings = parseStrings.parseStringsFromFile("Sim.Strings.cs")

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
              specialPercentage, passthroughShield, passthroughArmor, effectVsShield, effectVsAmor, shape, transparentShape, impactId, shieldImpactId, terrainImpactId = None):
    context["projectiles"].append({
        "projectileId": projectileId,
        "projectileType": "bullet"
    })

def newMissile(context, abbr, projectileId, startVelocity, endVelocity, acceleration, range, minRange, psuedoMass,
               glowRange, glowColorR, glowColorG, glowColorB, damaga, blast, concussionPercentage, elecPercentage, thermalPercentage,
              specialPercentage, passthroughShield, passthroughArmor,
               effectVsShield, effectVsAmor, tackType, turnRate1, rate1Timeout, turnRate2, cruising, cruiseHugTerrain,
               cruiseEnvelope, burnFuelWhileUnarmed, explodeOnMiss, shape, transparentShape, impactId, shieldImpactId, terrainImpactId = None):
    context["projectiles"].append({
        "projectileId": projectileId,
        "projectileType": "missile"
    })

def NewEnergy(context, abbr, projectileId, velocity, range, psuedoMass, glowRange, glowColorR, glowColorG, glowColorB,
            damage, blast, concussionPercentage, elecPercentage, thermalPercentage,
              specialPercentage, passthroughShield, passthroughArmor, effectVsShield, effectVsAmor, faceCamera, shape, transparentShape, impactId, shieldImpactId, terrainImpactId = None):
    context["projectiles"].append({
        "projectileId": projectileId,
        "projectileType": "energy"
    })

def NewBeam(context, abbr, projectileId, velocity, range, beamLength, segLength, targetTrack, psuedoMass, glowRange, glowColorR, glowColorG, glowColorB,
                damage, blast, concussionPercentage, elecPercentage, thermalPercentage,
              specialPercentage, passthroughShield, passthroughArmor, effectVsShield, effectVsAmor, faceCamera, shape, transparentShape, impactId, shieldImpactId, terrainImpactI):
    context["projectiles"].append({
        "projectileId": projectileId,
        "projectileType": "beam"
    })

def newMine(context, abbr, projectileId, velocity, range, psuedoMass, glowRange, glowColorR, glowColorG, glowColorB, damage, blast, concussionPercentage, elecPercentage, thermalPercentage,
              specialPercentage, passthroughShield, passthroughArmor, effectVsShield, effectVsAmor, shape, transparentShape, impactId, shieldImpactId, terrainImpactI):
    context["projectiles"].append({
        "projectileId": projectileId,
        "projectileType": "mine"
    })

def newBomb(context, abbr, projectileId, velocity, range, psuedoMass, glowRange, glowColor, damage, blast, concussionPercentage, elecPercentage, thermalPercentage,
              specialPercentage, passthroughShield, passthroughArmor, effectVsShield, effectVsAmor, shape, transparentShape, impactId, shieldImpactId, terrainImpactI):
    context["projectiles"].append({
        "projectileId": projectileId,
        "projectileType": "bomb"
    })