# Copyright Contributors to the OpenDCC project
# SPDX-License-Identifier: Apache-2.0

from pygments.lexer import RegexLexer
from pygments.token import Keyword


class ExpandVarsLexer(RegexLexer):
    name = "ExpandVars"
    aliases = ["expand_vars"]
    filenames = ["*.exp"]

    tokens = {
        "root": [
            (r"\${\w+}", Keyword),
            (r"\$\w+", Keyword),
        ]
    }
