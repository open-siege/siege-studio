def createGenericInfo(shortNameTag: object, longNameTag: object, smallBmp: object, smallDisBmp: object, largeBmp: object, largeDisBmp: object, description: object) -> object:
    return {
        "shortName": {
            "constantName": shortNameTag["constantName"],
            "numericTag": shortNameTag["numericTag"],
            "text": {
                "en": shortNameTag["text"]["en"]
            }
        },
        "longName": {
            "constantName": longNameTag["constantName"],
            "numericTag": longNameTag["numericTag"],
            "text": {
                "en": longNameTag["text"]["en"]
            }
        },
        "smallBmp": smallBmp,
        "smallDisBmp": smallDisBmp,
        "largeBmp": largeBmp,
        "largeDisBmp": largeDisBmp,
        "description": {
            "constantName": description["constantName"],
            "numericTag": description["numericTag"],
            "text": {
                "en": description["text"]["en"]
            }
        }
    }