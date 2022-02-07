from wand.api import library
from collections.abc import Iterable
import base64
import wand.color
import wand.image
import numpy
import xml.etree.ElementTree as ET

# TODO make these command line parameters
input = "src/besieged-theme.svg"
output = "src/besieged-theme.png"
theme_file = "src/besieged-theme.txt"

# Texture Generation

with open(input, "rb") as svg_file:
    with wand.image.Image() as image:
        with wand.color.Color('transparent') as background_color:
            library.MagickSetBackgroundColor(image.wand,
                                             background_color.resource)
        image.read(blob=svg_file.read(), format="svg")
        png_image = image.make_blob("png32")

root = ET.fromstring(png_image.decode("utf-8"))

image = list(root.iterfind("{http://www.w3.org/2000/svg}image"))

raw_data = image[0].get("href").replace("data:image/png;base64,", "")

with open(output, "wb") as out:
    out.write(base64.b64decode(raw_data))

# Theme Generation

# TODO support other types of SVG element transformations.
# They should likely all produce a matrix, to make the calculation part consistent.
def matrix(a, b, c, d, e, f):
    return ([a, c, e], [b, d, f], [0, 0, 1])


results = {}

svg_group = "{http://www.w3.org/2000/svg}g"

colorProperties = {"TextColor", "TextColorFocused", "TextColorDisabled", "BorderColor", "BackgroundColor"}

# TODO support more property types
# and also make this code more dynamic
def get_value_for_prop(prop, values, rect):
    if prop == "TitleBarHeight":
        return values[-1]

    if prop in colorProperties:
        return rect.get("style").replace("fill:", "").replace(";", "")

    if prop == "Scrollbar":
        return "&Scrollbar"

    if prop == "ListBox":
        return "&ListBox"

    return None


def process_node(item, parent_transform=None):
    id = item.get("id")

    if item.tag == svg_group:
        transform = item.get("transform")

        if transform is not None:
            transform = eval(transform)

        if parent_transform is not None and transform is not None:
            transform = numpy.dot(parent_transform, transform)
        elif parent_transform is not None and transform is None:
            transform = parent_transform

        if id is not None and "." in id:
            values = id.split(".")
            
            rect = item[0]
            x = float(rect.get("x"))
            y = float(rect.get("y"))
            width = float(rect.get("width"))
            height = float(rect.get("height"))
            
            [new_x, new_y, new_z] = numpy.dot(transform, [x, y, 1])
            [new_xx, new_yy, new_zz] = numpy.dot(transform, [x + width, y + height, 1])
            
            if len(values) == 2:
                [name, prop] = values            

                if name not in results:
                    results[name] = {}

                if prop not in results[name]:
                    results[name][prop] = {}

                results[name][prop] = get_value_for_prop(prop, [new_x, new_y, new_xx - new_x, new_yy - new_y], rect)
            
            if len(values) == 3:
                [name, prop, section] = values            
                
                if name not in results:
                    results[name] = {}

                if prop not in results[name]:
                    results[name][prop] = {}

                results[name][prop][section] = [new_x, new_y, new_xx - new_x, new_yy - new_y]
        for child in item:
            process_node(child, transform)


widgets_root = ET.parse(input)

for item in widgets_root.getroot():
    if item.tag == svg_group:
        process_node(item)

text_results = ""

op = "{"
close = "}"

for [key, value] in results.items():
    props = ""
    for [child, values] in value.items():
        if isinstance(values, Iterable) and "Part" in values:
            props += f'\t{child} = "{output}"'
            coords = values["Part"]
            props += f" Part({coords[0]}, {coords[1]}, {coords[2]}, {coords[3]})"

            if "Middle" in values:
                middle = values["Middle"]
                props += f" Middle({middle[0] - coords[0]}, {middle[1] - coords[1]}, {middle[2]}, {middle[3]})"

            props += " Smooth;\n"
        else:
            props += f'\t{child} = {values};\n'
    text_results += f"{key} {op}\n {props} {close}\n\n"

with open(theme_file, "w") as out:
    out.write(text_results)
