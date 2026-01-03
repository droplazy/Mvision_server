#ifndef GUIDETEXTSET_H
#define GUIDETEXTSET_H

#include <QDialog>
#include <QString>

namespace Ui {
class guidetextset;
}

class guidetextset : public QDialog
{
    Q_OBJECT

public:
    explicit guidetextset(QWidget *parent = nullptr);
    ~guidetextset();

    // 设置MainWindow的成员变量指针
    void setTextValues(QString* guideText, QString* slogan1, QString* slogan2);

    // 设置当前值显示
    void setCurrentValues(const QString& guideText, const QString& slogan1, const QString& slogan2);

    // 获取用户输入的值
    QString getGuideText() const { return m_guideText; }
    QString getSlogan1() const { return m_slogan1; }
    QString getSlogan2() const { return m_slogan2; }

private slots:
    void on_pushButton_browse_clicked();
    void on_pushButton_ok_clicked();

private:
    // 初始化UI
    void initUI();

    // 检查背景图片
    void checkBackgroundImage();

    // 获取当前目录路径
    QString getCurrentDirPath();

private:
    Ui::guidetextset *ui;

    // 保存用户输入的值
    QString m_guideText;
    QString m_slogan1;
    QString m_slogan2;

    // 指向MainWindow成员变量的指针
    QString* m_pGuideText;
    QString* m_pSlogan1;
    QString* m_pSlogan2;
};

#endif // GUIDETEXTSET_H
