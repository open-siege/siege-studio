import parseFiles
import engineInfo
import reactorInfo
import sensorInfo
import armorInfo
import shieldInfo
import weaponInfo
import intMountsInfo
import projectileInfo
import hercInfo
import json
from functools import partial

globalStrings = parseFiles.parseStringsFromFile("Sim.Strings.cs")

parseFiles.convertToJson("datEngine", globalStrings, engineInfo, "engines")
parseFiles.convertToJson("datReactor", globalStrings, reactorInfo, "reactors")
parseFiles.convertToJson("datSensor", globalStrings, sensorInfo, "sensors")
parseFiles.convertToJson("datArmor", globalStrings, armorInfo, "armors")
parseFiles.convertToJson("datShield", globalStrings, shieldInfo, "shields")
parseFiles.convertToJson("datWeapon", globalStrings, weaponInfo, "weapons")
parseFiles.convertToJson("datIntMounts", globalStrings, intMountsInfo, "internalMounts")
parseFiles.convertToJson("datProjectile", globalStrings, projectileInfo, "projectiles")

def buildVehicle(context, vehicleId, techBase, filename):
    herc = {
        "vehicleId": vehicleId,
        "techBase": techBase
    }
    context["vehicles"].append(herc)
    context["currentVehicle"] = herc
    print(f"Processing {filename}")
    with open(filename, "r") as stringFile:
        parseFiles.processFile(stringFile.read(), globalStrings, context)


def buildFlyer(context, vehicleId, filename):
    pass

def localExec(filename):
    pass

def createVehicleContext():
    result = hercInfo.createExecContext()
    result["buildHerc"] = partial(buildVehicle, result)
    result["buildTank"] = partial(buildVehicle, result)
    result["buildFlyer"] = partial(buildFlyer, result)
    result["buildDrone"] = partial(buildFlyer, result)
    result["exec"] = localExec

    return result


with open(f"datVehicle.cs", "r") as stringFile:
    fileContent = stringFile.read()
    fileContent = fileContent[fileContent.index("buildHerc( "):]
    context = createVehicleContext()
    parseFiles.processFile(fileContent, globalStrings, context)
    with open("datVehicle.json", "w") as outputFile:
        outputFile.write(json.dumps({"vehicles": context["vehicles"]}, indent="\t"))