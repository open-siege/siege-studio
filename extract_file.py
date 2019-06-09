import os

def write_files(raw_data, dest_dir, file_info):
    for index, info in enumerate(file_info):
        offset = info.start_offset
        end_offset = info.end_offset
        filename = info.filename
        with open(dest_dir + "/" + filename, "wb") as shapeFile:
            print("writing " + dest_dir + "/" + filename + ", compression type:", info.compression_type)
            new_file_byte_array = bytearray(raw_data[offset:end_offset])
            shapeFile.write(new_file_byte_array)


def extract_archive(import_filename, archive_module):
    with open(import_filename, "rb") as input_fd:
        raw_data = input_fd.read()

    dest_dir = import_filename.replace(".vol", "").replace(".VOL", "")

    file_info = archive_module.get_file_metadata(raw_data)

    if not os.path.exists(dest_dir):
        os.makedirs(dest_dir)
    write_files(raw_data, dest_dir, file_info)
