#ifndef FIRMWARE_H
#define FIRMWARE_H

#include <QDialog>

namespace Ui {
class firmware;
}

class firmware : public QDialog
{
    Q_OBJECT

public:
    explicit firmware(QWidget *parent = nullptr);
    ~firmware();

    // 获取固件信息
    QString getVersion() const;
    QString getFirmwarePath() const;

private slots:
    void on_pushButton_OK_clicked();
    void on_pushButton_cancel_clicked();
    void on_pushButton__Browse_clicked();

private:
    Ui::firmware *ui;

    int compareVersions(const QString &version1, const QString &version2);
    void checkExistingFirmware();  // 检查现有固件
    bool copyFirmwareFile(const QString &sourcePath, const QString &version);  // 复制固件文件
    bool isValidVersion(const QString &version);  // 验证版本号格式
    bool isValidFirmwareFile(const QString &filePath);  // 验证固件文件
    QString generateFirmwareFilename(const QString &version);  // 生成固件文件名
    void extractVersionFromFilename(const QString &filePath);
    bool eventFilter(QObject *watched, QEvent *event);
    QString getLatestFirmwareVersion();
};

#endif // FIRMWARE_H
