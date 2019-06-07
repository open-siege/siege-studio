import sys
import struct
import os
from collections import namedtuple

importFilenames = sys.argv[1:]

compressionTypes = {
    0: "none",
    1: "rle",
    2: "lz",
    3: "lzh"
}

FileInfo = namedtuple("FileInfo", "compression_type start_offset end_offset length filename")

vol = "VOL".encode("utf-8")
vols = "vols".encode("utf-8")
voli = "voli".encode("utf-8")
vblk = "VBLK".encode("utf-8")
eos = "\0".encode("utf-8")


def git_file_list_offsets(raw_data):
    offset = 0
    header_fmt = "<4sL"

    footer_fmt = "<4sL4sL4sL"
    (header, lengthUntilFooter) = struct.unpack_from(header_fmt, raw_data, offset)

    if vol not in header:
        raise ValueError("File header is not VOL as expected")

    offset = lengthUntilFooter
    footer = struct.unpack_from(footer_fmt, raw_data, offset)

    if footer[0] != vols:
        raise ValueError("vols section not found")

    offset += struct.calcsize(footer_fmt)
    file_list_end_index = offset + footer[-1]

    return offset, file_list_end_index


def get_file_names(raw_data):
    (offset, fileListEndIndex) = git_file_list_offsets(raw_data)
    filenames = []
    while offset < fileListEndIndex:
        end_index = raw_data.index(eos, offset)
        filenames.append(raw_data[offset:end_index].decode("utf-8"))
        offset += (end_index - offset + 1)
    return filenames, offset


def get_file_metadata(raw_data):
    (filenames, offset) = get_file_names(raw_data)
    item_header_fmt = "<4sL"
    item_fmt = "<4LB"
    file_info = []
    offset = raw_data.index(voli, offset)
    items_header = struct.unpack_from(item_header_fmt, raw_data, offset)

    if items_header[0] != voli:
        raise ValueError("voli section not found")

    offset += struct.calcsize(item_header_fmt)
    final_offset = offset + items_header[1]
    while offset < final_offset:
        item = struct.unpack_from(item_fmt, raw_data, offset)
        file_info.append(item)
        offset += struct.calcsize(item_fmt)

    return file_info, offset


def write_files(raw_data, dest_dir, file_info, filenames):
    fileFmt = "<4s4s"
    for index, info in enumerate(file_info):
        offset = info[2]
        filename = filenames[index]
        (fileHeader, fileLengthRaw) = struct.unpack_from(fileFmt, raw_data, offset)
        # turning the random data at the end of the file length into \0 to parse correctly
        (fileLength,) = struct.unpack("<L", fileLengthRaw[:-1] + eos)

        if (fileLength != info[3]):
            print("File length mismatch for", filename, "Footer value", info[3], "Header value", fileLength)


        offset += struct.calcsize(fileFmt)
        if fileHeader == vblk:
            print("writing " + dest_dir + "/" + filename)
            with open(dest_dir + "/" + filename, "wb") as shapeFile:
                new_file_byte_array = bytearray(raw_data[offset:offset + info[3]])
                shapeFile.write(new_file_byte_array)


def extract_archive(import_filename):
    with open(import_filename, "rb") as input_fd:
        raw_data = input_fd.read()

    dest_dir = import_filename.replace(".vol", "").replace(".VOL", "")

    (files, offset) = get_file_names(raw_data)
    (fileInfo, offset) = get_file_metadata(raw_data)

    if not os.path.exists(dest_dir):
        os.makedirs(dest_dir)
    write_files(raw_data, dest_dir, fileInfo, files)


for importFilename in importFilenames:

    print("reading " + importFilename)
    try:
        extract_archive(importFilename)
    except Exception as e:
        print(e)
