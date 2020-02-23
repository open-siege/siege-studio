import sys
import glob
import json

importFilenames = []

VEHICLE_ID = 26
ENGINE_ID = 28
REACTOR_ID = 30
COMPUTER_ID = 32
SHIELD_ID = 34
ARMOR_ID = 36

ENGINE_GROUP_ID_START = 124
ENGINE_ID_START = 46
ENGINE_ID_DELTA = 64
ENGINE_ID_GROUP_MAX_COUNT = 4

REACTOR_GROUP_ID_START = 170
REACTOR_ID_START = 93
REACTOR_ID_DELTA = 128
REACTOR_ID_GROUP_MAX_COUNT = 2

COMPUTER_GROUP_ID_START = 214
COMPUTER_ID_START = 99
COMPUTER_ID_DELTA = 128
COMPUTER_ID_GROUP_MAX_COUNT = 2

SHIELD_GROUP_ID_START = 247
SHIELD_ID_START = 24
SHIELD_ID_DELTA = 32
SHIELD_ID_GROUP_MAX_COUNT = 6

ARMOR_GROUP_ID_START = 101
ARMOR_ID_START = 86
ARMOR_ID_DELTA = 32
ARMOR_ID_GROUP_MAX_COUNT = 6

for importFilename in sys.argv[1:]:
    files = glob.glob(importFilename)
    importFilenames.extend(files)

def readDataFile(filename):
    with open(filename, "r") as vehicleInfo:
        return json.loads(vehicleInfo.read())

vehicleData = readDataFile("datVehicle.json")
engineData = readDataFile("datEngine.json")
reactorData = readDataFile("datReactor.json")
mountData = readDataFile("datIntMounts.json")
shieldData = readDataFile("datShield.json")
armorData = readDataFile("datArmor.json")

def generateRawIds(collection, collectionIdName,groupIdStart, componentIdStart, maxCount, delta):
    results = []
    groupId = groupIdStart
    engineId = componentIdStart
    for item in collection:
        results.append({
            "datId": item[collectionIdName],
            "fileIds": (groupId, engineId),
            "item": item
        })
        if len(results) % maxCount == 0:
            groupId += 1
            engineId = componentIdStart
        else:
            engineId += delta
    return results


engines = generateRawIds(engineData["engines"], "engineId", ENGINE_GROUP_ID_START, ENGINE_ID_START, ENGINE_ID_GROUP_MAX_COUNT, ENGINE_ID_DELTA)
reactors = generateRawIds(reactorData["reactors"], "reactorId", REACTOR_GROUP_ID_START, REACTOR_ID_START, REACTOR_ID_GROUP_MAX_COUNT, REACTOR_ID_DELTA)
computers = generateRawIds(filter(lambda x: x["componentType"] == "computer", mountData["internalMounts"]), "internalMountId", COMPUTER_GROUP_ID_START, COMPUTER_ID_START, COMPUTER_ID_GROUP_MAX_COUNT, COMPUTER_ID_DELTA)
shields = generateRawIds(shieldData["shields"], "shieldId", SHIELD_GROUP_ID_START, SHIELD_ID_START, SHIELD_ID_GROUP_MAX_COUNT, SHIELD_ID_DELTA)
armors = generateRawIds(armorData["armors"], "armorId", ARMOR_GROUP_ID_START, ARMOR_ID_START, ARMOR_ID_GROUP_MAX_COUNT, ARMOR_ID_DELTA)

defaultComponent = {
    "item": {
        "displayInfo": {
            "longName": {
                "text": {
                    "en": "N/A"
                }
            }
        }
    }
}


for importFilename in importFilenames:
    with open(importFilename, "rb") as rawVehicleFile:
        rawVehicle = rawVehicleFile.read()

    rawVehicleId = rawVehicle[VEHICLE_ID]
    rawEngineId = rawVehicle[ENGINE_ID]
    matchingInfo = next(filter(lambda x: x["vehicleId"] == rawVehicleId - 140, vehicleData["vehicles"]), None)
    if matchingInfo is not None:
        matchingEngine = next(filter(lambda x: x["fileIds"] == (rawVehicle[ENGINE_ID], rawVehicle[ENGINE_ID + 1]), engines), defaultComponent)
        matchingReactor = next(
            filter(lambda x: x["fileIds"] == (rawVehicle[REACTOR_ID], rawVehicle[REACTOR_ID + 1]), reactors),
            defaultComponent)

        matchingComputer = next(
            filter(lambda x: x["fileIds"] == (rawVehicle[COMPUTER_ID], rawVehicle[COMPUTER_ID + 1]), computers),
            defaultComponent)

        matchingShield = next(
            filter(lambda x: x["fileIds"] == (rawVehicle[SHIELD_ID], rawVehicle[SHIELD_ID + 1]), shields),
            defaultComponent)

        matchingArmor = next(
            filter(lambda x: x["fileIds"] == (rawVehicle[ARMOR_ID], rawVehicle[ARMOR_ID + 1]), armors),
            defaultComponent)

        engineName = matchingEngine["item"]["displayInfo"]["longName"]["text"]["en"]
        reactorName = matchingReactor["item"]["displayInfo"]["longName"]["text"]["en"]
        computerName = matchingComputer["item"]["displayInfo"]["longName"]["text"]["en"]
        shieldName = matchingShield["item"]["displayInfo"]["longName"]["text"]["en"]
        armorName = matchingArmor["item"]["displayInfo"]["longName"]["text"]["en"]

        print(importFilename, ": ", ", ".join([matchingInfo["identityTag"]["text"]["en"],engineName, reactorName, computerName, shieldName, armorName]))
    else:
        print(importFilename, rawVehicleId, rawEngineId)