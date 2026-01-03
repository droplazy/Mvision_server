#include "guidetextset.h"
#include "ui_guidetextset.h"
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QFileDialog>
#include <QMessageBox>
#include <QStandardPaths>
#include <QTextStream>

#include <QDesktopServices>

guidetextset::guidetextset(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::guidetextset)
    , m_guideText("")
    , m_slogan1("")
    , m_slogan2("")
    , m_pGuideText(nullptr)
    , m_pSlogan1(nullptr)
    , m_pSlogan2(nullptr)
{
    ui->setupUi(this);

    // 设置窗口标题
    setWindowTitle("引导文本设置");

    // 初始化UI
    initUI();

    // 检查背景图片
    checkBackgroundImage();
}

guidetextset::~guidetextset()
{
    delete ui;
}

void guidetextset::setTextValues(QString* guideText, QString* slogan1, QString* slogan2)
{
    m_pGuideText = guideText;
    m_pSlogan1 = slogan1;
    m_pSlogan2 = slogan2;

    // 显示当前值
    if (m_pGuideText) {
        ui->textEdit_LOGIN_GUIDE_TEXT->setPlainText(*m_pGuideText);
        m_guideText = *m_pGuideText;
    }
    if (m_pSlogan1) {
        ui->textEdit_LOGIN_SLOGAN1->setPlainText(*m_pSlogan1);
        m_slogan1 = *m_pSlogan1;
    }
    if (m_pSlogan2) {
        ui->lineEdit_LOGIN_SLOGAN2->setText(*m_pSlogan2);
        m_slogan2 = *m_pSlogan2;
    }
}

void guidetextset::setCurrentValues(const QString& guideText, const QString& slogan1, const QString& slogan2)
{
    ui->textEdit_LOGIN_GUIDE_TEXT->setPlainText(guideText);
    ui->textEdit_LOGIN_SLOGAN1->setPlainText(slogan1);
    ui->lineEdit_LOGIN_SLOGAN2->setText(slogan2);

    m_guideText = guideText;
    m_slogan1 = slogan1;
    m_slogan2 = slogan2;
}

void guidetextset::initUI()
{
    // 设置背景路径文本框为只读
    ui->textEdit_backgroundpath->setReadOnly(true);

    // 设置文本编辑框的占位符提示
    ui->textEdit_LOGIN_GUIDE_TEXT->setPlaceholderText("请输入引导语...");
    ui->textEdit_LOGIN_SLOGAN1->setPlaceholderText("请输入宣传语1...");
    ui->lineEdit_LOGIN_SLOGAN2->setPlaceholderText("请输入宣传语2...");

    // 设置确定按钮为默认按钮
    ui->pushButton_ok->setDefault(true);

    // 设置背景路径文本框样式
    ui->textEdit_backgroundpath->setStyleSheet(
        "QTextEdit { background-color: #f5f5f5; border: 1px solid #ccc; padding: 5px; }"
        );
}

void guidetextset::checkBackgroundImage()
{
    // 使用与Upload目录相同的定位方式
    QDir currentDir(QDir::currentPath());

    // 修正：不要以/开头，使用filePath正确拼接路径
    QString bgImagePath = currentDir.filePath("images/login-bg.jpg");

    // 打印调试信息
    qDebug() << "当前目录:" << currentDir.absolutePath();
    qDebug() << "查找背景图片路径:" << bgImagePath;

    QFileInfo bgFileInfo(bgImagePath);

    if (bgFileInfo.exists() && bgFileInfo.isFile()) {
        // 文件存在，显示绝对路径
        ui->textEdit_backgroundpath->setText(bgImagePath);

        // 设置文本颜色为正常
        ui->textEdit_backgroundpath->setStyleSheet(
            "QTextEdit { background-color: #f5f5f5; border: 1px solid #ccc; padding: 5px; color: #006400; }"
            );
        qDebug() << "找到背景图片:" << bgImagePath;
    } else {
        // 文件不存在，显示提示信息
        ui->textEdit_backgroundpath->setText("没有放置背景图片");

        // 也检查根目录下的images文件夹
        QDir rootDir = QDir::root();
        QString rootBgPath = rootDir.filePath("images/login-bg.jpg");
        qDebug() << "也检查了根目录路径:" << rootBgPath;

        QFileInfo rootBgInfo(rootBgPath);
        if (rootBgInfo.exists() && rootBgInfo.isFile()) {
            qDebug() << "警告：在根目录下找到了login-bg.jpg，这可能不是预期的位置";
            qDebug() << "建议将图片移动到程序目录下的images文件夹中";
        }

        // 设置文本颜色为红色以提醒
        ui->textEdit_backgroundpath->setStyleSheet(
            "QTextEdit { background-color: #f5f5f5; border: 1px solid #ccc; padding: 5px; color: #ff0000; font-weight: bold; }"
            );
    }
}

QString guidetextset::getCurrentDirPath()
{
    QDir currentDir(QDir::currentPath());
    return currentDir.absolutePath();
}

void guidetextset::on_pushButton_browse_clicked()
{
    // 使用与Upload目录相同的定位方式
    QDir currentDir(QDir::currentPath());
    QString imagesDirPath = currentDir.filePath("images");

    // 确保images目录存在，不存在则创建
    if (!QDir(imagesDirPath).exists()) {
        if (QDir().mkdir(imagesDirPath)) {
            qDebug() << "创建images目录:" << imagesDirPath;
        } else {
            QMessageBox::warning(this, "错误",
                                 "无法创建images目录！\n"
                                 "路径: " + imagesDirPath);
            return;
        }
    }

    // 检查目录是否真的存在
    if (!QDir(imagesDirPath).exists()) {
        QMessageBox::warning(this, "错误",
                             "images目录不存在且无法创建！\n"
                             "路径: " + imagesDirPath);
        return;
    }

    // 使用系统默认程序打开images文件夹
    QUrl folderUrl = QUrl::fromLocalFile(imagesDirPath);

    if (QDesktopServices::openUrl(folderUrl)) {
        qDebug() << "成功打开images文件夹:" << imagesDirPath;

        // 提示用户
        QMessageBox::information(this, "提示",
                                 "已打开images文件夹。\n\n"
                                 "请将名为 'login-bg.jpg' 的背景图片放入此文件夹。\n"
                                 "文件夹路径: " + imagesDirPath + "\n\n"
                                                       "放置图片后，请重新打开本窗口以刷新显示。");
    } else {
        QMessageBox::warning(this, "错误",
                             "无法打开images文件夹！\n"
                             "路径: " + imagesDirPath + "\n\n"
                                                   "请手动打开该目录。");
    }
}

void guidetextset::on_pushButton_ok_clicked()
{
    // 获取用户输入的文本
    QString guideText = ui->textEdit_LOGIN_GUIDE_TEXT->toPlainText().trimmed();
    QString slogan1 = ui->textEdit_LOGIN_SLOGAN1->toPlainText().trimmed();
    QString slogan2 = ui->lineEdit_LOGIN_SLOGAN2->toPlainText().trimmed();

    // 验证输入
    if (guideText.isEmpty()) {
        QMessageBox::warning(this, "警告", "引导语不能为空！");
        ui->textEdit_LOGIN_GUIDE_TEXT->setFocus();
        return;
    }

    if (slogan1.isEmpty()) {
        QMessageBox::warning(this, "警告", "宣传语1不能为空！");
        ui->textEdit_LOGIN_SLOGAN1->setFocus();
        return;
    }

    if (slogan2.isEmpty()) {
        QMessageBox::warning(this, "警告", "宣传语2不能为空！");
        ui->lineEdit_LOGIN_SLOGAN2->setFocus();
        return;
    }

    // 保存到成员变量
    m_guideText = guideText;
    m_slogan1 = slogan1;
    m_slogan2 = slogan2;

    // 如果提供了指针，更新原始值
    if (m_pGuideText) *m_pGuideText = guideText;
    if (m_pSlogan1) *m_pSlogan1 = slogan1;
    if (m_pSlogan2) *m_pSlogan2 = slogan2;

    // 提示保存成功
    QMessageBox::information(this, "成功",
                             "引导文本已保存！\n\n"
                             "引导语: " + guideText + "\n"
                                               "宣传语1: " + slogan1 + "\n"
                                             "宣传语2: " + slogan2);

    // 关闭窗口
    this->accept();
}
