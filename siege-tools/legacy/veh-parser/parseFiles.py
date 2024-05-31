import json

def parseStringsFromFile(importFilename):
    translations = {}
    with open(importFilename, "r") as stringFile:
        lines = stringFile.read().splitlines()
        for line in lines:
            if "=" in line and ";" in line:
                (name, values) = line.split("=")
                name = name.strip()
                splitValues = values.split(",")
                values = ";".join(values.replace(splitValues[0] + ",", "").strip().split(";")[:-1], )

                translations[name] = {
                    "constantName": name,
                    "numericTag": splitValues[0].strip(),
                    "text": {
                        "en": eval(values)
                    }
                }
    return translations

def processFile(stringData: str, globalStrings: dict, context: dict):
    lines = stringData.splitlines()
    skipLine = False
    for line in lines:
        if skipLine == True:
            skipLine = False
            continue
        originalLine = line
        line = line.strip()
        if line.startswith("//"):
            continue
        try:
            exec(line, globalStrings, context)
        except SyntaxError as error:
            if error.msg.find("EOF") != -1:
                nextLine = lines[lines.index(originalLine) + 1]
                exec(line + nextLine, globalStrings, context)
                skipLine = True
            else:
                print(error)

def convertToJsonWithContext(filename: str, globalStrings: dict, context: dict, finalKey: str):
    with open(f"{filename}.cs", "r") as stringFile:
        processFile(stringFile.read(), globalStrings, context)

    with open(f"{filename}.json", "w") as outputFile:
        outputFile.write(json.dumps({finalKey: context[finalKey]}, indent="\t"))

def convertToJson(filename: str, globalStrings: dict, module, finalKey: str):
    context = module.createExecContext()
    convertToJsonWithContext(filename, globalStrings, context, finalKey)