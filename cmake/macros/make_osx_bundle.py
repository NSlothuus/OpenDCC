# Copyright Contributors to the OpenDCC project
# SPDX-License-Identifier: Apache-2.0

import os, sys, site, zipfile, shutil, subprocess, glob
import resource
from posixpath import isabs
from distutils.dir_util import copy_tree
from macholib import MachO
from macholib import mach_o


def is_exe(macho_headers):
    filetype = macho_headers[0].header.filetype
    return filetype == mach_o.MH_EXECUTE


def is_dylib(macho_headers):
    filetype = macho_headers[0].header.filetype
    return filetype == mach_o.MH_DYLIB


def gather_execs_and_libs(install_dir, include_python_modules=False):
    execs = []
    libs = []
    ignored_dirs = [
        os.path.join(install_dir, "bin", "graphviz"),
        os.path.join(install_dir, "lib", "graphviz"),
    ]
    for dirname, subdirs, files in os.walk(install_dir):
        if not include_python_modules and dirname.startswith(
            os.path.join(install_dir, "lib", "python3.9")
        ):
            continue
        if dirname in ignored_dirs:
            continue
        for filename in files:
            abs_filename = os.path.join(dirname, filename)
            try:
                headers = MachO.MachO(abs_filename).headers
                if is_exe(headers):
                    execs.append(abs_filename)
                elif is_dylib(headers):
                    libs.append(abs_filename)

            except Exception:
                pass
    return execs, libs


def make_bundle_dir(dir, info_plist):
    os.makedirs(os.path.join(dir, "Contents", "MacOS"), exist_ok=True)
    os.makedirs(os.path.join(dir, "Contents", "Resources"), exist_ok=True)

    shutil.copy2(info_plist, os.path.join(dir, "Contents", os.path.basename(info_plist)))


def run_dylibbundler(targets, install_dir, resource_dir, extdeps_dir, copy_deps=False):
    args = ["dylibbundler"]
    args.append("-cd")  # If the output directory does not exist, create it.
    args.append("-of")  # When copying libraries to the output directory,
    # allow overwriting files when one with the same name already exists.
    # args.append('-ns') # no code sign
    if copy_deps:
        args.append("-b")  # Copies libraries to a local directory,
        # fixes their internal name so that they are aware of their new location
    ignore_paths = [
        os.path.join(install_dir, "lib"),
        os.path.join(install_dir, "plugin", "usd"),
        os.path.join(install_dir, "plugin", "opendcc"),
        os.path.join(resource_dir, "lib"),
        os.path.join(install_dir, "lib", "graphviz"),
    ]
    for p in ignore_paths:
        args.append("-i")
        args.append(p)

    args.append("-d")  # destination
    args.append(os.path.join(resource_dir, "lib"))
    args.append("-p")  # installed rpath
    args.append("@rpath/")  # assume executables lie in the 'bin' directory
    for target in targets:
        args.append("-x")
        args.append(target)

    # TODO: remove hardcoded paths
    search_dirs = [
        os.path.join(extdeps_dir, "pyside_5.15.2", "lib"),
        os.path.join(extdeps_dir, "ads-3.8.2_qt-5.15.2", "lib"),
        os.path.join(install_dir, "plugin", "usd"),
        os.path.join(install_dir, "lib"),
        os.path.join(install_dir, "plugin", "opendcc"),
        os.path.join(extdeps_dir, "boost_1.76.0", "lib"),
        os.path.join(extdeps_dir, "embree_3.12", "lib"),
        os.path.join(extdeps_dir, "tbb_2020.3", "lib"),
        os.path.join(extdeps_dir, "ocio_2.1.1", "lib"),
        os.path.join(extdeps_dir, "usd_22.05", "lib"),
        #'/Users/hetzner/projects/USD_install/lib',
        os.path.join(extdeps_dir, "ptex_2.4.0", "lib"),
        #'/usr/local/Cellar/python@3.9/3.9.14/Frameworks/Python.framework',
        os.path.join(extdeps_dir, "openexr_3.1.5", "lib"),
        os.path.join(extdeps_dir, "oiio_2.3.16.0", "lib"),
        os.path.join(extdeps_dir, "osl_1.11.17.0", "lib"),
        os.path.join(extdeps_dir, "alembic_1.8.2", "lib"),
        os.path.join(extdeps_dir, "openmesh_9.0", "lib"),
        os.path.join(extdeps_dir, "usd_22.05", "share", "usd", "examples", "plugin"),
    ]
    package_dirs = [t[0] for t in os.walk(os.path.join(install_dir, "packages"))]
    search_dirs.extend(package_dirs)

    for dir in search_dirs:
        args.append("-s")
        args.append(dir)

    print("Executing 'dylibbundler'...")
    print(" ".join(args))
    result = subprocess.run(
        args, input="quit", stdout=subprocess.PIPE, stderr=subprocess.STDOUT, timeout=900, text=True
    )
    if result.returncode != 0:
        print(result.stdout)
        exit(result.returncode)


def set_rpath_values(targets, rpaths):
    for target in targets:
        if not os.path.exists(target):
            continue
        macho_header = MachO.MachO(target).headers[0]
        for c in macho_header.commands:
            if c[0].cmd == mach_o.LC_RPATH:
                str_val = c[2].decode("utf-8").rstrip("\x00")
                install_name_tool(target, "-delete_rpath", str_val)
            elif c[0].cmd == mach_o.LC_LOAD_DYLIB:
                pass

        for rpath in rpaths:
            rel_path = "@loader_path/" + os.path.relpath(rpath, os.path.dirname(target))
            install_name_tool(target, "-add_rpath", rel_path)


def install_name_tool(target, flag, *args):
    args = ["install_name_tool", flag, *args]
    args.append(target)
    print(" ".join(args))
    result = subprocess.run(
        args, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, timeout=900, text=True
    )
    if result.returncode != 0:
        print(result.stdout)
        exit(result.returncode)


def install_qt_frameworks(resources_dir, install_dir, qt_path):
    qt_bins = glob.glob(os.path.join(install_dir, "lib", "Qt*"))
    for lib in qt_bins:
        os.remove(lib)
        target_name = os.path.basename(lib + ".framework")
        copy_tree(
            os.path.join(qt_path, "lib", target_name),
            os.path.join(os.path.join(resources_dir, "lib"), target_name),
        )


def install_python_lib(resources_dir, install_dir, extdeps_dir):
    # find python lib
    python_path = ""
    macho_header = MachO.MachO(os.path.join(install_dir, "bin", "dcc_base")).headers[0]
    for c in macho_header.commands:
        if c[0].cmd == mach_o.LC_LOAD_DYLIB:
            str_val = c[2].decode("utf-8").rstrip("\x00")
            if os.path.isabs(str_val) and str_val.endswith("Python.framework/Versions/3.9/Python"):
                python_path = str_val
                break
    if not python_path:
        return
    copied_python_path = os.path.join(install_dir, "lib", "Python")
    shutil.copy2(python_path, copied_python_path)
    install_name_tool(copied_python_path, "-id", "@rpath/Python")
    run_dylibbundler([copied_python_path], install_dir, resources_dir, extdeps_dir)

    execs, libs = gather_execs_and_libs(resources_dir, True)
    execs2, libs2 = gather_execs_and_libs(install_dir, True)
    for target in execs + libs + execs2 + libs2:
        install_name_tool(target, "-change", str_val, "@rpath/Python")


def replace_qt_conf(resources_dir):
    os.remove(os.path.join(resources_dir, "bin", "qt.conf"))
    f = open(os.path.join(resources_dir, "qt.conf"), "w")
    f.write(
        """[Paths]
Prefix=Resources
Plugins=qt-plugins"""
    )
    f.close()


# 1 install_dir
# 2 bundle_dir
# 3 path to Info.plist
# 4 extdeps dir
# 5 icon path
if len(sys.argv) != 6:
    print("Usage: {0} install_folder_path".format(sys.argv[0]))
    sys.exit(1)

install_dir = os.path.abspath(sys.argv[1])
dest_dir = os.path.abspath(sys.argv[2])
info_plist = os.path.abspath(sys.argv[3])
extdeps_dir = os.path.abspath(sys.argv[4])
icon_path = os.path.abspath(sys.argv[5])
lib_dir = os.path.join(install_dir, "lib")
bin_dir = os.path.join(install_dir, "bin")
resources_dir = os.path.join(dest_dir, "Contents", "Resources")
frameworks_dir = os.path.join(dest_dir, "Contents", "Frameworks")

# create bundle dir
make_bundle_dir(dest_dir, info_plist)
execs, libs = gather_execs_and_libs(install_dir, include_python_modules=False)

# # run dylibbundler on execs and libs
run_dylibbundler(execs + libs, install_dir, resources_dir, extdeps_dir, copy_deps=False)

# TODO: fix qt frameworks, right now we rely on CMake that has installed Qt binaries to install_dir/lib
install_qt_frameworks(resources_dir, install_dir, os.path.join(extdeps_dir, "qt_5.15.2"))
install_python_lib(resources_dir, install_dir, extdeps_dir)

# fix RPATH values of copied libs: set relative to targets @loader_paths
set_rpath_values(
    libs + execs, [os.path.join(install_dir, "lib"), os.path.join(install_dir, "plugin", "usd")]
)

# copy install_dir
copy_tree(install_dir, resources_dir, preserve_symlinks=True)

# replace qt.conf
replace_qt_conf(resources_dir)

# move dcc_base to MacOS
new_dcc_base_path = os.path.join(dest_dir, "Contents", "MacOS", "dcc_base")
shutil.move(os.path.join(resources_dir, "bin", "dcc_base"), new_dcc_base_path)
set_rpath_values(
    [new_dcc_base_path],
    [os.path.join(resources_dir, "lib"), os.path.join(resources_dir, "plugin", "usd")],
)

print("Finished")
