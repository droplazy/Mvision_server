// DeepSeekAI.h
#ifndef DEEPSEEKAI_H
#define DEEPSEEKAI_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>

class DeepSeekAI : public QObject
{
    Q_OBJECT

public:
    explicit DeepSeekAI(QObject *parent = nullptr);
    ~DeepSeekAI();

    // 设置API密钥
    void setApiKey(const QString &apiKey);

    // 发送请求
    void askQuestion(const QString &prompt);

signals:
    void responseReceived(const QString &response);
    void errorOccurred(const QString &error);

private slots:
    void onReplyFinished(QNetworkReply *reply);

private:
    QNetworkAccessManager *manager;
    QString apiKey;
    QString baseUrl;
};

#endif // DEEPSEEKAI_H
