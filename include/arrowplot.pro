QT += core gui widgets opengl openglwidgets
CONFIG  += c++17
TARGET   = arrowplot
TEMPLATE = app

HEADERS += include/arrowplot.hxx include/ArrowPlotWidget.hxx
SOURCES += src/collatz.cxx

unix:  LIBS += -lGL
win32: LIBS += -lopengl32
