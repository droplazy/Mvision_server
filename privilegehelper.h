#ifndef PRIVILEGEHELPER_H
#define PRIVILEGEHELPER_H

#include <QObject>
#include <QMessageBox>

class PrivilegeHelper : public QObject
{
    Q_OBJECT
public:
    static bool requireAdministrator();

private:
    static bool checkAdminRights();
    static bool restartWithAdmin();
};

#endif // PRIVILEGEHELPER_H
