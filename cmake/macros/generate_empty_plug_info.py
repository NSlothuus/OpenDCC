# Copyright Contributors to the OpenDCC project
# SPDX-License-Identifier: Apache-2.0

from jinja2 import Environment, FileSystemLoader
import os
import sys
from pxr import Plug


if __name__ == "__main__":
    template_path = os.path.join(
        os.path.abspath(Plug.Registry().GetPluginWithName("usd").resourcePath), "codegenTemplates"
    )

    if len(sys.argv) < 3:
        print(
            "Not enough arguments. Expected libraryName for codegen jinja template and output_path for resulting file"
        )
        sys.exit(-1)

    libraryName = sys.argv[1]
    output_path = sys.argv[2]
    j2_env = Environment(loader=FileSystemLoader(template_path), trim_blocks=True)
    j2_env.globals.update(libraryName=libraryName)

    plugInfo_template = j2_env.get_template("plugInfo.json")

    with open(output_path, "w") as f:
        f.write(plugInfo_template.render())
