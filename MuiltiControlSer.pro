QT       += core gui  mqtt network sql websockets

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17
RC_ICONS = rainbow.ico
# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0
win32 {
    # OpenSSL 配置
    INCLUDEPATH += "C:/Program Files/OpenSSL/include"
    LIBS += -L"C:/Program Files/OpenSSL/lib" -llibcrypto -llibssl

    # qrcode 配置
    INCLUDEPATH += "C:/Program Files/qrcode/include"
    LIBS += -L"C:/Program Files/qrcode/lib" -lqrencode

    # 添加 Windows 系统库（解决 __imp_WSAStartup 错误）
    LIBS += -lws2_32 -luser32 -lcrypt32 -ladvapi32
}
SOURCES += \
    QRCode/bitstream.c \
    QRCode/mask.c \
    QRCode/mmask.c \
    QRCode/mqrspec.c \
    QRCode/qrencode.c \
    QRCode/qrinput.c \
    QRCode/qrspec.c \
    QRCode/rscode.c \
    QRCode/split.c \
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
    UIclass/xfocr.cpp \
    ai_bragger.cpp \
    databasemanager.cpp \
    deepseekai.cpp \
    emailsender.cpp \
    httpserver.cpp \
    loghandler.cpp \
    main.cpp \
    mainwindow.cpp \
    mediamtx_manager.cpp \
    mqtt_server.cpp \
    mqttclient.cpp \
    privilegehelper.cpp \
    simplexfai.cpp \
    wechatpay.cpp

HEADERS += \
    QRCode/bitstream.h \
    QRCode/config.h \
    QRCode/mask.h \
    QRCode/mmask.h \
    QRCode/mqrspec.h \
    QRCode/qrencode.h \
    QRCode/qrencode_inner.h \
    QRCode/qrinput.h \
    QRCode/qrspec.h \
    QRCode/rscode.h \
    QRCode/split.h \
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
    UIclass/xfocr.h \
    ai_bragger.h \
    databasemanager.h \
    deepseekai.h \
    emailsender.h \
    httpserver.h \
    loghandler.h \
    mainwindow.h \
    mediamtx_manager.h \
    mqtt_server.h \
    mqttclient.h \
    privilegehelper.h \
    publicheader.h \
    simplexfai.h \
    wechatpay.h

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
    QRCode/QRCode.pri \
    htmldoc/forget.html \
    htmldoc/lanmu.html \
    htmldoc/login.html \
    htmldoc/logsuccess.html \
    htmldoc/register.html



