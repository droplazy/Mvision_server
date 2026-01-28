#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include <QMainWindow>
#include "mqttclient.h"
#include "httpserver.h"
#include "mqtt_server.h"
#include "emailsender.h"
#include <QSystemTrayIcon>
#include <QCloseEvent>
#include "mediamtx_manager.h"
#include "UIclass/realtimespeechrecognizer.h"
#include "./UIclass/livingcontrol.h"
#include "ai_bragger.h"
#include "simplexfai.h"

namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
protected:
    void closeEvent(QCloseEvent *event) override;
public slots:
    void DispayMqttclientStatus(bool status);
    void updateNewrqeInfo(QString info);
private slots:
    void on_pushButton_devlist_clicked();
    void on_pushButton_cmdquery_clicked();
    void on_pushButton_firmware_clicked();
    void on_pushButton_malluser_clicked();
    void on_pushButton_products_clicked();
    void on_pushButton_order_clicked();
    void on_pushButton_appeal_clicked();
    void on_pushButton_webUI_clicked();
    void on_pushButton_openmqtt_clicked();
    void updateHttpDeviceContainer();
    void on_pushButton_contorluser_clicked();
    void on_pushButton_cmddispatch_clicked();
    void on_pushButton_appcount_clicked();

    void on_pushButton_withdraw_clicked();

    void on_pushButton_clicked();

    void on_pushButton_livingcontrol_clicked();

    void onOneSecondTimerTimeout();

    void checkAndStartProgramSpeechRecognition();
private:
    Ui::MainWindow *ui;

    SimpleXFAI ai;
    QString IP;
    QString MQTT_PORT;
    QString HTTP_PORT;
    class DatabaseManager *p_db;
    class HttpServer *p_http;
    class mqttclient *p_mqtt_cli;
    class MQTT_server *p_mqttt_ser;
    MediaMTX_Manager manager;
    livingcontrol *livingControlWindow; //todo
    QTimer *m_oneSecondTimer;  // 1秒定时器
    AI_bragger *p_ai;
    QMap<QString, RealtimeSpeechRecognizer*> m_programSpeechRecognizers;  // 节目语音识别器映射
    class EmailSender *p_email;
    QSystemTrayIcon *trayIcon;
    QMenu *trayMenu;
    QAction *restoreAction;
    QAction *quitAction;
    void updateSystemTime();
  //  void initDatabaseAndServices();
    void initUIStatus();
    void initDatabase();
    void startMqttClient(const QString &ip, int port);
    void initEmailService();
    void connectHttpToMqtt();
    void stopMqttServices();
    bool isMqttServerRunning();
    bool validateNetworkConfig();
    bool checkMqttServerRunning();
    void connectEmailSignals();
    void disconnectHttpConnections();
    void disconnectMqttClientSignals();
    void connectHttpSignals();
    void enableNetworkControls(bool enable);
    bool readSmtpConfig(QString &email, QString &password);
    void onQuitAction();
    void onRestoreAction();
    void onTrayIconActivated(QSystemTrayIcon::ActivationReason reason);
    void createTrayMenu();
    void createTrayIcon();
    void changeEvent(QEvent *event) override;



    void createLivingControl();
    void switchToView(QWidget *view);

    void updateProgramVoiceText(const QString &commandId, const QString &text);
    void xunfeiAIprase(const AIpost &aiPost);
    void updateProgramBragger(const QString &commandId, const QString &comments);
    QStringList parseBracketComments(const QString &response);
    void checkAndGenerateBragger();
    void resetProgramGenerating(const QString &commandId);
};
#endif // MAINWINDOW_H
