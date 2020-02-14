import parseStrings
import engineInfo
import reactorInfo
import sensorInfo
import armorInfo
import shieldInfo
import projectileInfo
import json

globalStrings = parseStrings.parseStringsFromFile("Sim.Strings.cs")

def convertToJson(filename, module, finalKey):
    context = module.createExecContext()

    with open(f"{filename}.cs", "r") as stringFile:
        lines = stringFile.read().splitlines()
        for line in lines:
            line = line.strip()
            if line.startswith("//"):
                continue
            exec(line, globalStrings, context)

    with open(f"{filename}.json", "w") as outputFile:
        outputFile.write(json.dumps({finalKey: context[finalKey]}, indent="\t"))


convertToJson("datEngine", engineInfo, "engines")
convertToJson("datReactor", reactorInfo, "reactors")
convertToJson("datSensor", sensorInfo, "sensors")
convertToJson("datArmor", armorInfo, "armors")
convertToJson("datShield", shieldInfo, "shields")
convertToJson("datProjectile", projectileInfo, "projectiles")