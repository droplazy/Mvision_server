#include "firmware.h"
#include "ui_firmware.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QDir>
#include <QFile>
#include <QRegularExpression>
#include <QDebug>
#include <QRegularExpressionValidator>
#include <QMimeData>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QUrl>

firmware::firmware(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::firmware)
{
    ui->setupUi(this);

    // 禁止窗口拉伸
    setFixedSize(this->size());

    // 设置窗口标题
    setWindowTitle("固件上传");

    // 设置占位符文本
    ui->new_verison->setPlaceholderText("请输入版本号，如：1.0.0");
    ui->fimeware_path->setPlaceholderText("请选择固件文件或拖入文件");

    // 设置版本号输入限制（只允许数字和点）
    QRegularExpression regExp("^[0-9.]*$");
    QRegularExpressionValidator *validator = new QRegularExpressionValidator(regExp, this);
    ui->new_verison->setValidator(validator);

    // 启用拖放功能
    ui->fimeware_path->setAcceptDrops(true);
    ui->fimeware_path->installEventFilter(this);  // 添加事件过滤器

    // 检查现有固件
    checkExistingFirmware();
}

firmware::~firmware()
{
    delete ui;
}

// 事件过滤器处理拖放事件
bool firmware::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == ui->fimeware_path) {
        if (event->type() == QEvent::DragEnter) {
            QDragEnterEvent *dragEvent = static_cast<QDragEnterEvent*>(event);
            if (dragEvent->mimeData()->hasUrls()) {
                dragEvent->acceptProposedAction();
                return true;
            }
        }
        else if (event->type() == QEvent::Drop) {
            QDropEvent *dropEvent = static_cast<QDropEvent*>(event);
            QList<QUrl> urls = dropEvent->mimeData()->urls();
            if (!urls.isEmpty()) {
                QString filePath = urls.first().toLocalFile();
                ui->fimeware_path->setText(filePath);

                // 自动从文件名中提取版本号
                extractVersionFromFilename(filePath);

                return true;
            }
        }
    }
    return QDialog::eventFilter(watched, event);
}

QString firmware::getVersion() const
{
    return ui->new_verison->text().trimmed();
}

QString firmware::getFirmwarePath() const
{
    return ui->fimeware_path->text().trimmed();
}

// 比较版本号大小，返回：1 表示 version1 > version2，-1 表示 version1 < version2，0 表示相等
int firmware::compareVersions(const QString &version1, const QString &version2)
{
    QStringList v1Parts = version1.split('.');
    QStringList v2Parts = version2.split('.');

    int maxLength = qMax(v1Parts.size(), v2Parts.size());

    for (int i = 0; i < maxLength; i++) {
        int v1 = (i < v1Parts.size()) ? v1Parts[i].toInt() : 0;
        int v2 = (i < v2Parts.size()) ? v2Parts[i].toInt() : 0;

        if (v1 > v2) return 1;
        if (v1 < v2) return -1;
    }

    return 0;
}

// 获取最新固件版本文件名
QString firmware::getLatestFirmwareVersion()
{
    QDir downloadDir(QDir::current().filePath("Download"));

    // 如果Download目录不存在，返回空
    if (!downloadDir.exists()) {
        qDebug() << "Download目录不存在";
        return "";
    }

    // 设置过滤器，只查找firmware开头的.tar.gz文件
    QStringList filters;
    filters << "firmware_*.tar.gz";
    downloadDir.setNameFilters(filters);

    QFileInfoList fileList = downloadDir.entryInfoList(QDir::Files | QDir::NoDotAndDotDot);

    if (fileList.isEmpty()) {
        qDebug() << "Download目录中没有固件文件";
        return "";
    }

    QString latestVersion = "";
    QString latestFileName = "";

    foreach (const QFileInfo &fileInfo, fileList) {
        QString fileName = fileInfo.fileName();

        // 提取版本号，例如从"firmware_0.1.1.tar.gz"中提取"0.1.1"
        if (fileName.startsWith("firmware_") && fileName.endsWith(".tar.gz")) {
            QString versionStr = fileName.mid(9);  // 去掉"firmware_"
            versionStr = versionStr.left(versionStr.length() - 7);  // 去掉".tar.gz"

            // 比较版本号
            if (latestVersion.isEmpty() || compareVersions(versionStr, latestVersion) > 0) {
                latestVersion = versionStr;
                latestFileName = fileName;
            }
        }
    }

    return latestFileName;  // 返回完整的文件名
}

void firmware::checkExistingFirmware()
{
    QString latestFile = getLatestFirmwareVersion();

    if (latestFile.isEmpty()) {
        ui->label->setText("当前可能没有固件上传 请手动确认");
        return;
    }

    // 从文件名中提取版本号
    QRegularExpression versionRegExp("firmware_([0-9]+\\.[0-9]+\\.[0-9]+)\\.tar\\.gz");
    QRegularExpressionMatch match = versionRegExp.match(latestFile);

    if (match.hasMatch()) {
        QString version = match.captured(1);
        ui->label->setText(QString("现有固件版本：%1").arg(version));

        // 在版本号输入框中显示当前版本
        if (ui->new_verison->text().isEmpty()) {
            ui->new_verison->setText(version);
        }
    } else {
        ui->label->setText("发现固件文件但版本号格式无效");
    }
}

// 从文件名中提取版本号
void firmware::extractVersionFromFilename(const QString &filePath)
{
    QFileInfo fileInfo(filePath);
    QString baseName = fileInfo.baseName();  // 获取不带后缀的文件名

    // 尝试从文件名中提取版本号模式 (x.x.x)
    QRegularExpression versionInFile("(\\d+\\.\\d+\\.\\d+)");
    QRegularExpressionMatch match = versionInFile.match(baseName);

    if (match.hasMatch() && ui->new_verison->text().isEmpty()) {
        ui->new_verison->setText(match.captured(1));
    }
}

bool firmware::isValidVersion(const QString &version)
{
    // 版本号格式：x.x.x，其中x为数字
    QRegularExpression regExp("^\\d+\\.\\d+\\.\\d+$");
    return regExp.match(version).hasMatch();
}

bool firmware::isValidFirmwareFile(const QString &filePath)
{
    QFileInfo fileInfo(filePath);

    // 检查文件是否存在
    if (!fileInfo.exists()) {
        return false;
    }

    // 检查文件后缀是否为.tar.gz
    QString suffix = fileInfo.suffix().toLower();
    QString completeSuffix = fileInfo.completeSuffix().toLower();

    return (completeSuffix == "tar.gz" || suffix == "gz");
}

QString firmware::generateFirmwareFilename(const QString &version)
{
    return QString("firmware_%1.tar.gz").arg(version);
}

bool firmware::copyFirmwareFile(const QString &sourcePath, const QString &version)
{
    QDir downloadDir(QDir::current().filePath("Download"));

    // 创建Download目录（如果不存在）
    QDir dir(downloadDir);
    if (!dir.exists()) {
        if (!dir.mkpath(".")) {
            QMessageBox::critical(this, "错误", "无法创建Download目录");
            return false;
        }
    }

    // 生成目标文件名
    QString targetFilename = generateFirmwareFilename(version);
    QString targetPath = QDir(downloadDir).filePath(targetFilename);

    // 检查目标文件是否已存在
    if (QFile::exists(targetPath)) {
        int result = QMessageBox::question(this, "确认覆盖",
                                           QString("固件文件 %1 已存在，是否覆盖？").arg(targetFilename),
                                           QMessageBox::Yes | QMessageBox::No,
                                           QMessageBox::No);

        if (result == QMessageBox::No) {
            return false;
        }
        // 删除已存在的文件
        QFile::remove(targetPath);
    }

    // 使用QFile对象进行复制，以便获取错误信息
    QFile sourceFile(sourcePath);
    if (!sourceFile.exists()) {
        QMessageBox::critical(this, "错误", "源文件不存在");
        return false;
    }

    // 复制文件
    if (sourceFile.copy(targetPath)) {
        // 设置文件权限
        QFile::setPermissions(targetPath, QFile::ReadOwner | QFile::WriteOwner | QFile::ReadGroup | QFile::ReadOther);
        return true;
    } else {
        QMessageBox::critical(this, "错误",
                              QString("文件复制失败：%1").arg(sourceFile.errorString()));
        return false;
    }
}

void firmware::on_pushButton__Browse_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    "选择固件文件",
                                                    QDir::homePath(),
                                                    "固件文件 (*.tar.gz *.gz);;所有文件 (*)");

    if (!fileName.isEmpty()) {
        ui->fimeware_path->setText(fileName);
        extractVersionFromFilename(fileName);
    }
}

void firmware::on_pushButton_cancel_clicked()
{
    // 拒绝对话框
    reject();
}

void firmware::on_pushButton_OK_clicked()
{
    // 获取输入值
    QString version = getVersion();
    QString filePath = getFirmwarePath();

    // 验证版本号
    if (version.isEmpty()) {
        QMessageBox::warning(this, "警告", "请输入版本号");
        ui->new_verison->setFocus();
        return;
    }

    if (!isValidVersion(version)) {
        QMessageBox::warning(this, "警告",
                             "版本号格式无效，请使用 x.x.x 格式（如：1.0.0）");
        ui->new_verison->setFocus();
        return;
    }

    // 验证文件路径
    if (filePath.isEmpty()) {
        QMessageBox::warning(this, "警告", "请选择固件文件");
        ui->fimeware_path->setFocus();
        return;
    }

    if (!isValidFirmwareFile(filePath)) {
        QMessageBox::warning(this, "警告",
                             "请选择有效的固件文件（.tar.gz 或 .gz 格式）");
        ui->fimeware_path->setFocus();
        return;
    }

    // 检查文件是否存在
    QFileInfo sourceFile(filePath);
    if (!sourceFile.exists()) {
        QMessageBox::critical(this, "错误", "选择的文件不存在");
        return;
    }

    // 检查文件大小（可选）
    qint64 fileSize = sourceFile.size();
    const qint64 MAX_FILE_SIZE = 100 * 1024 * 1024; // 100MB
    if (fileSize > MAX_FILE_SIZE) {
        QMessageBox::warning(this, "警告",
                             QString("文件大小超过限制（最大 100MB，当前 %1 MB）")
                                 .arg(fileSize / (1024.0 * 1024.0), 0, 'f', 2));
        return;
    }

    // 复制文件到Download目录
    if (copyFirmwareFile(filePath, version)) {
        QMessageBox::information(this, "成功",
                                 QString("固件文件已成功上传为：\nfirmware_%1.tar.gz").arg(version));

        // 更新标签显示
        ui->label->setText(QString("现有固件版本：%1").arg(version));

        // 接受对话框
        accept();
    }
}
