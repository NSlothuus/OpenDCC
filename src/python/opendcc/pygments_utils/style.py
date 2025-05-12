# Copyright Contributors to the OpenDCC project
# SPDX-License-Identifier: Apache-2.0

from pygments.style import Style
from pygments.token import (
    Keyword,
    Name,
    Comment,
    String,
    Error,
    Literal,
    Number,
    Operator,
    Other,
    Punctuation,
    Text,
    Generic,
    Whitespace,
)


class OpenDCCStyle(Style):
    background_color = "#282a36"
    default_style = ""

    styles = {
        Comment: "#6e6e6e",
        Keyword: "#0ab4ff bold",
        Literal: "#f8f8f2",
        Number: "#f09650 bold",
        Operator: "#0a96d2",
        String: "#64B496",
        Text: "#c8c8c8",
        Name: "#c8c8c8",
        Whitespace: "#c8c8c8",
        Other: "#c8c8c8",
        Punctuation: "#c8c8c8",
        Error: "#c8c8c8",
    }


class OpenDCCLightStyle(Style):
    background_color = "#282a36"
    default_style = ""

    styles = {
        Comment: "#00bf08",
        Keyword: "#004cbf bold",
        Literal: "#f8f8f2",
        Number: "#f09650 bold",
        Operator: "#0a96d2",
        String: "#ff1010",
        Text: "#000000",
        Name: "#000000",
        Whitespace: "#000000",
        Other: "#000000",
        Punctuation: "#000000",
        Error: "#c8c8c8",
    }
