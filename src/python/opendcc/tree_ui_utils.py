# Copyright Contributors to the OpenDCC project
# SPDX-License-Identifier: Apache-2.0

from Qt import QtGui, QtCore, QtWidgets


class HeaderVisibilityMenu(QtWidgets.QMenu):
    def __init__(self, parent):
        QtWidgets.QMenu.__init__(self, parent=parent)
        self.view = parent

        self._permanent_columns = set()

        self.aboutToShow.connect(self.update_actions_list)

    def update_actions_list(self):
        self.clear()

        header = self.view.header()
        for column_index in range(self.view.header().count()):
            if column_index in self._permanent_columns:
                continue
            title = self.view.model().headerData(
                column_index, QtCore.Qt.Horizontal, QtCore.Qt.DisplayRole
            )
            if title == "Vis":  # I'm not happy about that either
                title = "Visibility"
            icon = self.view.model().headerData(
                column_index, QtCore.Qt.Horizontal, QtCore.Qt.DecorationRole
            )

            is_hidden = header.isSectionHidden(column_index)
            func = (lambda arg: lambda checked_flag: self.change_state(checked_flag, arg))(
                column_index
            )
            self.add_action(title, QtGui.QIcon(icon), not is_hidden, func)

    def add_action(self, name, icon, checked, func):
        action = QtWidgets.QWidgetAction(self)
        checkBox = QtWidgets.QCheckBox(name, self)
        checkBox.setIcon(icon)
        action.setDefaultWidget(checkBox)
        checkBox.setChecked(checked)
        checkBox.toggled.connect(func)
        self.addAction(action)
        return action

    def change_state(self, checked, column_index):
        if checked:
            self.view.showColumn(column_index)
        if not checked:
            self.view.hideColumn(column_index)

    def set_column_permanent(self, index, flag):
        if flag:
            self._permanent_columns.add(index)
        else:
            if index in self._permanent_columns:
                self._permanent_columns.remove(index)


def add_branches(initial_level):
    """Adds drawBranches method to class"""

    def wrapper(cls):
        pen_branch = QtGui.QPen(QtGui.QBrush(QtGui.QColor(130, 130, 130)), 1)
        pen_branch_hilite = QtGui.QPen(QtGui.QBrush(QtGui.QColor(146, 166, 179)), 1)
        pen_dropindicator = QtGui.QPen(QtGui.QBrush(QtGui.QColor(82, 133, 166)), 2)

        def drawBranches(self, painter, rect, index):
            """Draw the branch indicator of a row"""

            # Get the indentation level of the row
            level = initial_level  # yep
            tmpIndex = index.parent()
            while tmpIndex.isValid():
                level += 1
                tmpIndex = tmpIndex.parent()

            # Is the row highlighted (selected) ?
            isHighlight = self.selectionModel().isSelected(index)

            # Line width
            lineWidth = 1

            # Cell width
            cellWidth = int(rect.width() / (level + 1))

            # Current cell to draw in
            cellX = rect.x() + cellWidth * level
            cellY = rect.y()
            cellW = cellWidth
            cellH = rect.height()

            # Center of the cell
            centerX = cellX + int(cellW / 2) - int(lineWidth / 2)
            centerY = cellY + int(cellH / 2) - int(lineWidth / 2)

            # Backup the old pen
            oldPen = painter.pen()

            # Draw the branch indicator on the right most
            if self.model().hasChildren(index):
                # Branch icon properties
                rectRadius = 4
                crossMargin = 1

                # Is the row expanded ?
                isExpanded = self.isExpanded(index)

                # [+] and [-] are using different color when highlighted
                painter.setPen(self.pen_branch_hilite if isHighlight else self.pen_branch)

                # Draw a rectangle [ ] as the branch indicator
                painter.drawRect(
                    centerX - rectRadius, centerY - rectRadius, rectRadius * 2, rectRadius * 2
                )

                # Draw the '-' into the rectangle. i.e. [-]
                painter.drawLine(
                    centerX - rectRadius + crossMargin + lineWidth,
                    centerY,
                    centerX + rectRadius - crossMargin - lineWidth,
                    centerY,
                )

                # Draw the '|' into the rectangle. i.e. [+]
                if not isExpanded:
                    painter.drawLine(
                        centerX,
                        centerY - rectRadius + crossMargin + lineWidth,
                        centerX,
                        centerY + rectRadius - crossMargin - lineWidth,
                    )

                # Other ornaments are not highlighted
                painter.setPen(self.pen_branch)

                # Draw the '|' on the bottom. i.e. [-]
                #                                   |
                if isExpanded:
                    painter.drawLine(
                        centerX,
                        centerY + rectRadius + crossMargin + lineWidth,
                        centerX,
                        cellY + cellH,
                    )

                # Draw more ornaments when the row is not a top level row
                if level > 0:
                    # Draw the '-' on the left. i.e. --[+]
                    painter.drawLine(
                        cellX, centerY, centerX - rectRadius - crossMargin - lineWidth, centerY
                    )
            else:
                # Circle is not highlighted
                painter.setPen(self.pen_branch)

                # Draw the line and circle. i.e. --o
                if level > 0:
                    painter.drawLine(cellX, centerY, centerX, centerY)

                    # Backup the old brush
                    oldBrush = painter.brush()
                    painter.setBrush(self.pen_branch.brush())

                    # A filled circle
                    circleRadius = 2
                    painter.drawEllipse(
                        centerX - circleRadius,
                        centerY - circleRadius,
                        circleRadius * 2,
                        circleRadius * 2,
                    )

                    # Restore the old brush
                    painter.setBrush(oldBrush)

            # Draw other vertical and horizental lines on the left of the indicator
            if level > 0:
                # Move cell window to the left
                cellX -= cellWidth
                centerX -= cellWidth

                if index.sibling(index.row() + 1, index.column()).isValid():
                    # The row has more siblings. i.e. |
                    #                                 |--
                    #                                 |
                    painter.drawLine(centerX, cellY, centerX, cellY + cellH)
                    painter.drawLine(centerX, centerY, cellX + cellW, centerY)
                else:
                    # The row is the last row.   i.e. |
                    #                                 L--
                    painter.drawLine(centerX, cellY, centerX, centerY)
                    painter.drawLine(centerX, centerY, cellX + cellW, centerY)

                # More vertical lines on the left. i.e. ||||-
                tmpIndex = index.parent()
                for i in range(0, level - 1):
                    # Move the cell window to the left
                    cellX -= cellWidth
                    centerX -= cellWidth

                    # Draw vertical line if the row has silbings at this level
                    if tmpIndex.sibling(tmpIndex.row() + 1, tmpIndex.column()).isValid():
                        painter.drawLine(centerX, cellY, centerX, cellY + cellH)
                    tmpIndex = tmpIndex.parent()

            # Restore the old pen
            painter.setPen(oldPen)

        setattr(cls, "drawBranches", drawBranches)
        setattr(cls, "pen_branch", pen_branch)
        setattr(cls, "pen_branch_hilite", pen_branch_hilite)

        return cls

    return wrapper
