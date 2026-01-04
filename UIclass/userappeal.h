#ifndef USERAPPEAL_H
#define USERAPPEAL_H

#include <QDialog>
#include <QFileSystemModel>
#include <QStringListModel>
#include <QImage>
#include <QDir>

namespace Ui {
class userappeal;
}

class userappeal : public QDialog
{
    Q_OBJECT

public:
    explicit userappeal(QWidget *parent = nullptr);
    ~userappeal();

    // 设置当前用户（如果需要根据用户筛选）
    void setCurrentUser(const QString &username);

private slots:
    void on_next_clicked();
    void on_previos_clicked();
    void on_open_clicked();

    // listView双击事件
    void on_listView_doubleClicked(const QModelIndex &index);

    // 图片加载完成
   // void onImageLoaded();

private:
    // 初始化
    void initAppealDir();
    void loadAppealFolders();

    // 加载投诉单内容
    void loadAppealContent(const QString &folderName);

    // 加载图片
    void loadImages(const QString &folderPath);
    void loadCurrentImage();

    // 更新UI状态
    void updateNavigationButtons();

    // 获取文件内容
    QString readTextFile(const QString &filePath);

private:
    Ui::userappeal *ui;
    QString m_currentUser;           // 当前用户
    QString m_appealDirPath;         // 投诉目录路径
    QStringListModel *m_folderModel; // listView数据模型
    QStringList m_appealFolders;     // 投诉文件夹列表
    QString m_currentFolder;         // 当前选中的投诉文件夹
    QStringList m_imageFiles;        // 当前文件夹中的图片文件
    int m_currentImageIndex;         // 当前显示图片索引
    QImage m_currentImage;           // 当前显示的图片
   // void resizeEvent(QResizeEvent *event);
    void resizeEvent(QResizeEvent *event);
};

#endif // USERAPPEAL_H
