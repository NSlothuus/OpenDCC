#!/usr/bin/env python3
"""
Script to update Qt includes from old style (#include <QClass>) 
to Qt6 style with module prefixes (#include <QtModule/QClass>)
"""

import os
import re
import glob

# Mapping of Qt classes to their modules
QT_CLASS_TO_MODULE = {
    # QtCore
    'QCoreApplication': 'QtCore',
    'QObject': 'QtCore',
    'QTimer': 'QtCore',
    'QRunnable': 'QtCore',
    'QThreadPool': 'QtCore',
    'QDateTime': 'QtCore',
    'QDir': 'QtCore',
    'QFile': 'QtCore',
    'QFileInfo': 'QtCore',
    'QUrl': 'QtCore',
    'QSettings': 'QtCore',
    'QMetaType': 'QtCore',
    'QDebug': 'QtCore',
    'QRegularExpression': 'QtCore',
    'QVector': 'QtCore',
    'QHash': 'QtCore',
    'QSet': 'QtCore',
    'QString': 'QtCore',
    'QPoint': 'QtCore',
    'QSize': 'QtCore',
    'QRect': 'QtCore',
    'QMap': 'QtCore',
    'QStringBuilder': 'QtCore',
    
    # QtWidgets
    'QApplication': 'QtWidgets',
    'QWidget': 'QtWidgets',
    'QMainWindow': 'QtWidgets',
    'QLabel': 'QtWidgets',
    'QMessageBox': 'QtWidgets',
    'QPushButton': 'QtWidgets',
    'QTreeWidget': 'QtWidgets',
    'QMenu': 'QtWidgets',
    'QComboBox': 'QtWidgets',
    'QTabWidget': 'QtWidgets',
    'QFileDialog': 'QtWidgets',
    'QGridLayout': 'QtWidgets',
    'QVBoxLayout': 'QtWidgets',
    'QHBoxLayout': 'QtWidgets',
    'QBoxLayout': 'QtWidgets',
    'QHeaderView': 'QtWidgets',
    'QTableWidget': 'QtWidgets',
    'QSpinBox': 'QtWidgets',
    'QDoubleSpinBox': 'QtWidgets',
    'QLineEdit': 'QtWidgets',
    'QCheckBox': 'QtWidgets',
    'QTableView': 'QtWidgets',
    'QToolButton': 'QtWidgets',
    'QSlider': 'QtWidgets',
    'QPlainTextEdit': 'QtWidgets',
    'QAction': 'QtWidgets',
    'QActionGroup': 'QtWidgets',
    'QMenuBar': 'QtWidgets',
    'QStyledItemDelegate': 'QtWidgets',
    'QAbstractTableModel': 'QtWidgets',
    'QScrollBar': 'QtWidgets',
    'QSortFilterProxyModel': 'QtWidgets',
    'QLayout': 'QtWidgets',
    'QShortcut': 'QtWidgets',
    'QDoubleValidator': 'QtWidgets',
    'QToolTip': 'QtWidgets',
    'QStyleOption': 'QtWidgets',
    'QStyle': 'QtWidgets',
    
    # QtGui
    'QImage': 'QtGui',
    'QPainter': 'QtGui',
    'QOffscreenSurface': 'QtGui',
    'QMimeData': 'QtGui',
    'QPainterPath': 'QtGui',
    'QGuiApplication': 'QtGui',
    'QEvent': 'QtGui',
    'QContextMenuEvent': 'QtGui',
    'QMouseEvent': 'QtGui',
    'QKeySequence': 'QtGui',
    'QTranslator': 'QtGui',
    'QPixmap': 'QtGui',
    'QIcon': 'QtGui',
    'QFont': 'QtGui',
    'QFontMetrics': 'QtGui',
    'QPen': 'QtGui',
    'QBrush': 'QtGui',
    'QPaintDevice': 'QtGui',
    'QTextOption': 'QtGui',
    'QColor': 'QtGui',
    'QPalette': 'QtGui',
    'QWindow': 'QtGui',
    
    # QtOpenGL
    'QOpenGLContext': 'QtOpenGL',
    'QOpenGLWidget': 'QtOpenGL',
    
    # QtMultimedia
    'QMediaPlayer': 'QtMultimedia',
    'QAudioDecoder': 'QtMultimedia',
}

def update_qt_includes_in_file(filepath):
    """Update Qt includes in a single file."""
    try:
        with open(filepath, 'r', encoding='utf-8') as f:
            content = f.read()
        
        original_content = content
        updated = False
        
        # Pattern to match Qt includes: #include <QSomething>
        pattern = r'#include\s*<(Q[A-Z][a-zA-Z0-9_]*)\s*>'
        
        def replace_include(match):
            nonlocal updated
            qt_class = match.group(1)
            if qt_class in QT_CLASS_TO_MODULE:
                module = QT_CLASS_TO_MODULE[qt_class]
                updated = True
                return f'#include <{module}/{qt_class}>'
            else:
                # Keep unknown Qt classes as-is for now
                return match.group(0)
        
        content = re.sub(pattern, replace_include, content)
        
        if updated:
            with open(filepath, 'w', encoding='utf-8') as f:
                f.write(content)
            print(f"Updated: {filepath}")
            return True
        
    except Exception as e:
        print(f"Error processing {filepath}: {e}")
        return False
    
    return False

def main():
    """Main function to update all Qt includes."""
    # Get all .cpp and .h files
    cpp_files = []
    h_files = []
    
    # Search in src directory
    for root, dirs, files in os.walk('src'):
        for file in files:
            filepath = os.path.join(root, file)
            if file.endswith('.cpp'):
                cpp_files.append(filepath)
            elif file.endswith('.h'):
                h_files.append(filepath)
    
    print(f"Found {len(cpp_files)} .cpp files and {len(h_files)} .h files")
    
    updated_files = []
    
    # Update .cpp files
    print("Updating .cpp files...")
    for filepath in cpp_files:
        if update_qt_includes_in_file(filepath):
            updated_files.append(filepath)
    
    # Update .h files
    print("Updating .h files...")
    for filepath in h_files:
        if update_qt_includes_in_file(filepath):
            updated_files.append(filepath)
    
    print(f"\nUpdated {len(updated_files)} files total:")
    for f in updated_files:
        print(f"  {f}")

if __name__ == '__main__':
    main()