import reactorInfo

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

reactorContext = reactorInfo.createExecContext()

execString = 'newReactor (	200 ,	60 ,	400 ,	345	)'

exec(execString, reactorContext)


print(reactorContext["reactors"])