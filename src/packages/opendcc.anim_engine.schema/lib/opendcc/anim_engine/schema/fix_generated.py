# Copyright Contributors to the OpenDCC project
# SPDX-License-Identifier: Apache-2.0

import os
import sys
import platform
import re
import json


def fix_plug_info(output_dir):
    pluginfo_path = os.path.join(output_dir, "plugInfo.json.in")
    f = open(pluginfo_path, "r")
    contents = f.read()

    data = json.loads(re.sub("#.*", "", contents, flags=re.MULTILINE))

    sdf_metadata = data["Plugins"][0]["Info"].get("SdfMetadata", {})
    sdf_metadata = {"anim": {"appliesTo": "attributes", "type": "dictionary"}}

    data["Plugins"][0]["Info"]["SdfMetadata"] = sdf_metadata

    f = open(pluginfo_path, "w")
    json.dump(data, f, indent=4)


if __name__ == "__main__":
    if len(sys.argv) < 1:
        print("Not enough arguments!")
        sys.exit(1)

    fix_plug_info(os.getcwd())
