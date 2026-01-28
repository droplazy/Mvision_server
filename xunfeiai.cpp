#include "XunfeiAI.h"
#include <QNetworkRequest>
#include <QJsonDocument>
#include <QJsonArray>
#include <QDebug>
#include <QNetworkReply>


XunfeiAI::XunfeiAI(QObject *parent)
    : QObject(parent)
    , m_netManager(new QNetworkAccessManager(this))
{
    connect(m_netManager, &QNetworkAccessManager::finished,
            this, &XunfeiAI::onRequestFinished);
}

// 设置API密钥
void XunfeiAI::setup(const QString &apiKey, const QString &apiSecret)
{
    m_apiKey = apiKey;
    m_apiSecret = apiSecret;
}

// 发送问题
void XunfeiAI::ask(const QString &question)
{
    sendRequest(buildRequest("user", question));
}

// 发送带系统提示的问题
void XunfeiAI::askWithSystem(const QString &systemPrompt, const QString &question)
{
    QJsonObject request;
    request["model"] = "spark-x";

    QJsonArray messages;
    messages.append(QJsonObject{{"role", "system"}, {"content", systemPrompt}});
    messages.append(QJsonObject{{"role", "user"}, {"content", question}});

    request["messages"] = messages;
    request["max_tokens"] = m_maxTokens;
    request["temperature"] = m_temperature;

    if (m_thinkingEnabled) {
        request["thinking"] = QJsonObject{{"type", "auto"}};
    }

    if (m_webSearchEnabled) {
        QJsonArray tools;
        tools.append(QJsonObject{
            {"type", "web_search"},
            {"web_search", QJsonObject{{"enable", true}, {"search_mode", "normal"}}}
        });
        request["tools"] = tools;
    }

    QJsonDocument doc(request);
    sendRequest(doc.toJson());
}

// 构建请求数据
QByteArray XunfeiAI::buildRequest(const QString &role, const QString &content)
{
    QJsonObject request;
    request["model"] = "spark-x";

    QJsonArray messages;
    messages.append(QJsonObject{{"role", role}, {"content", content}});

    request["messages"] = messages;
    request["max_tokens"] = m_maxTokens;
    request["temperature"] = m_temperature;

    if (m_thinkingEnabled) {
        request["thinking"] = QJsonObject{{"type", "auto"}};
    }

    if (m_webSearchEnabled) {
        QJsonArray tools;
        tools.append(QJsonObject{
            {"type", "web_search"},
            {"web_search", QJsonObject{{"enable", true}, {"search_mode", "normal"}}}
        });
        request["tools"] = tools;
    }

    return QJsonDocument(request).toJson();
}

// 发送请求
void XunfeiAI::sendRequest(const QByteArray &data)
{
    if (m_apiKey.isEmpty() || m_apiSecret.isEmpty()) {
        emit errorOccurred("请先调用setup()设置API密钥");
        return;
    }

    QString url = "https://spark-api-open.xf-yun.com/v2/chat/completions";
    QString auth = QString("Bearer %1:%2").arg(m_apiKey).arg(m_apiSecret);

    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Authorization", auth.toUtf8());

    m_netManager->post(request, data);
}

// 解析响应
XunfeiAI::Response XunfeiAI::parseResponse(const QByteArray &data)
{
    Response result;
    result.success = false;

    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (doc.isNull()) {
        result.error = "响应数据格式错误";
        return result;
    }

    QJsonObject obj = doc.object();

    // 检查错误
    if (obj.contains("error")) {
        result.error = obj["error"].toObject()["message"].toString();
        return result;
    }

    // 提取AI回复
    if (obj.contains("choices") && obj["choices"].isArray()) {
        QJsonArray choices = obj["choices"].toArray();
        if (!choices.isEmpty()) {
            QJsonObject choice = choices[0].toObject();
            QJsonObject message = choice["message"].toObject();

            result.content = message["content"].toString();
            result.reasoning = message["reasoning_content"].toString();
            result.success = true;
        }
    }

    // 提取token使用量
    if (obj.contains("usage")) {
        QJsonObject usage = obj["usage"].toObject();
        result.tokens = usage["total_tokens"].toInt();
    }

    return result;
}

// 请求完成处理
void XunfeiAI::onRequestFinished()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply) return;

    Response response;

    if (reply->error() == QNetworkReply::NoError) {
        response = parseResponse(reply->readAll());
    } else {
        response.success = false;
        response.error = QString("网络错误: %1").arg(reply->errorString());
    }

    emit responseReceived(response);
    reply->deleteLater();
}

// 设置参数
void XunfeiAI::setMaxTokens(int maxTokens)
{
    m_maxTokens = qMax(1, maxTokens);
}

void XunfeiAI::setTemperature(float temperature)
{
    m_temperature = qBound(0.0f, temperature, 2.0f);
}

void XunfeiAI::enableThinking(bool enable)
{
    m_thinkingEnabled = enable;
}

void XunfeiAI::enableWebSearch(bool enable)
{
    m_webSearchEnabled = enable;
}
