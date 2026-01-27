#ifndef LIVINGCONTROL_H
#define LIVINGCONTROL_H

#include <QDialog>

namespace Ui {
class livingcontrol;
}

class livingcontrol : public QDialog
{
    Q_OBJECT

public:
    explicit livingcontrol(QWidget *parent = nullptr);
    ~livingcontrol();

private:
    Ui::livingcontrol *ui;
};

#endif // LIVINGCONTROL_H
