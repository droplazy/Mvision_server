QT       += core gui  mqtt network sql websockets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17
RC_ICONS = rainbow.ico
# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    UIclass/addproduct.cpp \
    UIclass/appacount.cpp \
    UIclass/applogin2.cpp \
    UIclass/commanddev.cpp \
    UIclass/commandlsit.cpp \
    UIclass/devicelistdialog.cpp \
    UIclass/disposewithdrwaw.cpp \
    UIclass/firmware.cpp \
    UIclass/guidetextset.cpp \
    UIclass/insertdev.cpp \
    UIclass/livingcontrol.cpp \
    UIclass/mallproducts.cpp \
    UIclass/mallusermanager.cpp \
    UIclass/managerui.cpp \
    UIclass/orderlist.cpp \
    UIclass/realtimespeechrecognizer.cpp \
    UIclass/userappeal.cpp \
    UIclass/withdraw.cpp \
    databasemanager.cpp \
    emailsender.cpp \
    httpserver.cpp \
    loghandler.cpp \
    main.cpp \
    mainwindow.cpp \
    mediamtx_manager.cpp \
    mqtt_server.cpp \
    mqttclient.cpp \
    privilegehelper.cpp

HEADERS += \
    UIclass/addproduct.h \
    UIclass/appacount.h \
    UIclass/applogin2.h \
    UIclass/commanddev.h \
    UIclass/commandlsit.h \
    UIclass/devicelistdialog.h \
    UIclass/disposewithdrwaw.h \
    UIclass/firmware.h \
    UIclass/guidetextset.h \
    UIclass/insertdev.h \
    UIclass/livingcontrol.h \
    UIclass/mallproducts.h \
    UIclass/mallusermanager.h \
    UIclass/managerui.h \
    UIclass/orderlist.h \
    UIclass/realtimespeechrecognizer.h \
    UIclass/userappeal.h \
    UIclass/withdraw.h \
    databasemanager.h \
    emailsender.h \
    httpserver.h \
    loghandler.h \
    mainwindow.h \
    mediamtx_manager.h \
    mqtt_server.h \
    mqttclient.h \
    privilegehelper.h \
    publicheader.h

FORMS += \
    UIclass/appacount.ui \
    UIclass/applogin2.ui \
    UIclass/commanddev.ui \
    UIclass/disposewithdrwaw.ui \
    UIclass/livingcontrol.ui \
    UIclass/withdraw.ui \
    addproduct.ui \
    commandlsit.ui \
    devicelistdialog.ui \
    firmware.ui \
    guidetextset.ui \
    insertdev.ui \
    mainwindow.ui \
    mallproducts.ui \
    mallusermanager.ui \
    managerui.ui \
    orderlist.ui \
    userappeal.ui

TRANSLATIONS += \
    MuiltiControlSer_zh_CN.ts



INCLUDEPATH +=E:\OpenCV-4.10.0\build\include  \
              E:\OpenCV-4.10.0\build\include\opencv \
              E:\OpenCV-4.10.0\build\include\opencv2




CONFIG(debug, debug|release): {

        LIBS += E:\OpenCV-4.10.0\build\x64\mingw\lib\libopencv_*d.dll.a
        }

        else:CONFIG(release, debug|release): {

        LIBS += E:\OpenCV-4.10.0\build\x64\mingw\lib\libopencv_*.dll.a
}

# INCLUDEPATH +=D:\Program Files\mosquitto
# LIBS += D:\Program Files\mosquitto



CONFIG += lrelease
CONFIG += embed_translations

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

DISTFILES += \
    htmldoc/forget.html \
    htmldoc/lanmu.html \
    htmldoc/login.html \
    htmldoc/logsuccess.html \
    htmldoc/register.html
