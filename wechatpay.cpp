// wechatpay.cpp
#include "wechatpay.h"
#include <QDebug>

// OpenSSL 头文件已经在 wechatpay.h 中包含了

WeChatPay::WeChatPay(QObject *parent)
    : QObject(parent)
    , m_manager(new QNetworkAccessManager(this))
{
    connect(m_manager, &QNetworkAccessManager::finished,
            this, &WeChatPay::onNetworkReply);

    // 初始化 OpenSSL
    OpenSSL_add_all_algorithms();
    ERR_load_crypto_strings();
}

WeChatPay::~WeChatPay()
{
    // 清理 OpenSSL
    EVP_cleanup();
    ERR_free_strings();
}

void WeChatPay::initialize(const QString &mchid,
                           const QString &appid,
                           const QString &privateKeyPath,
                           const QString &serialNo)
{
    m_mchid = mchid;
    m_appid = appid;
    m_serialNo = serialNo;

    // 加载私钥文件内容
    QFile file(privateKeyPath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        emit errorOccurred("无法打开私钥文件：" + privateKeyPath);
        return;
    }

    m_privateKeyData = file.readAll();
    file.close();

    // 注意：我们不再使用 QSslKey，而是直接保存私钥数据供 OpenSSL 使用
    if (m_privateKeyData.isEmpty()) {
        emit errorOccurred("私钥文件为空");
        return;
    }

    qDebug() << "微信支付初始化成功";
    qDebug() << "商户号:" << m_mchid;
    qDebug() << "APPID:" << m_appid;
    qDebug() << "证书序列号:" << m_serialNo;
    qDebug() << "私钥长度:" << m_privateKeyData.length() << "字节";
}

QString WeChatPay::getCurrentTimestamp()
{
    return QString::number(QDateTime::currentSecsSinceEpoch());
}

QString WeChatPay::generateNonceStr()
{
    const QString possibleChars("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789");
    QString nonce;
    for (int i = 0; i < 32; ++i) {
        int index = QRandomGenerator::global()->bounded(possibleChars.length());
        nonce.append(possibleChars.at(index));
    }
    return nonce;
}

QString WeChatPay::buildSignatureMessage(const QString &method,
                                         const QString &url,
                                         const QString &body,
                                         const QString &timestamp,
                                         const QString &nonce)
{
    // 按照微信支付V3规范构建签名串
    return method + "\n" +
           url + "\n" +
           timestamp + "\n" +
           nonce + "\n" +
           body + "\n";
}

QString WeChatPay::getLastOpenSSLError()
{
    char errBuf[256];
    ERR_error_string_n(ERR_get_error(), errBuf, sizeof(errBuf));
    return QString(errBuf);
}

QString WeChatPay::signMessage(const QString &message)
{
    QString signature;
    BIO *bio = nullptr;
    EVP_PKEY *pkey = nullptr;
    EVP_MD_CTX *ctx = nullptr;

    qDebug() << "开始RSA签名...";

    do {
        // 将私钥数据转换为BIO
        bio = BIO_new_mem_buf(m_privateKeyData.constData(), m_privateKeyData.size());
        if (!bio) {
            emitError("创建BIO失败");
            break;
        }

        // 从BIO读取私钥
        pkey = PEM_read_bio_PrivateKey(bio, NULL, NULL, NULL);
        if (!pkey) {
            emitError("解析私钥失败: " + getLastOpenSSLError());
            break;
        }

        // 创建EVP_MD_CTX
        ctx = EVP_MD_CTX_new();
        if (!ctx) {
            emitError("创建EVP_MD_CTX失败");
            break;
        }

        // 初始化签名操作
        if (EVP_DigestSignInit(ctx, NULL, EVP_sha256(), NULL, pkey) <= 0) {
            emitError("初始化签名失败: " + getLastOpenSSLError());
            break;
        }

        // 更新签名数据
        QByteArray msgData = message.toUtf8();
        if (EVP_DigestSignUpdate(ctx, msgData.constData(), msgData.size()) <= 0) {
            emitError("签名更新失败: " + getLastOpenSSLError());
            break;
        }

        // 获取签名长度
        size_t sig_len = 0;
        if (EVP_DigestSignFinal(ctx, NULL, &sig_len) <= 0) {
            emitError("获取签名长度失败: " + getLastOpenSSLError());
            break;
        }

        // 执行签名
        QByteArray sigBuf(sig_len, '\0');
        if (EVP_DigestSignFinal(ctx, (unsigned char*)sigBuf.data(), &sig_len) <= 0) {
            emitError("执行签名失败: " + getLastOpenSSLError());
            break;
        }

        sigBuf.resize(sig_len);
        signature = QString::fromLatin1(sigBuf.toBase64());

        qDebug() << "RSA签名成功，长度:" << signature.length();

    } while (false);

    // 清理资源
    if (ctx) EVP_MD_CTX_free(ctx);
    if (pkey) EVP_PKEY_free(pkey);
    if (bio) BIO_free(bio);

    // 清理OpenSSL错误队列
    ERR_clear_error();

    return signature;
}

void WeChatPay::emitError(const QString &error)
{
    qDebug() << "WeChatPay错误:" << error;
    emit errorOccurred(error);
}

QString WeChatPay::buildToken(const QString &method,
                              const QString &url,
                              const QString &body)
{
    QString timestamp = getCurrentTimestamp();
    QString nonce = generateNonceStr();

    // 构建签名串
    QString message = buildSignatureMessage(method, url, body, timestamp, nonce);

    qDebug() << "\n=== 签名信息 ===";
    qDebug() << "HTTP方法:" << method;
    qDebug() << "URL:" << url;
    qDebug() << "时间戳:" << timestamp;
    qDebug() << "随机串:" << nonce;
    qDebug() << "请求体:" << body;
    qDebug() << "签名串:\n" << message;

    // 签名
    QString signature = signMessage(message);
    if (signature.isEmpty()) {
        emitError("生成签名失败");
        return QString();
    }

    qDebug() << "生成的签名(前50字符):" << signature.left(50) + "...";
    qDebug() << "证书序列号:" << m_serialNo;
    qDebug() << "商户号:" << m_mchid;
    qDebug() << "================\n";

    // 构建Authorization头
    return QString("WECHATPAY2-SHA256-RSA2048 mchid=\"%1\",nonce_str=\"%2\",timestamp=\"%3\",serial_no=\"%4\",signature=\"%5\"")
        .arg(m_mchid)
        .arg(nonce)
        .arg(timestamp)
        .arg(m_serialNo)
        .arg(signature);
}

void WeChatPay::nativeOrder(const QString &description,
                            const QString &outTradeNo,
                            int totalFee,
                            const QString &notifyUrl,
                            const QString &clientIp)
{
    // 构建请求体
    QJsonObject amountObj;
    amountObj["total"] = totalFee;
    amountObj["currency"] = "CNY";

    QJsonObject sceneObj;
    sceneObj["payer_client_ip"] = clientIp;

    QJsonObject requestBody;
    requestBody["appid"] = m_appid;
    requestBody["mchid"] = m_mchid;
    requestBody["description"] = description;
    requestBody["out_trade_no"] = outTradeNo;
    requestBody["notify_url"] = notifyUrl;
    requestBody["amount"] = amountObj;
    requestBody["scene_info"] = sceneObj;

    QJsonDocument doc(requestBody);
    QByteArray bodyData = doc.toJson(QJsonDocument::Compact);

    qDebug() << "\n=== 发送Native下单请求 ===";
    qDebug() << "请求体:" << bodyData;

    // 构建认证token
    QString token = buildToken("POST", API_NATIVE_ORDER, QString::fromUtf8(bodyData));
    if (token.isEmpty()) {
        emitError("构建认证token失败");
        return;
    }

    // 发送请求
    QNetworkRequest request;
    request.setUrl(QUrl(API_BASE_URL + API_NATIVE_ORDER));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Accept", "application/json");
    request.setRawHeader("Authorization", token.toUtf8());

    qDebug() << "Authorization:" << token.left(100) + "..."; // 只打印前100个字符
    qDebug() << "================\n";

    m_manager->post(request, bodyData);
}

void WeChatPay::onNetworkReply(QNetworkReply *reply)
{
    QByteArray responseData = reply->readAll();
    int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

    qDebug() << "\n=== 收到响应 ===";
    qDebug() << "HTTP状态码:" << statusCode;
    qDebug() << "响应数据:" << responseData;
    qDebug() << "================\n";

    if (reply->error() != QNetworkReply::NoError) {
        emit nativeOrderFinished(false, "", "网络错误: " + reply->errorString());
        reply->deleteLater();
        return;
    }

    // 解析响应
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(responseData, &parseError);

    if (parseError.error != QJsonParseError::NoError) {
        emit nativeOrderFinished(false, "", "JSON解析失败: " + parseError.errorString());
        reply->deleteLater();
        return;
    }

    QJsonObject obj = doc.object();

    if (obj.contains("code_url")) {
        QString codeUrl = obj["code_url"].toString();
        emit nativeOrderFinished(true, codeUrl, "");
    } else if (obj.contains("code") && obj.contains("message")) {
        QString errorCode = obj["code"].toString();
        QString errorMsg = obj["message"].toString();
        emit nativeOrderFinished(false, "", QString("微信支付错误: %1 - %2").arg(errorCode).arg(errorMsg));
    } else {
        emit nativeOrderFinished(false, "", "未知响应格式");
    }

    reply->deleteLater();
}
