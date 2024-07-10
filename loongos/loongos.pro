QT       += core gui charts

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

INCLUDEPATH += Inc \
               HMAC/Inc

SOURCES += \
    HMAC/scr/utils_hmac.c \
    HMAC/scr/utils_md5.c \
    HMAC/scr/utils_sha1.c \
    Scr/client.c \
    Scr/mqtt.c \
    Scr/multi_thread.cpp \
    Scr/server.c \
    Scr/smtp.cpp \
    main.cpp \
    mainwindow.cpp

HEADERS += \
    HMAC/Inc/utils_hmac.h \
    HMAC/Inc/utils_md5.h \
    HMAC/Inc/utils_sha1.h \
    Inc/client.h \
    Inc/mqtt.h \
    Inc/multi_thread.h \
    Inc/server.h \
    Inc/smtp.h \
    mainwindow.h

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    res.qrc
