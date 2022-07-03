# QMake Build file
# QMAKE_CC = gcc
# QMAKE_LINK_C = gcc
# QMAKE_CXX = g++
# QMAKE_LINK = g++

BDDFLAGS = $$system(gcc BFAbstractionLibrary/compilerOptionGenerator.c -o /tmp/BFAbstractionCompilerOptionsProducer-$$(USER);/tmp/BFAbstractionCompilerOptionsProducer-$$(USER))
DEFINES += USE_CUDD NDEBUG
CFLAGS += -g -fpermissive

QMAKE_LFLAGS = # -static # -fsanitize=address

QMAKE_CFLAGS_X86_64 = -arch x86_64
QMAKE_CXXFLAGS_X86_64 = -arch x86_64

QMAKE_CFLAGS_RELEASE += -g \
    $$BDDFLAGS # -fsanitize=address
QMAKE_CXXFLAGS_RELEASE += -g -std=c++14  \
    $$BDDFLAGS # -fsanitize=address
QMAKE_CFLAGS_DEBUG += -g -Wall -Wextra \
    $$BDDFLAGS
QMAKE_CXXFLAGS_DEBUG += -g -std=c++14 -Wall -Wextra \
    $$BDDFLAGS

TEMPLATE = app \
    console
CONFIG += release
CONFIG -= app_bundle
CONFIG -= qt
HEADERS += BFAbstractionLibrary/bddDump.h BFAbstractionLibrary/BF.h BFAbstractionLibrary/BFCudd.h context.hpp booleanFormulaParser.hpp

SOURCES += BFAbstractionLibrary/bddDump.cpp BFAbstractionLibrary/BFCuddVarVector.cpp BFAbstractionLibrary/BFCudd.cpp BFAbstractionLibrary/BFCuddManager.cpp \
    BFAbstractionLibrary/BFCuddVarCube.cpp main.cpp context.cpp \
    booleanFormulaParser.cpp BFAbstractionLibrary/bddDump.cpp

TARGET = boollab
INCLUDEPATH = ../lib/cudd-2.5.0/include BFAbstractionLibrary 

LIBS += -L../lib/cudd-2.5.0/cudd -L../lib/cudd-2.5.0/util -L../lib/cudd-2.5.0/mtr -L../lib/cudd-2.5.0/st -L../lib/cudd-2.5.0/dddmp -L../lib/cudd-2.5.0/epd -lcudd -ldddmp -lmtr -lepd -lst -lutil

PKGCONFIG += 
QT -= gui \
    core
