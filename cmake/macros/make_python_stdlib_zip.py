# Copyright Contributors to the OpenDCC project
# SPDX-License-Identifier: Apache-2.0

import os, sys, site, zipfile

if len(sys.argv) != 2:
    print("Usage: {0} install_folder_path".format(sys.argv[0]))
    sys.exit(1)

python_version = str(sys.version_info[0]) + "." + str(sys.version_info[1])
if sys.platform == "win32":
    lib_path = os.path.join(sys.prefix, "lib")
else:
    lib_path = os.path.join(
        sys.prefix, "lib", "python" + str(sys.version_info[0]) + "." + str(sys.version_info[1])
    )
zf = zipfile.ZipFile(
    os.path.join(
        sys.argv[1], "python" + str(sys.version_info[0]) + str(sys.version_info[1]) + ".zip"
    ),
    mode="w",
    allowZip64=True,
)
exclude_dirs = ["site-packages", "lib-dynload"]
for dirname, subdirs, files in os.walk(lib_path):
    subdirs[:] = [d for d in subdirs if d not in exclude_dirs]
    root_path = os.path.relpath(dirname, lib_path)

    for filename in files:
        if os.path.splitext(filename)[1] != ".py":
            continue
        zf.write(os.path.join(dirname, filename), os.path.join(root_path, filename))

    skip_folder_path = False
    if root_path == ".":
        skip_folder_path = True
    elif dirname == lib_path:
        skip_folder_path = True
    if not skip_folder_path:
        zf.write(dirname, root_path)
zf.close()
