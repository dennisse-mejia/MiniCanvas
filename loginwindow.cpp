#include "loginwindow.h"
#include "./ui_loginwindow.h"
#include "mainwindow.h"
#include <QMessageBox>
#include "loginwindow.h"
#include "./ui_loginwindow.h"
#include "maestros.h"
#include "mainwindow.h"  // Esto asegura que registeredTeachers está bien referenciado


QMap<QString, QString> LoginWindow::registeredUsers = { {"Admin", "admin"} }; // Usuario por defecto



LoginWindow::LoginWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::LoginWindow)
{
    ui->setupUi(this);
    MainWindow *mainWin = new MainWindow();
    mainWin->cargarMaestrosDesdeArchivo();

}

LoginWindow::~LoginWindow()
{
    delete ui;
}

void LoginWindow::on_loginBtn_clicked()
{

    QString username = ui->usernameLineEdit->text().trimmed();
    QString password = ui->passwordLineEdit->text().trimmed();
    QString userType = ui->userTypeComboBox->currentText().trimmed();

    // 🔍 Verificar que los datos se han cargado correctamente
    qDebug() << "Usuarios registrados: " << MainWindow::registeredTeachers.keys();
    qDebug() << "Usuario ingresado: " << username;
    qDebug() << "Contraseña ingresada: " << password;

    // Verificar que el usuario y contraseña son correctos
    if (userType == "Registro") {
        // Validar usuario de tipo "Registro"
        if (registeredUsers.contains(username) && registeredUsers[username] == password) {
            QMessageBox::information(this, "Inicio de Sesión", "¡Bienvenido, " + username + "!");
            mainWin = new MainWindow();
            mainWin->setWelcomeMessage(username, userType);
            mainWin->show();
            this->close();
        } else {
            QMessageBox::warning(this, "Error", "Usuario o contraseña incorrectos.");
        }
    }
    else if (userType == "Maestros") {
            userType = "Maestro";
        if (MainWindow::registeredTeachers.contains(username) &&
            MainWindow::registeredTeachers[username] == password) {
            QMessageBox::information(this, "Inicio de Sesión", "¡Bienvenido Maestro " + username + "!");
            maestros *maestroWin = new maestros();
            maestroWin->setWelcomeMessage(username);
            maestroWin->show();
            this->close();
        } else {
            QMessageBox::warning(this, "Error", "Usuario o contraseña incorrectos.");
        }
    }
    else {
        QMessageBox::warning(this, "Error", "Solo los usuarios tipo 'Registro' o 'Maestro' pueden acceder.");
    }

}

void LoginWindow::resetFields()
{
    ui->usernameLineEdit->clear();
    ui->passwordLineEdit->clear();
    ui->userTypeComboBox->setCurrentIndex(0); // Seleccionar el primer elemento
}


