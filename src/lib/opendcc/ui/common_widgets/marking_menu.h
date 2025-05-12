/*
 * Copyright Contributors to the OpenDCC project
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once
#include "opendcc/opendcc.h"
#include "opendcc/ui/common_widgets/api.h"
#include <QWidget>
#include <QMenu>
#include <stack>
#include <unordered_map>

OPENDCC_NAMESPACE_OPEN

/**
 * @brief The MarkingMenu class represents a custom widget
 * for displaying a marking menu and provides functionality
 * for setting an extended menu and handling mouse movement.
 */
class OPENDCC_UI_COMMON_WIDGETS_API MarkingMenu : public QWidget
{
    Q_OBJECT;

public:
    /**
     * @brief Constructs a MarkingMenu object with the specified
     * global position and parent widget.
     *
     * @param global_pos The global position of the marking menu.
     * @param parent The parent widget.
     */
    MarkingMenu(const QPoint& global_pos, QWidget* parent = nullptr);
    virtual ~MarkingMenu() = default;

    /**
     * @brief Sets the extended menu for the marking menu.
     *
     * @param menu The extended menu to set.
     */
    void set_extended_menu(QMenu* menu);
    /**
     * @brief Handles mouse movement for the marking menu.
     *
     * @param global_pos The global position of the mouse.
     */
    void on_mouse_move(const QPoint& global_pos);

    /**
     * @brief Gets the currently hovered action in the marking menu.
     *
     * @return The hovered QAction object.
     */
    QAction* get_hovered_action() const { return m_hovered_action; }

protected:
    virtual QPoint get_widget_pos(uint32_t action_index, const QRect& rect) = 0;
    virtual void paintEvent(QPaintEvent* event) override;

    struct ItemInfo
    {
        QRect rect;
        QAction* action;
    };

    std::stack<QMenu*> m_menu_stack;
    std::unordered_map<QMenu*, std::vector<ItemInfo>> m_actions;
    QPoint m_mouse_pos;
    QAction* m_hovered_action = nullptr;

private:
    std::vector<QPointF> m_polyline;
    bool m_can_go_back = false;
};

OPENDCC_NAMESPACE_CLOSE
