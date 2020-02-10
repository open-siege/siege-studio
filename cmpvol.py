import sys
import glob
import hashlib

importFiles = []

HEADER = '\033[95m'
OKBLUE = '\033[94m'
OKGREEN = '\033[92m'
WARNING = '\033[93m'
FAIL = '\033[91m'
ENDC = '\033[0m'
BOLD = '\033[1m'
UNDERLINE = '\033[4m'

for importFile in sys.argv[1:]:
    files = glob.glob(importFile)
    importFiles.extend(files)

for importFile in importFiles:
    with open(importFile, "rb") as volumeFile:
        rawData = volumeFile.read()
    file1Hash = hashlib.sha256(rawData).hexdigest()

    with open(importFile + ".new", "rb") as volumeFile:
        rawData = volumeFile.read()
    file2Hash = hashlib.sha256(rawData).hexdigest()

    if file1Hash == file2Hash:
        print(f"{OKGREEN}{importFile} matches {importFile}.new{ENDC}")

    if file1Hash != file2Hash:
        print(f"{FAIL}{importFile} does not match {importFile}.new{ENDC}")

