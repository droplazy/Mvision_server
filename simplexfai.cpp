#include "simplexfai.h"
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QDebug>

SimpleXFAI::SimpleXFAI(QObject *parent)
    : QObject(parent)
    , m_manager(new QNetworkAccessManager(this))
    , m_reply(nullptr)
{
    // 从你的截图设置默认认证信息
    m_apiKey = "0731bdabe8a186215737d1edeb15b9ea";
    m_apiSecret = "MGM2NGNlYWM4NTA3Mzc3ZmY4ODIzZmZh";
}

void SimpleXFAI::setAuthInfo(const QString &apiKey, const QString &apiSecret)
{
    m_apiKey = apiKey;
    m_apiSecret = apiSecret;
}

void SimpleXFAI::askQuestion(const QString &question)
{
    qDebug() << "=== 发送问题到讯飞AI ===";
    qDebug() << "问题:" << question;

    if (question.isEmpty()) {
        emit errorOccurred("问题不能为空");
        return;
    }

    // 如果有正在进行的请求，先取消
    if (m_reply && m_reply->isRunning()) {
        m_reply->abort();
        m_reply->deleteLater();
        m_reply = nullptr;
    }

    // 构建请求URL
    QUrl url("https://spark-api-open.xf-yun.com/v2/chat/completions");

    QNetworkRequest request(url);

    // 设置请求头 - 认证方式：AK:SK（从文档看）
    QString authHeader = "Bearer " + m_apiKey + ":" + m_apiSecret;
    request.setRawHeader("Authorization", authHeader.toUtf8());
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    // 构建请求体
    QJsonObject requestBody;
    requestBody["model"] = "spark-x";  // X1.5模型

    // 消息数组
    QJsonArray messages;
    QJsonObject message;
    message["role"] = "user";
    message["content"] = question;
    messages.append(message);
    requestBody["messages"] = messages;

    // 可选参数
    requestBody["temperature"] = 1.2;
    requestBody["max_tokens"] = 65535;

    // 非流式响应
    requestBody["stream"] = false;

    QJsonDocument doc(requestBody);
    QByteArray postData = doc.toJson();

    qDebug() << "发送请求...";

    // 发送请求
    m_reply = m_manager->post(request, postData);

    // 连接完成信号
    connect(m_reply, &QNetworkReply::finished, this, &SimpleXFAI::onReplyFinished);
}

void SimpleXFAI::onReplyFinished()
{
    if (!m_reply) return;

    QNetworkReply *reply = m_reply;
    m_reply = nullptr;

    if (reply->error() != QNetworkReply::NoError) {
        QString errorStr = QString("网络错误: %1").arg(reply->errorString());
        qDebug() << errorStr;
        emit errorOccurred(errorStr);
        reply->deleteLater();
        return;
    }

    // 读取响应
    QByteArray responseData = reply->readAll();
    reply->deleteLater();

    qDebug() << "收到响应，大小:" << responseData.size() << "字节";
    qDebug() << "响应内容:" << QString::fromUtf8(responseData);

    // 解析JSON
    QJsonDocument doc = QJsonDocument::fromJson(responseData);
    if (doc.isNull()) {
        emit errorOccurred("响应不是有效的JSON");
        return;
    }

    QJsonObject response = doc.object();

    // 检查错误码
    int code = response["code"].toInt();
    if (code != 0) {
        QString errorMsg = response["message"].toString();
        QString errorStr = QString("讯飞API错误 %1: %2").arg(code).arg(errorMsg);
        qDebug() << errorStr;
        emit errorOccurred(errorStr);
        return;
    }

    // 提取回复内容
    if (response.contains("choices")) {
        QJsonArray choices = response["choices"].toArray();
        if (!choices.isEmpty()) {
            QJsonObject choice = choices.first().toObject();
            if (choice.contains("message")) {
                QJsonObject message = choice["message"].toObject();
                QString content = message["content"].toString();

                if (!content.isEmpty()) {
                    qDebug() << "AI回复:" << content;
                    emit responseReceived(content);
                } else {
                    emit errorOccurred("AI回复为空");
                }
            } else {
                emit errorOccurred("响应格式错误：缺少message字段");
            }
        } else {
            emit errorOccurred("响应格式错误：choices数组为空");
        }
    } else {
        emit errorOccurred("响应格式错误：缺少choices字段");
    }
}
