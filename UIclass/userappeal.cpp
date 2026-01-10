#include "userappeal.h"
#include "ui_userappeal.h"
#include <QDebug>
#include <QMessageBox>
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QDesktopServices>
#include <QFileDialog>
#include <QImageReader>
#include <QFileInfo>
#include <QStandardPaths>
#include <QProcess>
#include <QListView>
#include <QModelIndex>

userappeal::userappeal(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::userappeal)
    , m_currentUser("")
    , m_appealDirPath("")
    , m_folderModel(nullptr)
    , m_currentImageIndex(-1)
{
    ui->setupUi(this);

    // 设置窗口标题
    setWindowTitle("用户投诉处理");

    // 初始化投诉目录
    initAppealDir();

    // 创建数据模型
    m_folderModel = new QStringListModel(this);
    ui->listView->setModel(m_folderModel);

    // 设置textEdit为只读
    ui->textEdit->setReadOnly(true);

    // 设置label_pic属性
    ui->label_pic->setAlignment(Qt::AlignCenter);
    ui->label_pic->setStyleSheet("QLabel { background-color: #f0f0f0; border: 1px solid #ccc; }");

    // 初始加载投诉文件夹
    loadAppealFolders();

    // 连接信号槽
    connect(ui->listView, &QListView::doubleClicked, this, &userappeal::on_listView_doubleClicked);

    // 初始禁用导航按钮
    ui->previos->setEnabled(false);
    ui->next->setEnabled(false);
    ui->open->setEnabled(false);
}

userappeal::~userappeal()
{
    delete m_folderModel;
    delete ui;
}

void userappeal::setCurrentUser(const QString &username)
{
    m_currentUser = username;
    // 重新加载，可以按用户筛选
    loadAppealFolders();
}

void userappeal::initAppealDir()
{
    // 使用和Upload目录相同的定位方式
    QDir currentDir(QDir::currentPath());
    m_appealDirPath = currentDir.filePath("UserAppeal");

    // 检查目录是否存在，不存在则创建
    if (!QDir(m_appealDirPath).exists()) {
        if (QDir().mkdir(m_appealDirPath)) {
            qDebug() << "创建投诉目录:" << m_appealDirPath;
        } else {
            qDebug() << "创建投诉目录失败:" << m_appealDirPath;
        }
    }
}

void userappeal::loadAppealFolders()
{
    QDir appealDir(m_appealDirPath);

    // 检查目录是否存在
    if (!appealDir.exists()) {
        ui->textEdit->setText(QString("找不到投诉目录: %1").arg(m_appealDirPath));
        qDebug() << "投诉目录不存在:" << m_appealDirPath;

        // 清空文件夹列表
        m_folderModel->setStringList(QStringList());
        m_appealFolders.clear();
        return;
    }

    // 获取所有文件夹
    QStringList filters;
    filters << "*";

    QFileInfoList folderList = appealDir.entryInfoList(filters, QDir::Dirs | QDir::NoDotAndDotDot);

    m_appealFolders.clear();

    // 筛选文件夹（可以根据用户筛选）
    foreach (const QFileInfo &folderInfo, folderList) {
        QString folderName = folderInfo.fileName();

        // 如果需要按用户筛选
        if (!m_currentUser.isEmpty()) {
            // 假设文件夹格式为 "用户名_投诉ID" 或包含用户名
            if (folderName.contains(m_currentUser, Qt::CaseInsensitive)) {
                m_appealFolders.append(folderName);
            }
        } else {
            // 不筛选，显示所有
            m_appealFolders.append(folderName);
        }
    }

    // 按名称排序（最新的在前面）
    m_appealFolders.sort();
    std::reverse(m_appealFolders.begin(), m_appealFolders.end());

    // 更新模型
    m_folderModel->setStringList(m_appealFolders);

    // 如果没有投诉文件夹，显示提示
    if (m_appealFolders.isEmpty()) {
        ui->textEdit->setText(QString("投诉目录为空: %1").arg(m_appealDirPath));
        qDebug() << "没有找到投诉文件夹";
    }
}

void userappeal::loadAppealContent(const QString &folderName)
{
    if (folderName.isEmpty()) {
        ui->textEdit->setText("请选择有效的投诉文件夹");
        return;
    }

    m_currentFolder = folderName;
    QString folderPath = m_appealDirPath + "/" + folderName;
    QDir folderDir(folderPath);

    if (!folderDir.exists()) {
        ui->textEdit->setText(QString("文件夹不存在: %1/%2").arg(m_appealDirPath).arg(folderName));
        ui->label_pic->setText("文件夹不存在");
        ui->previos->setEnabled(false);
        ui->next->setEnabled(false);
        return;
    }

    // 查找txt文件
    QStringList txtFiles = folderDir.entryList(QStringList() << "*.txt", QDir::Files);

    if (txtFiles.isEmpty()) {
        ui->textEdit->setText(QString("投诉单: %1\n\n文件夹中没有找到文本文件").arg(folderName));
    } else {
        // 读取第一个txt文件
        QString txtFilePath = folderPath + "/" + txtFiles.first();
        QString content = readTextFile(txtFilePath);

        // 显示内容
        QString displayText = QString("投诉单: %1\n\n文件: %2\n\n内容:\n%3")
                                  .arg(folderName)
                                  .arg(txtFiles.first())
                                  .arg(content);

        ui->textEdit->setText(displayText);
    }

    // 加载图片
    loadImages(folderPath);

    // 更新导航按钮状态
    updateNavigationButtons();
}

QString userappeal::readTextFile(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return QString("无法打开文件: %1\n错误: %2").arg(filePath).arg(file.errorString());
    }

    QTextStream in(&file);
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    in.setEncoding(QStringConverter::Utf8);
#else
    in.setCodec("UTF-8");
#endif

    QString content = in.readAll();
    file.close();

    return content;
}

void userappeal::loadImages(const QString &folderPath)
{
    QDir folderDir(folderPath);

    // 查找图片文件
    QStringList imageFilters;
    imageFilters << "*.jpg" << "*.jpeg" << "*.png" << "*.bmp" << "*.gif" << "*.webp";
    m_imageFiles = folderDir.entryList(imageFilters, QDir::Files);

    // 按名称排序
    m_imageFiles.sort();

    // 重置图片索引
    m_currentImageIndex = -1;

    if (m_imageFiles.isEmpty()) {
        ui->label_pic->setText("没有找到图片文件");
        ui->label_pic->setPixmap(QPixmap());
    } else {
        // 加载第一张图片
        m_currentImageIndex = 0;
        loadCurrentImage();
    }
}

void userappeal::loadCurrentImage()
{
    if (m_currentImageIndex < 0 || m_currentImageIndex >= m_imageFiles.size()) {
        ui->label_pic->setText("没有图片");
        return;
    }

    QString imagePath = m_appealDirPath + "/" + m_currentFolder + "/" + m_imageFiles[m_currentImageIndex];

    // 使用QImageReader处理更多格式
    QImageReader reader(imagePath);
    reader.setAutoTransform(true);
    QImage image = reader.read();

    if (image.isNull()) {
        ui->label_pic->setText(QString("无法加载图片:\n%1\n错误: %2")
                                   .arg(m_imageFiles[m_currentImageIndex])
                                   .arg(reader.errorString()));
    } else {
        // 调整图片大小以适应label
        QPixmap pixmap = QPixmap::fromImage(image);
        int labelWidth = ui->label_pic->width() - 10;
        int labelHeight = ui->label_pic->height() - 10;

        // 如果label尺寸有效，则缩放图片
        if (labelWidth > 10 && labelHeight > 10) {
            // 保持纵横比缩放
            QPixmap scaledPixmap = pixmap.scaled(labelWidth, labelHeight,
                                                 Qt::KeepAspectRatio, Qt::SmoothTransformation);
            ui->label_pic->setPixmap(scaledPixmap);
        } else {
            // 直接设置原图
            ui->label_pic->setPixmap(pixmap);
        }

        // 更新状态信息
        qDebug() << QString("显示图片 %1/%2: %3 (%4x%5)")
                        .arg(m_currentImageIndex + 1)
                        .arg(m_imageFiles.size())
                        .arg(m_imageFiles[m_currentImageIndex])
                        .arg(image.width())
                        .arg(image.height());
    }
}

void userappeal::updateNavigationButtons()
{
    bool hasImages = !m_imageFiles.isEmpty();
    bool hasPrevious = hasImages && (m_currentImageIndex > 0);
    bool hasNext = hasImages && (m_currentImageIndex < m_imageFiles.size() - 1);

    ui->previos->setEnabled(hasPrevious);
    ui->next->setEnabled(hasNext);
    ui->open->setEnabled(!m_currentFolder.isEmpty());
}

void userappeal::on_listView_doubleClicked(const QModelIndex &index)
{
    if (!index.isValid()) {
        return;
    }

    QString folderName = index.data().toString();
    if (folderName.isEmpty()) {
        return;
    }

    // 加载投诉单内容
    loadAppealContent(folderName);
}

void userappeal::on_next_clicked()
{
    if (m_imageFiles.isEmpty()) {
        return;
    }

    if (m_currentImageIndex < m_imageFiles.size() - 1) {
        m_currentImageIndex++;
        loadCurrentImage();
        updateNavigationButtons();
    }
}

void userappeal::on_previos_clicked()
{
    if (m_imageFiles.isEmpty()) {
        return;
    }

    if (m_currentImageIndex > 0) {
        m_currentImageIndex--;
        loadCurrentImage();
        updateNavigationButtons();
    }
}

void userappeal::on_open_clicked()
{
    if (m_currentFolder.isEmpty()) {
        QMessageBox::information(this, "提示", "请先选择一个投诉单");
        return;
    }

    QString folderPath = m_appealDirPath + "/" + m_currentFolder;
    QDir folderDir(folderPath);

    if (!folderDir.exists()) {
        QMessageBox::warning(this, "错误", QString("文件夹不存在: %1").arg(folderPath));
        return;
    }

    // 使用系统默认程序打开文件夹
    QUrl folderUrl = QUrl::fromLocalFile(folderPath);

    // 跨平台方式打开文件夹
    if (!QDesktopServices::openUrl(folderUrl)) {
        QMessageBox::warning(this, "错误", QString("无法打开文件夹: %1").arg(folderPath));
    }
}

// 添加resizeEvent函数，当窗口大小改变时重新调整图片
void userappeal::resizeEvent(QResizeEvent *event)
{
 //   QWidget::resizeEvent(event);

    // 如果当前有图片显示，重新加载以调整大小
    if (m_currentImageIndex >= 0 && m_currentImageIndex < m_imageFiles.size()) {
        loadCurrentImage();
    }
}
