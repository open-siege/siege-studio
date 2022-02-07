import sys
import glob
import json

importFilenames = []

for importFilename in sys.argv[1:]:
    files = glob.glob(importFilename)
    importFilenames.extend(files)

def readDataFile(filename):
    with open(filename, "r") as vehicleInfo:
        return json.loads(vehicleInfo.read())


constants = readDataFile("vehFileConstants.json")
vehicleData = readDataFile("datVehicle.json")
engineData = readDataFile("datEngine.json")
reactorData = readDataFile("datReactor.json")
mountData = readDataFile("datIntMounts.json")
shieldData = readDataFile("datShield.json")
armorData = readDataFile("datArmor.json")

def generateRawIds(collection, collectionIdName, componentInfo):
    results = []
    groupId = componentInfo["groupIdStart"]
    engineId = componentInfo["componentIdStart"]
    for item in collection:
        results.append({
            "datId": item[collectionIdName],
            "fileIds": (groupId, engineId),
            "item": item
        })
        if len(results) % componentInfo["groupMaxCount"] == 0:
            groupId += 1
            engineId = componentInfo["componentIdStart"]
        else:
            engineId += componentInfo["idDelta"]
    return results


engines = generateRawIds(engineData["engines"], "engineId", constants["engine"])
reactors = generateRawIds(reactorData["reactors"], "reactorId", constants["reactor"])
computers = generateRawIds(filter(lambda x: x["componentType"] == "computer", mountData["internalMounts"]), "internalMountId", constants["computer"])
shields = generateRawIds(shieldData["shields"], "shieldId", constants["shield"])
armors = generateRawIds(armorData["armors"], "armorId", constants["armor"])

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


def findComponent(rawVehicle, componentInfo, collection, fallback):
    index = componentInfo["idIndex"]
    return next(
        filter(lambda x: x["fileIds"] == (rawVehicle[index], rawVehicle[index + 1]), collection),
        fallback)
def getComponentName(component):
    return component["item"]["displayInfo"]["longName"]["text"]["en"]

for importFilename in importFilenames:
    with open(importFilename, "rb") as rawVehicleFile:
        rawVehicle = rawVehicleFile.read()

    engineId = constants["engine"]["idIndex"]
    reactorId = constants["reactor"]["idIndex"]
    computerId = constants["computer"]["idIndex"]
    shieldId = constants["shield"]["idIndex"]
    armorId = constants["armor"]["idIndex"]
    rawVehicleId = rawVehicle[constants["vehicle"]["idIndex"]]
    rawEngineId = rawVehicle[engineId]
    matchingInfo = next(filter(lambda x: x["vehicleId"] == rawVehicleId - 140, vehicleData["vehicles"]), None)
    if matchingInfo is not None:
        matchingEngine = findComponent(rawVehicle, constants["engine"], engines, defaultComponent)
        matchingReactor = findComponent(rawVehicle, constants["reactor"], reactors, defaultComponent)
        matchingComputer = findComponent(rawVehicle, constants["computer"], computers, defaultComponent)
        matchingShield = findComponent(rawVehicle, constants["shield"], shields, defaultComponent)
        matchingArmor = findComponent(rawVehicle, constants["armor"], armors, defaultComponent)

        engineName = getComponentName(matchingEngine)
        reactorName = getComponentName(matchingReactor)
        computerName = getComponentName(matchingComputer)
        shieldName = getComponentName(matchingShield)
        armorName = getComponentName(matchingArmor)

        print(importFilename, ": ", ", ".join([matchingInfo["identityTag"]["text"]["en"],engineName, reactorName, computerName, shieldName, armorName]))
    else:
        print(importFilename, rawVehicleId, rawEngineId)