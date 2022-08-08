import sys
import glob
import conanfile

current_version = conanfile.LocalConanFile.version.split(".")

if sys.argv[1] == "major":
    current_version[0] = str(int(current_version[0]) + 1)
    current_version[1] = "0"
    current_version[2] = "0"
elif sys.argv[1] == "minor":
    current_version[1] = str(int(current_version[1]) + 1)
    current_version[2] = "0"
elif sys.argv[1] == "patch":
    current_version[2] = str(int(current_version[2]) + 1)

current_version = ".".join(current_version)
old_version = conanfile.LocalConanFile.version

paths = [".", "3space", "3space-studio", "extender", "extender/darkstar"]

for filename in glob.glob("**/*.py", recursive=True):
    if "conanfile.py" not in filename:
        continue
    print(f"Updating {filename}")
    with open(filename, "r") as file:
        # f for f in files if not f.endswith('.rc')
        old = f"version = \"{old_version}\""
        new = f"version = \"{current_version}\""
        lines = [line.replace(old, new) if old in line else line for line in file.readlines()]

    with open(filename, "w") as file:
        file.writelines(lines)

print(f"New version is {current_version}")