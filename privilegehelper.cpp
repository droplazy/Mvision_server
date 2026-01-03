#include "privilegehelper.h"

#ifdef Q_OS_WIN
#include <windows.h>
#include <shellapi.h>
#include <QApplication>

#pragma comment(lib, "shell32.lib")
#endif

bool PrivilegeHelper::checkAdminRights()
{
#ifdef Q_OS_WIN
    BOOL isAdmin = FALSE;
    PSID adminGroup = NULL;

    SID_IDENTIFIER_AUTHORITY ntAuthority = SECURITY_NT_AUTHORITY;
    if (AllocateAndInitializeSid(&ntAuthority, 2,
                                 SECURITY_BUILTIN_DOMAIN_RID,
                                 DOMAIN_ALIAS_RID_ADMINS,
                                 0, 0, 0, 0, 0, 0,
                                 &adminGroup)) {
        if (!CheckTokenMembership(NULL, adminGroup, &isAdmin)) {
            isAdmin = FALSE;
        }
        FreeSid(adminGroup);
    }
    return isAdmin == TRUE;
#else
    // Linux/macOS: 检查是否为root
    return geteuid() == 0;
#endif
}

bool PrivilegeHelper::restartWithAdmin()
{
#ifdef Q_OS_WIN
    wchar_t modulePath[MAX_PATH];
    GetModuleFileNameW(NULL, modulePath, MAX_PATH);

    SHELLEXECUTEINFOW sei = { sizeof(sei) };
    sei.lpVerb = L"runas";
    sei.lpFile = modulePath;
    sei.hwnd = NULL;
    sei.nShow = SW_NORMAL;

    return ShellExecuteExW(&sei) == TRUE;
#else
    // Linux/macOS: 使用pkexec或gksu
    QString program = "pkexec";  // 或 "gksu", "kdesu"
    QStringList arguments;
    arguments << QApplication::applicationFilePath();

    return QProcess::startDetached(program, arguments);
#endif
}

bool PrivilegeHelper::requireAdministrator()
{
    if (checkAdminRights()) {
        return true; // 已经是管理员
    }

    // 提示用户
    QMessageBox msgBox;
    msgBox.setWindowTitle(QObject::tr("需要管理员权限"));
    msgBox.setText(QObject::tr("此程序需要管理员权限才能正常运行。"));
    msgBox.setInformativeText(QObject::tr("是否以管理员权限重新启动？"));
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msgBox.setDefaultButton(QMessageBox::Yes);
    msgBox.setIcon(QMessageBox::Question);

    if (msgBox.exec() == QMessageBox::Yes) {
        if (restartWithAdmin()) {
            QApplication::exit(0); // 退出当前实例
            return true;
        } else {
            QMessageBox::critical(nullptr,
                                  QObject::tr("错误"),
                                  QObject::tr("无法以管理员权限启动程序。"));
        }
    }

    return false; // 用户拒绝或启动失败
}
