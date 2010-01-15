include(parser/parser.pri)

DEPENDPATH += $$PWD $$PWD/metatype
INCLUDEPATH += $$PWD/..

HEADERS += \
    $$PWD/qml_global.h \
    $$PWD/qmlidcollector.h \
    $$PWD/qmldocument.h \
    $$PWD/qmlpackageinfo.h \
    $$PWD/qmlsymbol.h \
    $$PWD/QmlMetaTypeBackend.h \
    $$PWD/qmltypesystem.h

SOURCES += \
    $$PWD/qmlidcollector.cpp \
    $$PWD/qmldocument.cpp \
    $$PWD/qmlsymbol.cpp \
    $$PWD/qmlpackageinfo.cpp \
    $$PWD/QmlMetaTypeBackend.cpp \
    $$PWD/qmltypesystem.cpp

contains(QT_CONFIG, declarative) {
    QT += declarative

    DEFINES += BUILD_DECLARATIVE_BACKEND

    HEADERS += \
        $$PWD/QtDeclarativeMetaTypeBackend.h

    SOURCES += \
        $$PWD/QtDeclarativeMetaTypeBackend.cpp
}
