import parseStrings
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
import glob

globalStrings = parseStrings.parseStringsFromFile("Sim.Strings.cs")

def convertToJson(filename, module, finalKey):
    context = module.createExecContext()

    with open(f"{filename}.cs", "r") as stringFile:
        lines = stringFile.read().splitlines()
        skipLine = False
        for line in lines:
            if skipLine == True:
                skipLine = False
                continue
            line = line.strip()
            if line.startswith("//"):
                continue
            try:
                exec(line, globalStrings, context)
            except SyntaxError as error:
                if error.msg.find("EOF") != -1:
                    nextLine = lines[lines.index(line) + 1]
                    exec(line + nextLine, globalStrings, context)
                    skipLine = True
                else:
                    print(error)

    with open(f"{filename}.json", "w") as outputFile:
        outputFile.write(json.dumps({finalKey: context[finalKey]}, indent="\t"))


convertToJson("datEngine", engineInfo, "engines")
convertToJson("datReactor", reactorInfo, "reactors")
convertToJson("datSensor", sensorInfo, "sensors")
convertToJson("datArmor", armorInfo, "armors")
convertToJson("datShield", shieldInfo, "shields")
convertToJson("datWeapon", weaponInfo, "weapons")
convertToJson("datIntMounts", intMountsInfo, "internalMounts")
convertToJson("datProjectile", projectileInfo, "projectiles")

hercs = glob.glob("datHerc_*.cs")

for herc in hercs:
    convertToJson(herc.replace(".cs", ""), hercInfo, "hercs")