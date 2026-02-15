// wechatpay.cpp
#include "wechatpay.h"
#include <QDebug>
#include <qrencode.h>

#include <QEventLoop>

WeChatPay::WeChatPay(QObject *parent)
    : QObject(parent)
    , m_manager(new QNetworkAccessManager(this))
{
    // 初始化 OpenSSL
    OpenSSL_add_all_algorithms();
    ERR_load_crypto_strings();

    initialize(
        MCHID,
        APPID_WECHAT,
        //"C:/Windows/SysWOW64/WXCertUtil/cert/1106426124_20260215_cert/apiclient_key.pem",
        "./apiclient_key.pem",
        seriral_no
        );
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
        return;
    }

    m_privateKeyData = file.readAll();
    file.close();

    if (m_privateKeyData.isEmpty()) {
        return;
    }

    qDebug() << "微信支付初始化成功";
    qDebug() << "商户号:" << m_mchid;
    qDebug() << "APPID:" << m_appid;
    qDebug() << "证书序列号:" << m_serialNo;
}
QPixmap WeChatPay::requestPayment(const SQL_Order &order, const QString &notifyUrl, const QString &clientIp)
{
    // 使用订单信息生成支付描述
    QString description = order.productName;
    if (description.isEmpty()) {
        description = QString("商品订单-%1").arg(order.orderId);
    }

    // 金额转换为分
    int totalFee = static_cast<int>(order.totalPrice * 100);

    // 调用同步支付接口获取二维码链接
    QString codeUrl = nativeOrderSync(description, order.orderId, totalFee, notifyUrl, clientIp);

    if (codeUrl.isEmpty()) {
        return QPixmap();  // 返回空图片
    }

    // 生成二维码图片
    return createQRCode(codeUrl, 250);
}
QPixmap WeChatPay::createQRCode(const QString &text, int size)
{
    if (text.isEmpty()) {
        return QPixmap();
    }

    // 生成二维码数据
    QRcode *qrcode = QRcode_encodeString(text.toUtf8().constData(),
                                         2,  // 版本
                                         QR_ECLEVEL_L,  // 纠错级别
                                         QR_MODE_8,     // 编码模式
                                         0);            // 区分大小写

    if (!qrcode) {
        qDebug() << "二维码生成失败";
        return QPixmap();
    }

    // 创建图像
    int width = qrcode->width;
    QImage image(width, width, QImage::Format_Mono);

    // 设置颜色表
    image.setColorCount(2);
    image.setColor(0, qRgb(255, 255, 255));  // 白色
    image.setColor(1, qRgb(0, 0, 0));        // 黑色

    // 填充白色背景
    image.fill(0);

    // 绘制二维码
    unsigned char *data = qrcode->data;
    for (int y = 0; y < width; y++) {
        for (int x = 0; x < width; x++) {
            if (*data & 1) {
                image.setPixel(x, y, 1);  // 黑色
            }
            data++;
        }
    }

    // 缩放并转换为QPixmap
    QPixmap pixmap = QPixmap::fromImage(image.scaled(size, size, Qt::KeepAspectRatio, Qt::SmoothTransformation));

    // 释放内存
    QRcode_free(qrcode);

    return pixmap;
}
QString WeChatPay::getCurrentTimestamp()
{
    return QString::number(QDateTime::currentSecsSinceEpoch());
}
QString WeChatPay::nativeOrderSync(const QString &description,
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
    qDebug() << "订单号:" << outTradeNo;
    qDebug() << "金额:" << totalFee << "分";
    qDebug() << "请求体:" << bodyData;

    // 构建认证token
    QString token = buildToken("POST", API_NATIVE_ORDER, QString::fromUtf8(bodyData));
    if (token.isEmpty()) {
        return QString();
    }

    // 发送请求
    QNetworkRequest request;
    request.setUrl(QUrl(API_BASE_URL + API_NATIVE_ORDER));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Accept", "application/json");
    request.setRawHeader("Authorization", token.toUtf8());

    QNetworkReply *reply = m_manager->post(request, bodyData);

    // 使用事件循环等待请求完成（同步）
    QEventLoop loop;
    connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();

    // 处理响应
    QByteArray responseData = reply->readAll();
    int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

    qDebug() << "\n=== 收到响应 ===";
    qDebug() << "HTTP状态码:" << statusCode;
    qDebug() << "响应数据:" << responseData;
    qDebug() << "================\n";

    QString codeUrl;

    if (reply->error() == QNetworkReply::NoError) {
        // 解析响应
        QJsonParseError parseError;
        QJsonDocument doc = QJsonDocument::fromJson(responseData, &parseError);

        if (parseError.error == QJsonParseError::NoError) {
            QJsonObject obj = doc.object();
            if (obj.contains("code_url")) {
                codeUrl = obj["code_url"].toString();
                qDebug() << "✅ 下单成功，二维码链接:" << codeUrl;
            } else if (obj.contains("code") && obj.contains("message")) {
                QString errorCode = obj["code"].toString();
                QString errorMsg = obj["message"].toString();
            } else {
            }
        } else {
        }
    } else {
    }

    reply->deleteLater();
    return codeUrl;
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
            break;
        }

        // 从BIO读取私钥
        pkey = PEM_read_bio_PrivateKey(bio, NULL, NULL, NULL);
        if (!pkey) {
            break;
        }

        // 创建EVP_MD_CTX
        ctx = EVP_MD_CTX_new();
        if (!ctx) {
            break;
        }

        // 初始化签名操作
        if (EVP_DigestSignInit(ctx, NULL, EVP_sha256(), NULL, pkey) <= 0) {
            break;
        }

        // 更新签名数据
        QByteArray msgData = message.toUtf8();
        if (EVP_DigestSignUpdate(ctx, msgData.constData(), msgData.size()) <= 0) {
            break;
        }

        // 获取签名长度
        size_t sig_len = 0;
        if (EVP_DigestSignFinal(ctx, NULL, &sig_len) <= 0) {
            break;
        }

        // 执行签名
        QByteArray sigBuf(sig_len, '\0');
        if (EVP_DigestSignFinal(ctx, (unsigned char*)sigBuf.data(), &sig_len) <= 0) {
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

bool WeChatPay::closeOrder(const QString &outTradeNo)
{
    qDebug() << "\n=== 开始关闭订单 ===";
    qDebug() << "订单号:" << outTradeNo;

    // 构建URL路径（注意路径中包含订单号）
    QString urlPath = QString("/v3/pay/transactions/out-trade-no/%1/close").arg(outTradeNo);

    // 构建请求体（只需要mchid）
    QJsonObject requestBody;
    requestBody["mchid"] = m_mchid;

    QJsonDocument doc(requestBody);
    QByteArray bodyData = doc.toJson(QJsonDocument::Compact);

    qDebug() << "请求体:" << bodyData;

    // 构建认证token
    QString token = buildToken("POST", urlPath, QString::fromUtf8(bodyData));
    if (token.isEmpty()) {
        qDebug("构建认证token失败");
        return false;
    }

    // 发送请求
    QNetworkRequest request;
    request.setUrl(QUrl(API_BASE_URL + urlPath));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Accept", "application/json");
    request.setRawHeader("Authorization", token.toUtf8());

    QNetworkReply *reply = m_manager->post(request, bodyData);

    // 使用事件循环等待请求完成（同步）
    QEventLoop loop;
    connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();

    // 处理响应
    int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    QByteArray responseData = reply->readAll();

    qDebug() << "\n=== 关闭订单响应 ===";
    qDebug() << "HTTP状态码:" << statusCode;
    if (!responseData.isEmpty()) {
        qDebug() << "响应数据:" << responseData;
    }
    qDebug() << "==================\n";

    bool success = false;

    if (reply->error() == QNetworkReply::NoError) {
        // 根据文档，成功关闭订单返回 204 No Content
        if (statusCode == 204) {
            qDebug() << "✅ 订单关闭成功";
            success = true;
        } else {
            qDebug() << "❌ 订单关闭失败，非预期的状态码:" << statusCode;
        }
    } else {
        // 解析错误信息
        if (!responseData.isEmpty()) {
            QJsonParseError parseError;
            QJsonDocument doc = QJsonDocument::fromJson(responseData, &parseError);

            if (parseError.error == QJsonParseError::NoError) {
                QJsonObject obj = doc.object();
                QString errorCode = obj["code"].toString();
                QString errorMsg = obj["message"].toString();
            } else {
            }
        } else {
        }
    }

    reply->deleteLater();
    return success;
}
