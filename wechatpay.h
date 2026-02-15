// wechatpay.h
#ifndef WECHATPAY_H
#define WECHATPAY_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonObject>
#include <QJsonDocument>
#include <QDateTime>
#include <QRandomGenerator>
#include <QFile>
#include <QCryptographicHash>

// OpenSSL 头文件
#include <openssl/pem.h>
#include <openssl/rsa.h>
#include <openssl/sha.h>
#include <openssl/evp.h>
#include <openssl/bio.h>
#include <openssl/err.h>

class WeChatPay : public QObject
{
    Q_OBJECT

public:
    explicit WeChatPay(QObject *parent = nullptr);
    ~WeChatPay();  // 添加析构函数用于清理 OpenSSL

    // 初始化：商户号、APPID、私钥文件路径、证书序列号
    void initialize(const QString &mchid,
                    const QString &appid,
                    const QString &privateKeyPath,
                    const QString &serialNo);

    // Native下单
    void nativeOrder(const QString &description,
                     const QString &outTradeNo,
                     int totalFee,
                     const QString &notifyUrl,
                     const QString &clientIp);

signals:
    void nativeOrderFinished(bool success, const QString &codeUrl, const QString &errorMsg);
    void errorOccurred(const QString &error);

private slots:
    void onNetworkReply(QNetworkReply *reply);

private:
    // 基础配置
    QString m_mchid;
    QString m_appid;
    QString m_serialNo;

    // 存储私钥数据（用于 OpenSSL）
    QByteArray m_privateKeyData;

    // 网络管理器
    QNetworkAccessManager *m_manager;

    // API 地址常量
    const QString API_BASE_URL = "https://api.mch.weixin.qq.com";
    const QString API_NATIVE_ORDER = "/v3/pay/transactions/native";

    // 工具函数
    QString getCurrentTimestamp();
    QString generateNonceStr();

    // 签名相关函数
    QString buildSignatureMessage(const QString &method,
                                  const QString &url,
                                  const QString &body,
                                  const QString &timestamp,
                                  const QString &nonce);

    QString signMessage(const QString &message);  // 使用 OpenSSL 签名
    QString buildToken(const QString &method,
                       const QString &url,
                       const QString &body);

    // 错误处理
    void emitError(const QString &error);
    QString getLastOpenSSLError();  // 获取 OpenSSL 错误信息
};

#endif
