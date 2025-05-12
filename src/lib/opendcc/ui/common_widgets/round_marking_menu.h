/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once
#include "opendcc/opendcc.h"
#include "opendcc/ui/common_widgets/api.h"
#include "opendcc/ui/common_widgets/marking_menu.h"

OPENDCC_NAMESPACE_OPEN

/**
 * @brief The RoundMarkingMenu class represents a round marking menu.
 *
 * It inherits from MarkingMenu and provides functionality for
 * creating a round marking menu at a specific position.
 */
class OPENDCC_UI_COMMON_WIDGETS_API RoundMarkingMenu : public MarkingMenu
{
    Q_OBJECT;

public:
    /**
     * @brief Constructs a RoundMarkingMenu object at the specified global
     * position with the given parent widget.
     *
     * @param global_pos The global position where the marking menu should be created.
     * @param parent The parent widget.
     */
    RoundMarkingMenu(const QPoint& global_pos, QWidget* parent = nullptr);

protected:
    virtual QPoint get_widget_pos(uint32_t action_index, const QRect& rect) override;

private:
    int m_radius = 125;
};

OPENDCC_NAMESPACE_CLOSE