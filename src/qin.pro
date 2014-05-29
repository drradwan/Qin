######################################################################
# Automatically generated by qmake (2.01a) Fri Dec 10 15:12:06 2010
######################################################################

TEMPLATE = lib
QT += sql xml
TARGET = qin
DEPENDPATH += . plugins
INCLUDEPATH += . plugins
LIBS += -lchewing -lsunpinyin-3.0

# Input
RESOURCES += qin.qrc

HEADERS += \
  QinConfig.h \
  QinEngine.h \
  QinIMBases.h \
  QVirtualKeyboard.h \
  plugins/QinChewing.h \
  plugins/QinPinyin.h

FORMS += QVirtualKeyboard.ui

SOURCES += \
  Qin.cpp \
  QinEngine.cpp \
  QinIMBases.cpp \
  QVirtualKeyboard.cpp \
  plugins/QinChewing.cpp \
  plugins/QinPinyin.cpp \