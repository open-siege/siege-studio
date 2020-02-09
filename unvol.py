import sys
import extract_file
import volinfo
import glob

importFilenames = []

for importFilename in sys.argv[1:]:
    files = glob.glob(importFilename)
    importFilenames.extend(files)

for importFilename in importFilenames:
    print("processing " + importFilename)
    try:
        extract_file.extract_archive(importFilename, volinfo)
    except Exception as e:
        print(e)
