
// import sys
// import struct
// import os

// from collections import namedtuple

// def readFiles(numFiles, infoFmt, offset, adjustExtension):
// 	files = []
// 	for i in range(numFiles):
// 		rawValues = struct.unpack_from(infoFmt, rawData, offset)
// 		fileName = rawValues[0]
// 		fileOffset = rawValues[1]
// 		fileName = fileName.split("\0".encode("ascii"))[0]
// 		offset += struct.calcsize(infoFmt)
// 		if adjustExtension:
// 			fileName = fileName.replace("_", ".")
// 		files.append((fileName, fileOffset))

// 	return files


// importFilenames = sys.argv[1:]

// for importFilename in importFilenames:

// 	if not os.path.exists("extracted"):
// 		os.makedirs("extracted")

// 	print("reading " + importFilename)
// 	try:
// 		with open(importFilename, "rb") as input_fd:
// 			rawData = input_fd.read()
// 		# if there is a CREDITS_PAL, it's the Colony Wars Red Run RSC
// 		if rawData.find("CREDITS_PAL".encode("ascii")) != -1:
// 			# Can't find where the total number of files are in the file
// 			# and this is so bad :(
// 			numFiles = 1209
// 			offset = rawData.find("CREDITS_PAL".encode("utf-8"))
// 			infoFmt = "<16sl"
// 			tempFiles = readFiles(numFiles, infoFmt, offset, True)
// 			offset += numFiles * struct.calcsize(infoFmt)

// 			moreInfoFmt = "<2L"
// 			# This is even worse.
// 			# I would love to figure out why we jump so far
// 			# This is the starting point of the file offsets
// 			# And still might be wrong
// 			offset = 27592
// 			files = []
// 			for i in range(numFiles):
// 				fileInfo = struct.unpack_from(moreInfoFmt, rawData, offset)
// 				offset += struct.calcsize(moreInfoFmt)
// 				files.append((tempFiles[i][0], fileInfo[1]))

// 			print(files[0], files[1])
// 		else:
// 			offset = 0
// 			(numFiles, ) = struct.unpack_from("<L", rawData, offset)
// 			offset += struct.calcsize("<L")
// 			infoFmt = "<16s2l4h"
// 			files = readFiles(numFiles, infoFmt, offset, False)

// 			# check if the second filename has been parsed properly
// 			if ".".encode("utf-8") not in files[1][0]:
// 				infoFmt = "<16sl"
// 				files = readFiles(numFiles, infoFmt, offset, False)

// 		for index, file in enumerate(files):
// 			filename, fileOffset = file
// 			print("extracting " + filename.decode("utf-8") + " " + str(fileOffset))
// 			nextFileOffset = len(rawData)
// 			if index + 1 < numFiles:
// 				nextFilename, nextFileOffset = files[index + 1]
// 			with open("extracted/" + filename.decode("utf-8"), "wb") as shapeFile:
// 				newFileByteArray = bytearray(rawData[fileOffset:nextFileOffset])
// 				shapeFile.write(newFileByteArray)

// 	except Exception as e:
// 		print(e)
