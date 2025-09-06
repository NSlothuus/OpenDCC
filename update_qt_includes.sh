#!/bin/bash

# Script to update Qt includes from old style to Qt6 style with module prefixes

# Function to update Qt includes in a file
update_qt_includes() {
    local file="$1"
    echo "Updating: $file"
    
    # QtCore includes
    sed -i 's|#include <QCoreApplication>|#include <QtCore/QCoreApplication>|g' "$file"
    sed -i 's|#include <QObject>|#include <QtCore/QObject>|g' "$file"
    sed -i 's|#include <QTimer>|#include <QtCore/QTimer>|g' "$file"
    sed -i 's|#include <QRunnable>|#include <QtCore/QRunnable>|g' "$file"
    sed -i 's|#include <QThreadPool>|#include <QtCore/QThreadPool>|g' "$file"
    sed -i 's|#include <QDateTime>|#include <QtCore/QDateTime>|g' "$file"
    sed -i 's|#include <QDir>|#include <QtCore/QDir>|g' "$file"
    sed -i 's|#include <QFile>|#include <QtCore/QFile>|g' "$file"
    sed -i 's|#include <QFileInfo>|#include <QtCore/QFileInfo>|g' "$file"
    sed -i 's|#include <QUrl>|#include <QtCore/QUrl>|g' "$file"
    sed -i 's|#include <QSettings>|#include <QtCore/QSettings>|g' "$file"
    sed -i 's|#include <QMetaType>|#include <QtCore/QMetaType>|g' "$file"
    sed -i 's|#include <QDebug>|#include <QtCore/QDebug>|g' "$file"
    sed -i 's|#include <QRegularExpression>|#include <QtCore/QRegularExpression>|g' "$file"
    sed -i 's|#include <QVector>|#include <QtCore/QVector>|g' "$file"
    sed -i 's|#include <QHash>|#include <QtCore/QHash>|g' "$file"
    sed -i 's|#include <QSet>|#include <QtCore/QSet>|g' "$file"
    sed -i 's|#include <QString>|#include <QtCore/QString>|g' "$file"
    sed -i 's|#include <QPoint>|#include <QtCore/QPoint>|g' "$file"
    sed -i 's|#include <QSize>|#include <QtCore/QSize>|g' "$file"
    sed -i 's|#include <QRect>|#include <QtCore/QRect>|g' "$file"
    sed -i 's|#include <QMap>|#include <QtCore/QMap>|g' "$file"
    sed -i 's|#include <QStringBuilder>|#include <QtCore/QStringBuilder>|g' "$file"
    
    # QtWidgets includes
    sed -i 's|#include <QApplication>|#include <QtWidgets/QApplication>|g' "$file"
    sed -i 's|#include <QWidget>|#include <QtWidgets/QWidget>|g' "$file"
    sed -i 's|#include <QMainWindow>|#include <QtWidgets/QMainWindow>|g' "$file"
    sed -i 's|#include <QLabel>|#include <QtWidgets/QLabel>|g' "$file"
    sed -i 's|#include <QMessageBox>|#include <QtWidgets/QMessageBox>|g' "$file"
    sed -i 's|#include <QPushButton>|#include <QtWidgets/QPushButton>|g' "$file"
    sed -i 's|#include <QTreeWidget>|#include <QtWidgets/QTreeWidget>|g' "$file"
    sed -i 's|#include <QMenu>|#include <QtWidgets/QMenu>|g' "$file"
    sed -i 's|#include <QComboBox>|#include <QtWidgets/QComboBox>|g' "$file"
    sed -i 's|#include <QTabWidget>|#include <QtWidgets/QTabWidget>|g' "$file"
    sed -i 's|#include <QFileDialog>|#include <QtWidgets/QFileDialog>|g' "$file"
    sed -i 's|#include <QGridLayout>|#include <QtWidgets/QGridLayout>|g' "$file"
    sed -i 's|#include <QVBoxLayout>|#include <QtWidgets/QVBoxLayout>|g' "$file"
    sed -i 's|#include <QHBoxLayout>|#include <QtWidgets/QHBoxLayout>|g' "$file"
    sed -i 's|#include <QBoxLayout>|#include <QtWidgets/QBoxLayout>|g' "$file"
    sed -i 's|#include <QHeaderView>|#include <QtWidgets/QHeaderView>|g' "$file"
    sed -i 's|#include <QTableWidget>|#include <QtWidgets/QTableWidget>|g' "$file"
    sed -i 's|#include <QSpinBox>|#include <QtWidgets/QSpinBox>|g' "$file"
    sed -i 's|#include <QDoubleSpinBox>|#include <QtWidgets/QDoubleSpinBox>|g' "$file"
    sed -i 's|#include <QLineEdit>|#include <QtWidgets/QLineEdit>|g' "$file"
    sed -i 's|#include <QCheckBox>|#include <QtWidgets/QCheckBox>|g' "$file"
    sed -i 's|#include <QTableView>|#include <QtWidgets/QTableView>|g' "$file"
    sed -i 's|#include <QToolButton>|#include <QtWidgets/QToolButton>|g' "$file"
    sed -i 's|#include <QSlider>|#include <QtWidgets/QSlider>|g' "$file"
    sed -i 's|#include <QPlainTextEdit>|#include <QtWidgets/QPlainTextEdit>|g' "$file"
    sed -i 's|#include <QAction>|#include <QtWidgets/QAction>|g' "$file"
    sed -i 's|#include <QActionGroup>|#include <QtWidgets/QActionGroup>|g' "$file"
    sed -i 's|#include <QMenuBar>|#include <QtWidgets/QMenuBar>|g' "$file"
    sed -i 's|#include <QStyledItemDelegate>|#include <QtWidgets/QStyledItemDelegate>|g' "$file"
    sed -i 's|#include <QAbstractTableModel>|#include <QtCore/QAbstractTableModel>|g' "$file"
    sed -i 's|#include <QScrollBar>|#include <QtWidgets/QScrollBar>|g' "$file"
    sed -i 's|#include <QSortFilterProxyModel>|#include <QtCore/QSortFilterProxyModel>|g' "$file"
    sed -i 's|#include <QLayout>|#include <QtWidgets/QLayout>|g' "$file"
    sed -i 's|#include <QShortcut>|#include <QtWidgets/QShortcut>|g' "$file"
    sed -i 's|#include <QDoubleValidator>|#include <QtGui/QDoubleValidator>|g' "$file"
    sed -i 's|#include <QToolTip>|#include <QtWidgets/QToolTip>|g' "$file"
    sed -i 's|#include <QStyleOption>|#include <QtWidgets/QStyleOption>|g' "$file"
    sed -i 's|#include <QStyle>|#include <QtWidgets/QStyle>|g' "$file"
    
    # QtGui includes
    sed -i 's|#include <QImage>|#include <QtGui/QImage>|g' "$file"
    sed -i 's|#include <QPainter>|#include <QtGui/QPainter>|g' "$file"
    sed -i 's|#include <QOffscreenSurface>|#include <QtGui/QOffscreenSurface>|g' "$file"
    sed -i 's|#include <QMimeData>|#include <QtCore/QMimeData>|g' "$file"
    sed -i 's|#include <QPainterPath>|#include <QtGui/QPainterPath>|g' "$file"
    sed -i 's|#include <QGuiApplication>|#include <QtGui/QGuiApplication>|g' "$file"
    sed -i 's|#include <QEvent>|#include <QtCore/QEvent>|g' "$file"
    sed -i 's|#include <QContextMenuEvent>|#include <QtGui/QContextMenuEvent>|g' "$file"
    sed -i 's|#include <QMouseEvent>|#include <QtGui/QMouseEvent>|g' "$file"
    sed -i 's|#include <QKeySequence>|#include <QtGui/QKeySequence>|g' "$file"
    sed -i 's|#include <QTranslator>|#include <QtCore/QTranslator>|g' "$file"
    sed -i 's|#include <QPixmap>|#include <QtGui/QPixmap>|g' "$file"
    sed -i 's|#include <QIcon>|#include <QtGui/QIcon>|g' "$file"
    sed -i 's|#include <QFont>|#include <QtGui/QFont>|g' "$file"
    sed -i 's|#include <QFontMetrics>|#include <QtGui/QFontMetrics>|g' "$file"
    sed -i 's|#include <QPen>|#include <QtGui/QPen>|g' "$file"
    sed -i 's|#include <QBrush>|#include <QtGui/QBrush>|g' "$file"
    sed -i 's|#include <QPaintDevice>|#include <QtGui/QPaintDevice>|g' "$file"
    sed -i 's|#include <QTextOption>|#include <QtGui/QTextOption>|g' "$file"
    sed -i 's|#include <QColor>|#include <QtGui/QColor>|g' "$file"
    sed -i 's|#include <QPalette>|#include <QtGui/QPalette>|g' "$file"
    sed -i 's|#include <QWindow>|#include <QtGui/QWindow>|g' "$file"
    
    # QtOpenGL includes
    sed -i 's|#include <QOpenGLContext>|#include <QtOpenGL/QOpenGLContext>|g' "$file"
    sed -i 's|#include <QOpenGLWidget>|#include <QtOpenGL/QOpenGLWidget>|g' "$file"
    
    # QtMultimedia includes
    sed -i 's|#include <QMediaPlayer>|#include <QtMultimedia/QMediaPlayer>|g' "$file"
    sed -i 's|#include <QAudioDecoder>|#include <QtMultimedia/QAudioDecoder>|g' "$file"
}

# Find all .cpp and .h files and update them
echo "Updating Qt includes in all C++ files..."

find src -name "*.cpp" -o -name "*.h" | while read -r file; do
    if grep -q '#include <Q[A-Z]' "$file"; then
        update_qt_includes "$file"
    fi
done

echo "Done updating Qt includes!"