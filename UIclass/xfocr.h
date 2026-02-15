#ifndef XFOCR_H
#define XFOCR_H

#include <QObject>
#include <QString>
#include <QByteArray>
#include <QImage>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QFile>
#include <QJsonObject>
#include <QFileInfo>
#include <QMessageAuthenticationCode>
#include <QCryptographicHash>
#include <QEvent>



class XFOCR : public QObject
{
    Q_OBJECT

public:
    explicit XFOCR(QObject *parent = nullptr);

    // 设置认证信息
    void setAuthInfo(const QString &appId, const QString &apiKey, const QString &apiSecret);

    // 同步识别接口 - 阻塞直到返回结果
    QString recognizeImageSync(const QString &filePath, int timeoutMs = 30000);

    // 异步识别接口 - 立即返回，通过信号通知结果
    void recognizeImageAsync(const QString &filePath);

    // 是否正在识别
    bool isProcessing() const { return m_isProcessing; }

signals:
    // 异步识别完成信号
    void recognitionFinished(bool success, const QString &text, const QString &error);

    // 同步识别完成信号（内部使用）
    void asyncFinished();

private slots:
    void onReplyFinished(QNetworkReply *reply);

private:
    // 生成鉴权参数
    QString generateAuthorization() const;

    // 获取GMT时间
    QString getGMTTime() const;

    // 图片转base64
    QByteArray imageToBase64(const QImage &image) const;
    QByteArray imageToBase64(const QString &filePath) const;

    // 构建请求JSON
    QByteArray buildRequestJson(const QByteArray &imageBase64, const QString &format) const;

    // 解析响应
    bool parseResponse(const QByteArray &data, QString &resultText, QString &error) const;

private:
    QString m_appId;
    QString m_apiKey;
    QString m_apiSecret;
    QNetworkAccessManager *m_networkManager;
    bool m_isProcessing;
    QString m_lastResult;
    QString m_lastError;
    QEventLoop m_eventLoop;  // 用于同步调用
};

#endif // XFOCR_H
