#include "XFOCR.h"
#include <QNetworkRequest>
#include <QDateTime>
#include <QBuffer>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonParseError>
#include <QDebug>
#include <QEventLoop>
#include <QTimer>
#include <QUrl>

XFOCR::XFOCR(QObject *parent)
    : QObject(parent)
    , m_networkManager(new QNetworkAccessManager(this))
    , m_isProcessing(false)
{
    connect(m_networkManager, &QNetworkAccessManager::finished,
            this, &XFOCR::onReplyFinished);
}

void XFOCR::setAuthInfo(const QString &appId, const QString &apiKey, const QString &apiSecret)
{
    m_appId = appId;
    m_apiKey = apiKey;
    m_apiSecret = apiSecret;
}

QString XFOCR::recognizeImageSync(const QString &filePath, int timeoutMs)
{
    if (!QFile::exists(filePath)) {
        return QString("Error: File not exists: %1").arg(filePath);
    }

    if (m_isProcessing) {
        return "Error: OCR is busy";
    }

    m_isProcessing = true;
    m_lastResult.clear();
    m_lastError.clear();
    m_eventLoop.quit();

    auto conn = connect(this, &XFOCR::asyncFinished, [this]() {
        m_eventLoop.quit();
    });

    recognizeImageAsync(filePath);

    QTimer timeoutTimer;
    timeoutTimer.setSingleShot(true);
    timeoutTimer.start(timeoutMs);

    m_eventLoop.exec();
    disconnect(conn);
    m_isProcessing = false;

    if (!timeoutTimer.isActive()) {
        return QString("Error: Timeout after %1 ms").arg(timeoutMs);
    }

    if (m_lastError.isEmpty()) {
        return m_lastResult;
    } else {
        return QString("Error: %1").arg(m_lastError);
    }
}

void XFOCR::recognizeImageAsync(const QString &filePath)
{
    if (!QFile::exists(filePath)) {
        emit recognitionFinished(false, "", QString("File not exists: %1").arg(filePath));
        return;
    }

    m_isProcessing = true;
    QByteArray imageBase64 = imageToBase64(filePath);

    if (imageBase64.isEmpty()) {
        emit recognitionFinished(false, "", "Failed to load image");
        m_isProcessing = false;
        return;
    }

    if (imageBase64.size() > 4 * 1024 * 1024) {
        emit recognitionFinished(false, "", "Image too large (max 4MB)");
        m_isProcessing = false;
        return;
    }

    QString format = QFileInfo(filePath).suffix().toLower();
    if (format == "jpeg") format = "jpg";

    QByteArray requestData = buildRequestJson(imageBase64, format);
    QString auth = generateAuthorization();
    QString date = getGMTTime();

    QString encodedAuth = QString::fromLatin1(auth.toUtf8().toBase64());
    QString encodedDate = QUrl::toPercentEncoding(date);

    QString url = QString("https://api.xf-yun.com/v1/private/sf8e6aca1?authorization=%1&host=api.xf-yun.com&date=%2")
                      .arg(encodedAuth).arg(encodedDate);

    QNetworkRequest request;
    request.setUrl(QUrl(url));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Accept", "application/json");

    m_networkManager->post(request, requestData);
}

void XFOCR::onReplyFinished(QNetworkReply *reply)
{
    m_isProcessing = false;
    reply->deleteLater();

    QString resultText;
    QString errorMsg;
    bool success = false;

    if (reply->error() != QNetworkReply::NoError) {
        errorMsg = QString("Network error: %1").arg(reply->errorString());
    } else {
        QByteArray responseData = reply->readAll();
        if (responseData.isEmpty()) {
            errorMsg = "Empty response";
        } else {
            success = parseResponse(responseData, resultText, errorMsg);
        }
    }

    m_lastResult = resultText;
    m_lastError = errorMsg;
    emit recognitionFinished(success, resultText, errorMsg);
    emit asyncFinished();
}

QByteArray XFOCR::imageToBase64(const QString &filePath) const
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "无法打开文件:" << filePath;
        return QByteArray();
    }

    QByteArray data = file.readAll();
    file.close();
    return data.toBase64();
}

QByteArray XFOCR::imageToBase64(const QImage &image) const
{
    QByteArray data;
    QBuffer buffer(&data);
    buffer.open(QIODevice::WriteOnly);
    image.save(&buffer, "PNG");
    buffer.close();
    return data.toBase64();
}

QByteArray XFOCR::buildRequestJson(const QByteArray &imageBase64, const QString &format) const
{
    QJsonObject header;
    header.insert("app_id", m_appId);
    header.insert("status", 3);

    QJsonObject result;
    result.insert("encoding", "utf8");
    result.insert("compress", "raw");
    result.insert("format", "json");

    QJsonObject sf8e6aca1;
    sf8e6aca1.insert("category", "ch_en_public_cloud");
    sf8e6aca1.insert("result", result);

    QJsonObject parameter;
    parameter.insert("sf8e6aca1", sf8e6aca1);

    QJsonObject imageData;
    imageData.insert("encoding", format);
    imageData.insert("status", 3);
    imageData.insert("image", QString::fromLatin1(imageBase64));

    QJsonObject sf8e6aca1Data;
    sf8e6aca1Data.insert("sf8e6aca1_data_1", imageData);

    QJsonObject request;
    request.insert("header", header);
    request.insert("parameter", parameter);
    request.insert("payload", sf8e6aca1Data);

    QJsonDocument doc(request);
    return doc.toJson(QJsonDocument::Compact);
}

QString XFOCR::getGMTTime() const
{
    QDateTime utc = QDateTime::currentDateTimeUtc();
    QString dayNames[] = {"Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun"};
    QString monthNames[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun",
                            "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

    int dayOfWeek = utc.date().dayOfWeek() - 1;
    if (dayOfWeek < 0) dayOfWeek = 6;

    return QString("%1, %2 %3 %4 %5:%6:%7 GMT")
        .arg(dayNames[dayOfWeek])
        .arg(utc.date().day(), 2, 10, QLatin1Char('0'))
        .arg(monthNames[utc.date().month() - 1])
        .arg(utc.date().year())
        .arg(utc.time().hour(), 2, 10, QLatin1Char('0'))
        .arg(utc.time().minute(), 2, 10, QLatin1Char('0'))
        .arg(utc.time().second(), 2, 10, QLatin1Char('0'));
}

QString XFOCR::generateAuthorization() const
{
    QString date = getGMTTime();
    QString signatureOrigin = QString("host: api.xf-yun.com\ndate: %1\nPOST /v1/private/sf8e6aca1 HTTP/1.1").arg(date);

    QByteArray key = m_apiSecret.toUtf8();
    QByteArray data = signatureOrigin.toUtf8();

    QMessageAuthenticationCode hmac(QCryptographicHash::Sha256);
    hmac.setKey(key);
    hmac.addData(data);
    QByteArray hmacResult = hmac.result();

    QString signature = QString::fromLatin1(hmacResult.toBase64());

    return QString("api_key=\"%1\", algorithm=\"hmac-sha256\", headers=\"host date request-line\", signature=\"%2\"")
        .arg(m_apiKey).arg(signature);
}

bool XFOCR::parseResponse(const QByteArray &data, QString &resultText, QString &error) const
{
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);

    if (parseError.error != QJsonParseError::NoError) {
        error = QString("JSON error: %1").arg(parseError.errorString());
        return false;
    }

    QJsonObject root = doc.object();
    QJsonObject header = root.value("header").toObject();
    int code = header.value("code").toInt(-1);

    if (code != 0) {
        error = QString("API error %1").arg(code);
        return false;
    }

    QString textBase64 = root.value("payload").toObject()
                             .value("result").toObject()
                             .value("text").toString();

    if (textBase64.isEmpty()) {
        error = "No text in response";
        return false;
    }

    QByteArray textData = QByteArray::fromBase64(textBase64.toLatin1());
    QJsonDocument textDoc = QJsonDocument::fromJson(textData, &parseError);

    if (parseError.error != QJsonParseError::NoError) {
        error = QString("Parse error: %1").arg(parseError.errorString());
        return false;
    }

    QJsonObject textObj = textDoc.object();
    QJsonArray pages = textObj.value("pages").toArray();

    QString allText;
    for (const QJsonValue &pageVal : pages) {
        QJsonArray lines = pageVal.toObject().value("lines").toArray();
        for (const QJsonValue &lineVal : lines) {
            QJsonArray words = lineVal.toObject().value("words").toArray();
            for (const QJsonValue &wordVal : words) {
                allText += wordVal.toObject().value("content").toString();
            }
        }
    }

    if (allText.isEmpty()) {
        error = "No text recognized";
        return false;
    }

    resultText = allText;
    return true;
}
