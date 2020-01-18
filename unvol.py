import sys
import extract_file
import volinfo

importFilenames = sys.argv[1:]

for importFilename in importFilenames:

    print("processing " + importFilename)
    try:
        extract_file.extract_archive(importFilename, volinfo)
    except Exception as e:
        print(e)
