#ifndef LOGINWINDOW_H
#define LOGINWINDOW_H

#include <QMainWindow>
#include <QMap>
#include "mainwindow.h"


QT_BEGIN_NAMESPACE
namespace Ui {
class LoginWindow;
}
QT_END_NAMESPACE
class MainWindow;

class LoginWindow : public QMainWindow
{
    Q_OBJECT

public:
    LoginWindow(QWidget *parent = nullptr);
    ~LoginWindow();
    void resetFields();
    // Almacenar usuarios y contraseñas
    static QMap<QString, QString> registeredUsers;

private slots:
    void on_loginBtn_clicked();

private:
    Ui::LoginWindow *ui;
    MainWindow *mainWin;  // Agregar un puntero a MainWindow



};
#endif // LOGINWINDOW_H
