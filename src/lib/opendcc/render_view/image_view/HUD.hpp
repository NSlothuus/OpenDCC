// Copyright Contributors to the OpenDCC project
// SPDX-License-Identifier: Apache-2.0

#pragma once
#include "opendcc/opendcc.h"
#include <QImage>
#include <functional>

OPENDCC_NAMESPACE_OPEN

class RenderViewHUD
{
public:
    static void init_gl();
    void add_text(int x, int y, const std::string& text);
    void draw(std::function<void(float widget_x, float widget_y, float& image_x, float& image_y)> image_to_widget_pos);
    void clear() { m_groups.clear(); }

private:
    static GLint m_simple_textue_shader_program;
    struct Group
    {
        ~Group();
        int x = 0;
        int y = 0;
        GLuint texture;
        QImage image;
    };

    std::vector<Group> m_groups;
};

OPENDCC_NAMESPACE_CLOSE
