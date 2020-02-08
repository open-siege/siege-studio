import os
import json
import subprocess

def write_files(import_filename, raw_data, dest_dir, file_info):
    for index, info in enumerate(file_info):
        offset = info.start_offset
        end_offset = info.end_offset
        filename = info.filename
        if (info.compression_type == "none"):
            with open(dest_dir + "/" + filename, "wb") as shapeFile:
                print("extracting " + dest_dir + "/" + filename + ", compression type:", info.compression_type)
                new_file_byte_array = bytearray(raw_data[offset:end_offset])
                shapeFile.write(new_file_byte_array)
        else:
            print("using extract.exe to extract " + dest_dir + "/" + filename + ", compression type:", info.compression_type)
            subprocess.call(["extract.exe", import_filename, info.filename, dest_dir + "/" + filename])
            with open(dest_dir + "/" + filename, "rb") as outputFile:
                raw_data = outputFile.read()
                if len(raw_data) != info.size:
                    print(filename + " has extra bytes which are being truncated")
                
                raw_data = raw_data[0:info.size]
                
            with open(dest_dir + "/" + filename, "wb") as outputFile:
                outputFile.write(bytearray(raw_data))


def extract_archive(import_filename, archive_module):
    with open(import_filename, "rb") as input_fd:
        raw_data = input_fd.read()

    dest_dir = import_filename.replace(".vol", "").replace(".VOL", "")

    file_info = archive_module.get_file_metadata(raw_data)

    volumeStructure = {
        "files": []
    }

    for file in file_info:
        volumeStructure["files"].append(file.filename)

    if not os.path.exists(dest_dir):
        os.makedirs(dest_dir)

    print("Writing " + os.path.join(dest_dir, import_filename + ".json") )
    with open(os.path.join(dest_dir, import_filename + ".json"), "w") as volumeFile:
        volumeFile.write(json.dumps(volumeStructure, indent="\t"))

    write_files(import_filename, raw_data, dest_dir, file_info)
