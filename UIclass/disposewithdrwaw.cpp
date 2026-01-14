#include "disposewithdrwaw.h"
#include "ui_disposewithdrwaw.h"
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QUrl>
#include <QFileDialog>
#include <QMessageBox>
#include <QDebug>
#include <QImageReader>
#include <QSqlQuery>
#include <QDateTime>

disposewithdrwaw::disposewithdrwaw(QWidget *parent,
                                   DatabaseManager *dbManager,
                                   const QString &withdrawId)
    : QDialog(parent)
    , ui(new Ui::disposewithdrwaw)
    , dbManager(dbManager)
    , currentWithdrawId(withdrawId)
{
    ui->setupUi(this);

    // 设置窗口标题
    setWindowTitle("处理提现申请");

    // 设置固定大小
    setFixedSize(400, 468);

    // 设置拖放支持
    setupDragDrop();

    // 设置图片预览标签的占位文本
    ui->label_image->setText("拖入转账截图或点击浏览");
    ui->label_image->setAlignment(Qt::AlignCenter);
    ui->label_image->setStyleSheet("border: 2px dashed #aaa; padding: 10px;");

    // 连接按钮信号
    // connect(ui->pushButton_browse, &QPushButton::clicked,
    //         this, &disposewithdrwaw::on_pushButton_browse_clicked);
}

disposewithdrwaw::~disposewithdrwaw()
{
    delete ui;
}

void disposewithdrwaw::setWithdrawInfo(const QString &withdrawId,
                                       const QString &username,
                                       double amount,
                                       const QString &alipayAccount)
{
    currentWithdrawId = withdrawId;

    // 显示提现信息
    ui->label_in->setText(QString("处理提现申请\n"
                                    "提现ID: %1\n"
                                    "用户名: %2\n"
                                    "金额: ¥%3\n"
                                    "支付宝: %4")
                                .arg(withdrawId)
                                .arg(username)
                                .arg(amount, 0, 'f', 2)
                                .arg(alipayAccount));
}

void disposewithdrwaw::setupDragDrop()
{
    // 启用拖放
    this->setAcceptDrops(true);
    ui->lineEdit->setAcceptDrops(true);
    ui->label_image->setAcceptDrops(true);

    // 处理拖放事件
    ui->lineEdit->installEventFilter(this);
    ui->label_image->installEventFilter(this);
}

bool disposewithdrwaw::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == ui->lineEdit || obj == ui->label_image) {
        if (event->type() == QEvent::DragEnter) {
            QDragEnterEvent *dragEvent = static_cast<QDragEnterEvent*>(event);
            if (dragEvent->mimeData()->hasUrls()) {
                dragEvent->acceptProposedAction();
                return true;
            }
        }
        else if (event->type() == QEvent::Drop) {
            QDropEvent *dropEvent = static_cast<QDropEvent*>(event);
            const QMimeData *mimeData = dropEvent->mimeData();

            if (mimeData->hasUrls()) {
                QList<QUrl> urls = mimeData->urls();
                if (!urls.isEmpty()) {
                    QString filePath = urls.first().toLocalFile();

                    if (obj == ui->lineEdit) {
                        ui->lineEdit->setText(filePath);
                        updateImagePreview(filePath);
                    } else if (obj == ui->label_image) {
                        updateImagePreview(filePath);
                    }
                }
                return true;
            }
        }
    }
    return QDialog::eventFilter(obj, event);
}

bool disposewithdrwaw::validateImage(const QString &filePath)
{
    // 检查文件是否存在
    QFileInfo fileInfo(filePath);
    if (!fileInfo.exists()) {
        QMessageBox::warning(this, "错误", "文件不存在！");
        return false;
    }

    // 检查文件大小（限制为10MB）
    if (fileInfo.size() > 10 * 1024 * 1024) {
        QMessageBox::warning(this, "错误", "文件太大，请选择小于10MB的图片！");
        return false;
    }

    // 检查文件格式
    QString suffix = fileInfo.suffix().toLower();
    QStringList supportedFormats = {"jpg", "jpeg", "png", "bmp", "gif"};
    if (!supportedFormats.contains(suffix)) {
        QMessageBox::warning(this, "错误",
                             "不支持的文件格式！\n支持格式: JPG, PNG, BMP, GIF");
        return false;
    }

    // 尝试加载图片检查是否有效
    QImageReader reader(filePath);
    if (!reader.canRead()) {
        QMessageBox::warning(this, "错误", "无法读取图片文件！");
        return false;
    }

    return true;
}

void disposewithdrwaw::updateImagePreview(const QString &filePath)
{
    if (!validateImage(filePath)) {
        return;
    }

    // 设置lineEdit文本
    ui->lineEdit->setText(filePath);

    // 加载并显示图片预览
    QPixmap pixmap(filePath);

    // 缩放图片以适应标签
    if (!pixmap.isNull()) {
        // 保持纵横比缩放
        QPixmap scaledPixmap = pixmap.scaled(ui->label_image->size() - QSize(10, 10),
                                             Qt::KeepAspectRatio,
                                             Qt::SmoothTransformation);
        ui->label_image->setPixmap(scaledPixmap);
        ui->label_image->setAlignment(Qt::AlignCenter);
    }
}

void disposewithdrwaw::on_pushButton_browse_clicked()
{
    // 打开文件对话框
    QString filePath = QFileDialog::getOpenFileName(
        this,
        "选择转账截图",
        QDir::homePath(),
        "图片文件 (*.jpg *.jpeg *.png *.bmp *.gif);;所有文件 (*.*)"
        );

    if (!filePath.isEmpty()) {
        updateImagePreview(filePath);
    }
}

void disposewithdrwaw::on_pushButton_check_clicked()
{
    // 验证输入
    QString imagePath = ui->lineEdit->text().trimmed();

    if (imagePath.isEmpty()) {
        QMessageBox::warning(this, "提示", "请选择转账截图！");
        return;
    }

    if (!validateImage(imagePath)) {
        return;
    }

    // 确认处理
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "确认处理",
                                  "确定要完成此提现处理吗？\n"
                                  "此操作将更新提现状态为已完成。",
                                  QMessageBox::Yes | QMessageBox::No);

    if (reply != QMessageBox::Yes) {
        return;
    }

    // 1. 备份图片到指定目录
    QString backupPath = backupImage(imagePath, currentWithdrawId);
    if (backupPath.isEmpty()) {
        QMessageBox::critical(this, "错误", "图片备份失败！");
        emit withdrawProcessed(currentWithdrawId, false);
        return;
    }

    // 2. 更新数据库状态
    if (!dbManager) {
        QMessageBox::critical(this, "错误", "数据库连接失败！");
        return;
    }

    QSqlQuery query;
    query.prepare(R"(
        UPDATE WithdrawRecords
        SET status = 'completed',
            update_time = datetime('now', 'localtime'),
            remark = :remark
        WHERE withdraw_id = :withdraw_id
    )");

    QString remark = QString("转账截图已保存: %1").arg(backupPath);
    query.bindValue(":withdraw_id", currentWithdrawId);
    query.bindValue(":remark", remark);

    if (!query.exec()) {
        qDebug() << "更新提现状态失败:" << query.lastError().text();
        QMessageBox::critical(this, "错误",
                              QString("更新提现状态失败:\n%1")
                                  .arg(query.lastError().text()));
        emit withdrawProcessed(currentWithdrawId, false);
        return;
    }

    qDebug() << "提现记录已处理完成:" << currentWithdrawId;
    qDebug() << "图片备份路径:" << backupPath;

    // 显示成功信息
    QMessageBox::information(this, "成功",
                             QString("提现处理已完成！\n"
                                     "图片已备份到:\n%1")
                                 .arg(backupPath));

    // 发出处理完成信号
    emit withdrawProcessed(currentWithdrawId, true);

    // 关闭对话框
    accept();
}
QString disposewithdrwaw::backupImage(const QString &originalPath, const QString &withdrawId)
{
    QFileInfo originalFile(originalPath);
    if (!originalFile.exists()) {
        qDebug() << "原始图片不存在:" << originalPath;
        return QString();
    }

    // 创建目标目录：当前目录/withdraw/提现ID/
    QDir currentDir = QDir::current();
    QString withdrawDir = "withdraw";

    // 创建 withdraw 目录（如果不存在）
    if (!currentDir.exists(withdrawDir)) {
        if (!currentDir.mkdir(withdrawDir)) {
            qDebug() << "创建 withdraw 目录失败";
            return QString();
        }
    }

    // 进入 withdraw 目录
    if (!currentDir.cd(withdrawDir)) {
        qDebug() << "进入 withdraw 目录失败";
        return QString();
    }

    // 创建以提现ID为名的子目录
    if (!currentDir.exists(withdrawId)) {
        if (!currentDir.mkdir(withdrawId)) {
            qDebug() << "创建提现ID目录失败:" << withdrawId;
            return QString();
        }
    }

    // 进入提现ID目录
    if (!currentDir.cd(withdrawId)) {
        qDebug() << "进入提现ID目录失败:" << withdrawId;
        return QString();
    }

    // 生成备份文件名（提现ID_时间戳.扩展名）
    QString timestamp = QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss");
    QString extension = originalFile.suffix();
    QString backupFileName = QString("%1_%2.%3")
                                 .arg(withdrawId)
                                 .arg(timestamp)
                                 .arg(extension);

    QString backupFilePath = currentDir.absoluteFilePath(backupFileName);

    // 复制文件
    QFile sourceFile(originalPath);
    if (!sourceFile.copy(backupFilePath)) {
        qDebug() << "复制文件失败:" << sourceFile.errorString();
        return QString();
    }

    qDebug() << "图片备份成功:";
    qDebug() << "  原始路径:" << originalPath;
    qDebug() << "  备份路径:" << backupFilePath;

    // 返回相对路径（相对于当前目录）
    QString relativePath = QString("withdraw/%1/%2")
                               .arg(withdrawId)
                               .arg(backupFileName);

    return relativePath;
}
