######################################################################
# Automatically generated by qmake (2.01a) Fri Dec 10 15:12:06 2010
######################################################################

TEMPLATE = lib
QT += sql xml
TARGET = qin
DEPENDPATH += . plugins
INCLUDEPATH += . plugins
LIBS += -lsunpinyin-3.0
# Turn this on to enable debugging output to the console
#DEFINES += DEBUG

# Input
RESOURCES += qin.qrc

HEADERS += \
  QinConfig.h \
  QinEngine.h \
  QinIMBases.h \
  QVirtualKeyboard.h \
  plugins/QinPinyin.h

FORMS += QVirtualKeyboard.ui

SOURCES += \
  Qin.cpp \
  QinEngine.cpp \
  QinIMBases.cpp \
  QVirtualKeyboard.cpp \
  plugins/QinPinyin.cpp \
