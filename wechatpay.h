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
#include <QEventLoop>
#include <QPixmap>

// OpenSSL 头文件
#include <openssl/pem.h>
#include <openssl/rsa.h>
#include <openssl/sha.h>
#include <openssl/evp.h>
#include <openssl/bio.h>
#include <openssl/err.h>

// qrencode 头文件
#include "publicheader.h"


#define APPID_WECHAT "ww57f759ba26ab8662"
#define MCHID "1106426124"
#define seriral_no "2D641F75283524A8BB2EF089E83863A041C0B3FA"



class WeChatPay : public QObject
{
    Q_OBJECT

public:
    explicit WeChatPay(QObject *parent = nullptr);
    ~WeChatPay();

    // 初始化：商户号、APPID、私钥文件路径、证书序列号
    void initialize(const QString &mchid,
                    const QString &appid,
                    const QString &privateKeyPath,
                    const QString &serialNo);

    // 同步支付接口：传入订单信息，返回二维码图片（失败返回空QPixmap）
    QPixmap requestPayment(const SQL_Order &order, const QString &notifyUrl, const QString &clientIp);

    bool closeOrder(const QString &outTradeNo);
private:
    // 基础配置
    QString m_mchid;
    QString m_appid;
    QString m_serialNo;
    QByteArray m_privateKeyData;

    // 网络管理器
    QNetworkAccessManager *m_manager;

    // API 地址常量
    const QString API_BASE_URL = "https://api.mch.weixin.qq.com";
    const QString API_NATIVE_ORDER = "/v3/pay/transactions/native";

    // 工具函数
    QString getCurrentTimestamp();
    QString generateNonceStr();
    QString buildSignatureMessage(const QString &method,
                                  const QString &url,
                                  const QString &body,
                                  const QString &timestamp,
                                  const QString &nonce);
    QString signMessage(const QString &message);
    QString buildToken(const QString &method,
                       const QString &url,
                       const QString &body);
    QString nativeOrderSync(const QString &description,
                            const QString &outTradeNo,
                            int totalFee,
                            const QString &notifyUrl,
                            const QString &clientIp);

    // 二维码生成函数
    QPixmap createQRCode(const QString &text, int size = 200);

    void emitError(const QString &error);
    QString getLastOpenSSLError();
    void nativeOrder(const QString &description, const QString &outTradeNo, int totalFee, const QString &notifyUrl, const QString &clientIp);
};

#endif
