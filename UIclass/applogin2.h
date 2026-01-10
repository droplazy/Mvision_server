#ifndef APPLOGIN2_H
#define APPLOGIN2_H

#include <QDialog>
#include <QString>
#include "DatabaseManager.h"
namespace Ui {
class applogin2;
}

class applogin2 : public QDialog
{
    Q_OBJECT

public:
    ~applogin2();

    // 获取用户输入的信息
    QString getAccount() const;
    QString getPassword() const;
    QString getVerifyCode() const;

    // 6. 生成指令ID
    QString  m_commandId;

    applogin2(const QString &deviceSerial, const QString &platformName, const QString &currentStatus, const QString &commandId, DatabaseManager *p_db, QWidget *parent);
signals:
    void sendJsonToMQTT(const QString& deviceSerial,
                        const QString& JSON);

public slots:

    void onLoginResultReceived(const QString& commandId, const QString& status, const QString& message);
    void onCRcodeImgReceived(const QString& commandId);
private slots:
    void on_pushButton_loggin_clicked();
    void on_pushButton_sendcode_clicked();

    void on_pushButton_CRcode_clicked();

private:
    Ui::applogin2 *ui;

    QString m_deviceSerial;    // 设备号
    QString m_platformName;    // 平台名称
    QString m_currentStatus;   // 当前状态
    QString m_account;         // 保存账号，用于结果显示
  //  QTimer* m_qrcodeCheckTimer;  // 新增：检查二维码的定时器
    DatabaseManager* m_dbManager;  // 保存数据库指针

    // 初始化UI
    void initUI();
    QString getCurrentTimestamp() const;

    // 生成JSON的辅助函数
    QString generateSendCodeJson(const QString& account);
    QString generateLoginJson(const QString& account, const QString& verifyCode);
    void insertCommandFromJson(const QString &jsonString);
    QString generateLoginCRcode();

    // 二维码相关方法
    QString checkAndDisplayQRCode();
    void displayQRCodeImage(const QString& imagePath);
    void cleanupQRCodeResources();
};

#endif // APPLOGIN2_H
