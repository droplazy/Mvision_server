// DeepSeekAI.cpp
#include "DeepSeekAI.h"
#include <QNetworkRequest>
#include <QUrl>
#include <QDebug>

DeepSeekAI::DeepSeekAI(QObject *parent)
    : QObject(parent)
    , apiKey("sk-02d007d6f3a547e6a2d4395e57c79e93")  // 这里使用你的API密钥
    , baseUrl("https://api.deepseek.com")
{
    manager = new QNetworkAccessManager(this);
}

DeepSeekAI::~DeepSeekAI()
{
    delete manager;
}

void DeepSeekAI::setApiKey(const QString &apiKey)
{
    this->apiKey = apiKey;
}

void DeepSeekAI::askQuestion(const QString &prompt)
{
    if (apiKey.isEmpty()) {
        emit errorOccurred("API密钥未设置");
        return;
    }

    QUrl url(baseUrl + "/chat/completions");
    QNetworkRequest request(url);

    // 设置请求头
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Authorization", QString("Bearer %1").arg(apiKey).toUtf8());

    // 构建请求体
    QJsonObject requestBody;
    requestBody["model"] = "deepseek-chat";  // 使用非思考模式
    requestBody["stream"] = false;

    QJsonArray messages;
    QJsonObject systemMessage;
    systemMessage["role"] = "system";
    systemMessage["content"] = "你是一个有用的助手";
    messages.append(systemMessage);

    QJsonObject userMessage;
    userMessage["role"] = "user";
    userMessage["content"] = prompt;
    messages.append(userMessage);

    requestBody["messages"] = messages;

    QJsonDocument doc(requestBody);
    QByteArray data = doc.toJson();

    // 发送请求
    QNetworkReply *reply = manager->post(request, data);

    // 连接信号
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        onReplyFinished(reply);
    });
}

void DeepSeekAI::onReplyFinished(QNetworkReply *reply)
{
    reply->deleteLater();

    if (reply->error() != QNetworkReply::NoError) {
        emit errorOccurred(reply->errorString());
        return;
    }

    QByteArray responseData = reply->readAll();
    QJsonDocument doc = QJsonDocument::fromJson(responseData);

    if (doc.isNull()) {
        emit errorOccurred("响应解析失败");
        return;
    }

    QJsonObject json = doc.object();

    // 解析响应
    if (json.contains("choices") && json["choices"].isArray()) {
        QJsonArray choices = json["choices"].toArray();
        if (!choices.isEmpty()) {
            QJsonObject choice = choices.first().toObject();
            if (choice.contains("message") && choice["message"].isObject()) {
                QJsonObject message = choice["message"].toObject();
                if (message.contains("content")) {
                    QString response = message["content"].toString();
                    emit responseReceived(response);
                    return;
                }
            }
        }
    }

    emit errorOccurred("响应格式不正确");
}
