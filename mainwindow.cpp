#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QMessageBox>
#include "loginwindow.h"
#include "ui_loginwindow.h"
#include <QSet>
#include <QFile>
#include <QTextStream>
#include <QString>
#include <QWidget>
#include "maestros.h"
#include <QTableWidget>
#include "asignaciones.h"




QMap<QString, QString> MainWindow::nombresMaestros = {};
QMap<QString, QString> MainWindow::profesionesMaestros = {};
QMap<QString, QString> MainWindow::sueldosMaestros = {};
QMap<QString, QString> MainWindow::rolesMaestros = {};



// 🔹 Definir variables estáticas aquí para evitar "undefined reference"
QMap<QString, QString> MainWindow::registeredTeachers = {};
QMap<QString, QString> MainWindow::registeredTeacherIDs = {};


QMap<QString, QString> MainWindow::registeredAlumnos = {};
QMap<QString, QString> MainWindow::cuentasAlumnos = {};
QMap<QString, QString> MainWindow::nombresAlumnos = {};
QMap<QString, QString> MainWindow::carrerasAlumnos = {};
QMap<QString, QString> MainWindow::clasesCursadasAlumnos = {};
QMap<QString, QString> MainWindow::rolesAlumnos = {};

// 🔹 Definir `QMap` para clases
QMap<QString, QString> MainWindow::registeredClases = {};
QMap<QString, QString> MainWindow::horaClases = {};
QMap<QString, QString> MainWindow::unidadesValorativasClases = {};
QMap<QString, QString> MainWindow::semestreClases = {};
QMap<QString, QString> MainWindow::periodoClases = {};
QMap<QString, QString> MainWindow::yearClases = {};



MainWindow::MainWindow(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);


    configurarTablaMaestros(); // 🔹 Configurar la tabla
    cargarMaestrosEnTabla();
    cargarMaestrosDesdeArchivo();

    cargarAlumnosDesdeArchivo(); // 🔹 Cargar los alumnos desde el archivo al iniciar
    cargarAlumnosEnTabla();

    cargarClasesDesdeArchivo();
    cargarClasesEnTabla();


    // Asegurar que el label defaultUserLabel siempre tenga "Registro"
    ui->defaultUserLabel->setText("Registro");

    disconnect(ui->crearRegistroUserBtn, nullptr, nullptr, nullptr); // Elimina conexiones duplicadas

    // Conectar el botón de crear usuario admin
    connect(ui->crearRegistroUserBtn, &QPushButton::clicked, this, &MainWindow::on_crearRegistroUserBtn_clicked);


}


MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setWelcomeMessage(const QString &username, const QString &userType)
{
    // Guardar los valores en variables de la clase (si es necesario)
    this->username = username;
    this->userType = userType;


    // Mostrar los valores en los QLabel
    ui->usernameMyAcc->setText(username);
    ui->userTypeMyAcc->setText(userType);


    // Mostrar "Mi Cuenta" después del login
    ui->stackedWidget->setCurrentIndex(0);
}



void MainWindow::on_crearRegistroUserBtn_clicked()
{

    QString username = ui->registroUserLineE->text().trimmed();
    QString password = ui->passwordRegistroLineE->text().trimmed();
    QString userType = "Registro";  // Por defecto

    // Validar que los campos no estén vacíos
    if (username.isEmpty() || password.isEmpty()) {
        QMessageBox::warning(this, "Error", "Por favor, complete todos los campos.");
        return;
    }

    // Validar si el usuario ya existe
    if (LoginWindow::registeredUsers.contains(username)) {
        QMessageBox::warning(this, "Error", "El usuario ya está registrado.");
        return;
    }

    // Agregar el nuevo usuario a la lista global
    LoginWindow::registeredUsers.insert(username, password);


    // Simulación de almacenamiento (podrías guardar en una base de datos o archivo)
    QMessageBox::information(this, "Usuario Creado",
                             "Se ha creado el usuario '" + username + "' con éxito.\n"
                                                                      "Tipo de usuario: " + userType);

    // Limpiar los campos después de la creación
    ui->registroUserLineE->clear();
    ui->passwordRegistroLineE->clear();
}


void MainWindow::on_quitBtn_clicked()
{
    // Mostrar la ventana de login si está oculta
    foreach(QWidget *widget, QApplication::topLevelWidgets()) {
        LoginWindow *loginWin = qobject_cast<LoginWindow*>(widget);
        if (loginWin) {
            loginWin->resetFields();
            loginWin->show();
            break;
        }
    }

    this->close(); // Cerrar la ventana actual (MainWindow)

}

// MAESTROS

void MainWindow::configurarTablaMaestros()
{
    // Definir el número de columnas
    ui->tableWidgetTeacher->setColumnCount(7);

    // Establecer encabezados de columna
    QStringList headers = {"Username", "ID", "Nombre", "Profesión", "Sueldo", "Contraseña", "Rol"};
    ui->tableWidgetTeacher->setHorizontalHeaderLabels(headers);

    // Ajustar el tamaño de las columnas para que se acomoden al contenido
    ui->tableWidgetTeacher->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    // No permitir que las celdas sean editables directamente
    ui->tableWidgetTeacher->setEditTriggers(QAbstractItemView::NoEditTriggers);

    // Permitir seleccionar filas completas
    ui->tableWidgetTeacher->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableWidgetTeacher->setSelectionMode(QAbstractItemView::SingleSelection);
}

void MainWindow::cargarMaestrosEnTabla()
{
    // Asegurar que la tabla está vacía antes de llenarla
    ui->tableWidgetTeacher->setRowCount(0);

    // Obtener la cantidad de maestros registrados
    int row = 0;
    for (const QString &username : registeredTeachers.keys()) {
        ui->tableWidgetTeacher->insertRow(row); // Agregar una nueva fila

        ui->tableWidgetTeacher->setItem(row, 0, new QTableWidgetItem(username));
        ui->tableWidgetTeacher->setItem(row, 1, new QTableWidgetItem(registeredTeacherIDs.value(username, "")));
        ui->tableWidgetTeacher->setItem(row, 2, new QTableWidgetItem(nombresMaestros.value(username, "")));
        ui->tableWidgetTeacher->setItem(row, 3, new QTableWidgetItem(profesionesMaestros.value(username, "")));
        ui->tableWidgetTeacher->setItem(row, 4, new QTableWidgetItem(sueldosMaestros.value(username, "")));
        ui->tableWidgetTeacher->setItem(row, 5, new QTableWidgetItem(registeredTeachers.value(username, ""))); // Contraseña
        ui->tableWidgetTeacher->setItem(row, 6, new QTableWidgetItem(rolesMaestros.value(username, "")));

        row++;
    }
}

void MainWindow::guardarMaestrosEnArchivo() {
    QFile file("maestros.txt");
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qDebug() << "No se pudo abrir el archivo maestros.txt para escritura.";
        return;
    }

    QTextStream out(&file);

    for (auto it = registeredTeachers.begin(); it != registeredTeachers.end(); ++it) {
        QString usuario = it.key();
        QString password = it.value();
        QString id = registeredTeacherIDs.value(usuario, "");
        QString nombre = nombresMaestros.value(usuario, "");
        QString profesion = profesionesMaestros.value(usuario, "");
        QString sueldo = sueldosMaestros.value(usuario, "");
        QString rol = "Maestro"; // Siempre fijo

        out << usuario << "," << id << "," << nombre << "," << profesion << ","
            << sueldo << "," << password << "," << rol << "\n";
    }
    file.close();
    actualizarAsignaciones();
}




void MainWindow::cargarMaestrosDesdeArchivo() {
    QFile file("maestros.txt");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "Error: No se pudo abrir maestros.txt";
        return;
    }

    QTextStream in(&file);
    registeredTeachers.clear();
    registeredTeacherIDs.clear();
    nombresMaestros.clear();
    profesionesMaestros.clear();
    sueldosMaestros.clear();
    rolesMaestros.clear();

    while (!in.atEnd()) {
        QString line = in.readLine().trimmed(); // Eliminamos espacios extra

        // Verificar que la línea no esté vacía antes de procesarla
        if (line.isEmpty()) {
            qDebug() << " Línea vacía detectada, saltando...";
            continue;
        }

        QStringList parts = line.split(",");

        if (parts.size() == 7) {  // Verificar que haya 7 elementos
            QString usuario = parts[0].trimmed();
            QString id = parts[1].trimmed();
            QString nombre = parts[2].trimmed();
            QString profesion = parts[3].trimmed();
            QString sueldo = parts[4].trimmed();
            QString password = parts[5].trimmed();
            QString rol = parts[6].trimmed();

            // Si el usuario o contraseña están vacíos, ignoramos la línea
            if (usuario.isEmpty() || password.isEmpty()) {
                qDebug() << "Datos inválidos detectados en línea: " << line;
                continue;
            }

            // Insertar en los mapas solo si los datos son válidos
            registeredTeacherIDs.insert(usuario, id);
            registeredTeachers.insert(usuario, password);
            nombresMaestros.insert(usuario, nombre);
            profesionesMaestros.insert(usuario, profesion);
            sueldosMaestros.insert(usuario, sueldo);
            rolesMaestros.insert(usuario, rol);

            qDebug() << "Maestro cargado: " << usuario << " ID: " << id;
        } else {
            qDebug() << "Línea mal formada en el archivo: " << line;
        }
    }
    file.close();

}


void MainWindow::on_crearMaestroBtn_clicked() {
    QString id = ui->IDMaestroLineEdit->text().trimmed();
    QString nombre = ui->maestroNamelineEdit->text().trimmed();
    QString profesion = ui->profesionComboBox->currentText();
    QString sueldo = ui->sueldoLineEdit->text().trimmed();
    QString username = ui->usernameMaestroLineEdit->text().trimmed();
    QString password = ui->passwordMaestroLineEdit->text().trimmed();
    QString rol = "Maestro";  // 🔹 Asegurar que siempre se guarde "Maestro"

    // Validaciones
    if (id.isEmpty() || nombre.isEmpty() || profesion.isEmpty() || sueldo.isEmpty() ||
        username.isEmpty() || password.isEmpty()) {
        QMessageBox::warning(this, "Error", "Todos los campos son obligatorios.");
        return;
    }

    // Validar que el sueldo solo contenga números
    bool sueldoEsNumero = false;
    sueldo.toInt(&sueldoEsNumero);
    if (!sueldoEsNumero) {
        QMessageBox::warning(this, "Error", "El sueldo debe contener solo números.");
        return;
    }


    // Validar que el ID sea único
    if (registeredTeacherIDs.contains(id)) {
        QMessageBox::warning(this, "Error", "El ID ya está registrado.");
        return;
    }

    // Validar que el username sea único
    if (registeredTeachers.contains(username)) {
        QMessageBox::warning(this, "Error", "El nombre de usuario ya está registrado.");
        return;
    }

    // Guardar en los mapas de datos
    registeredTeacherIDs.insert(username, id);
    registeredTeachers.insert(username, password);
    nombresMaestros.insert(username, nombre);
    profesionesMaestros.insert(username, profesion);
    sueldosMaestros.insert(username, sueldo);
    rolesMaestros.insert(username, rol);

    // 🔹 Guardar en el archivo
    guardarMaestrosEnArchivo();

    QMessageBox::information(this, "Éxito", "Nuevo maestro registrado correctamente.");

    limpiarCamposNuevoMaestro();
}

void MainWindow::limpiarCamposNuevoMaestro() {
    ui->IDMaestroLineEdit->clear();
    ui->maestroNamelineEdit->clear();
    ui->sueldoLineEdit->clear();
    ui->usernameMaestroLineEdit->clear();
    ui->passwordMaestroLineEdit->clear();
}


void MainWindow::on_miCuentaBtn_clicked()
{
    ui->stackedWidget->setCurrentWidget(ui->page);
}


void MainWindow::on_goToPage_2_clicked()
{
    ui->stackedWidget->setCurrentWidget(ui->page_2);

}


void MainWindow::on_maestrosBtn_clicked()
{
    ui->stackedWidget->setCurrentWidget(ui->page_5); // Muestra la tabla de maestros
    cargarMaestrosEnTabla();
}


void MainWindow::on_addTeacherBtn_clicked()
{
    ui->rolMaestroDefaultLabel->setText("Maestro");
    ui->stackedWidget->setCurrentWidget(ui->page_3); // Página para agregar maestro

}


void MainWindow::on_modifyTeacherBtn_clicked()
{
    // Obtener la fila seleccionada en tableWidgetTeacher
    int row = ui->tableWidgetTeacher->currentRow();

    // Si no se ha seleccionado ninguna fila, mostrar un mensaje y salir
    if (row == -1) {
        QMessageBox::warning(this, "Error", "Por favor, seleccione un maestro de la lista antes de modificar.");
        return;
    }

    // Obtener datos de la fila seleccionada
    QString username = ui->tableWidgetTeacher->item(row, 0)->text(); // Columna de Username
    QString id = ui->tableWidgetTeacher->item(row, 1)->text();
    QString nombre = ui->tableWidgetTeacher->item(row, 2)->text();
    QString profesion = ui->tableWidgetTeacher->item(row, 3)->text();
    QString sueldo = ui->tableWidgetTeacher->item(row, 4)->text();
    QString password = ui->tableWidgetTeacher->item(row, 5)->text();
    QString rol = ui->tableWidgetTeacher->item(row, 6)->text(); // Siempre será "Maestro"

    // Cargar los datos en los campos de page_4
    ui->IDMaestroLineEdit_2->setText(id);
    ui->maestroNamelineEdit_2->setText(nombre);
    ui->profesionComboBox_2->setCurrentText(profesion);
    ui->sueldoLineEdit_2->setText(sueldo);
    ui->usernameMaestroLineEdit_2->setText(username);
    ui->passwordMaestroLineEdit_2->setText(password);
    ui->rolMaestroDefaultLabel_2->setText("Maestro"); // Fijar el rol

    // Hacer que el rol sea no editable
    ui->rolMaestroDefaultLabel_2->setEnabled(false);
    ui->stackedWidget->setCurrentWidget(ui->page_4); // Página para modificar maestro

}



void MainWindow::on_modificarMaestroBtn_clicked()
{
    int row = ui->tableWidgetTeacher->currentRow();

    // Verificar que hay una fila seleccionada
    if (row == -1) {
        QMessageBox::warning(this, "Error", "No hay ningún maestro seleccionado para modificar.");
        return;
    }

    QString oldId = ui->tableWidgetTeacher->item(row, 1)->text(); // ID original del maestro
    QString oldUsername = ui->tableWidgetTeacher->item(row, 0)->text(); // Username original

    // Obtener nuevos datos modificados
    QString id = ui->IDMaestroLineEdit_2->text().trimmed();
    QString nombre = ui->maestroNamelineEdit_2->text().trimmed();
    QString profesion = ui->profesionComboBox_2->currentText();
    QString sueldo = ui->sueldoLineEdit_2->text().trimmed();
    QString username = ui->usernameMaestroLineEdit_2->text().trimmed(); // Nuevo username
    QString password = ui->passwordMaestroLineEdit_2->text().trimmed();
    QString rol = "Maestro"; // No se modifica

    // Validaciones
    if (id.isEmpty() || nombre.isEmpty() || profesion.isEmpty() || sueldo.isEmpty() ||
        username.isEmpty() || password.isEmpty()) {
        QMessageBox::warning(this, "Error", "Todos los campos son obligatorios.");
        return;
    }

    // Evitar duplicar usernames (si el username cambió, verificar que no exista)
    if (oldUsername != username && registeredTeachers.contains(username)) {
        QMessageBox::warning(this, "Error", "El nombre de usuario ya está registrado.");
        return;
    }


    if (oldUsername != username) {
        actualizarReferenciasEnAsignaciones(oldUsername, username, false);
    }


    // 🔹 Primero actualizar en los mapas de datos
    if (oldUsername != username) {
        // Eliminar el usuario anterior si el username ha cambiado
        registeredTeacherIDs.remove(oldUsername);
        registeredTeachers.remove(oldUsername);
        nombresMaestros.remove(oldUsername);
        profesionesMaestros.remove(oldUsername);
        sueldosMaestros.remove(oldUsername);
        rolesMaestros.remove(oldUsername);
    }

    // Agregar la información actualizada en los mapas de datos
    registeredTeacherIDs[username] = id;
    registeredTeachers[username] = password;
    nombresMaestros[username] = nombre;
    profesionesMaestros[username] = profesion;
    sueldosMaestros[username] = sueldo;
    rolesMaestros[username] = rol;

    // 🔹 Actualizar en la tabla
    ui->tableWidgetTeacher->setItem(row, 0, new QTableWidgetItem(username));
    ui->tableWidgetTeacher->setItem(row, 1, new QTableWidgetItem(id));
    ui->tableWidgetTeacher->setItem(row, 2, new QTableWidgetItem(nombre));
    ui->tableWidgetTeacher->setItem(row, 3, new QTableWidgetItem(profesion));
    ui->tableWidgetTeacher->setItem(row, 4, new QTableWidgetItem(sueldo));
    ui->tableWidgetTeacher->setItem(row, 5, new QTableWidgetItem(password));
    ui->tableWidgetTeacher->setItem(row, 6, new QTableWidgetItem(rol));

    // 🔹 Guardar cambios en el archivo maestros.txt
    guardarMaestrosEnArchivo();
    actualizarAsignaciones();

    QMessageBox::information(this, "Éxito", "Datos del maestro modificados correctamente.");

    // Regresar a la lista de maestros
    ui->stackedWidget->setCurrentWidget(ui->page_5);
}



void MainWindow::on_deleteTeacherBtn_clicked()
{
    int row = ui->tableWidgetTeacher->currentRow();

    // 1️⃣ Verificar si hay una fila seleccionada
    if (row == -1) {
        QMessageBox::warning(this, "Error", "Seleccione un maestro de la lista para eliminar.");
        return;
    }

    // 2️⃣ Obtener el username del maestro seleccionado
    QString idMaestro = ui->tableWidgetTeacher->item(row, 1)->text();
    QString username = ui->tableWidgetTeacher->item(row, 0)->text();

    // 3️⃣ Confirmar eliminación con el usuario
    QMessageBox::StandardButton confirm;
    confirm = QMessageBox::question(this, "Confirmar eliminación",
                                    "¿Está seguro de que desea eliminar al maestro '" + username + "'?",
                                    QMessageBox::Yes | QMessageBox::No);
    if (confirm == QMessageBox::No) {
        return;
    }

    // 4️⃣ Eliminar de los mapas de datos
    registeredTeachers.remove(username);
    registeredTeacherIDs.remove(username);
    nombresMaestros.remove(username);
    profesionesMaestros.remove(username);
    sueldosMaestros.remove(username);
    rolesMaestros.remove(username);

    // 5️⃣ Eliminar la fila de la tabla
    ui->tableWidgetTeacher->removeRow(row);

    // 6️⃣ Guardar cambios en el archivo (actualizando sin el usuario eliminado)
    guardarMaestrosEnArchivo();
    actualizarAsignaciones();

    QMessageBox::information(this, "Eliminado", "El maestro '" + username + "' ha sido eliminado correctamente.");

}


// ALUMNOS


void MainWindow::on_alumnosBtn_clicked()
{
    ui->stackedWidget->setCurrentWidget(ui->page_6); // 🔹 Mostrar page_6
    cargarAlumnosEnTabla();
}


void MainWindow::on_addAlumnoBtn_clicked()
{
    ui->stackedWidget->setCurrentWidget(ui->page_7); // 🔹 Ir a page_7
    ui->rolAlumnoLineEdit->setText("Alumno"); // 🔹 Fijar rol "Alumno"
    ui->rolAlumnoLineEdit->setEnabled(false);
}


void MainWindow::on_crearAlumnoBtn_clicked()
{
    QString cuenta = ui->cuentaLineEdit->text().trimmed();
    QString nombre = ui->nombreLineEdit->text().trimmed();
    QString carrera = ui->carreraComboBox->currentText();
    QString clasesCursadas = ui->clasesCursadasLineEdit->text().trimmed();
    QString usuario = ui->usuarioAlumnoLineEdit->text().trimmed();
    QString password = ui->passwordAlumnoLineEdit->text().trimmed();
    QString rol = "Alumno"; // 🔹 Fijar rol por defecto

    // 🔴 Validaciones
    if (cuenta.isEmpty() || nombre.isEmpty() || carrera.isEmpty() ||
        clasesCursadas.isEmpty() || usuario.isEmpty() || password.isEmpty()) {
        QMessageBox::warning(this, "Error", "Todos los campos son obligatorios.");
        return;
    }

    // Validar que cuenta y clases cursadas sean números
    bool cuentaEsNumero, clasesEsNumero;
    cuenta.toInt(&cuentaEsNumero);
    clasesCursadas.toInt(&clasesEsNumero);
    if (!cuentaEsNumero || !clasesEsNumero) {
        QMessageBox::warning(this, "Error", "Cuenta y clases cursadas deben ser números.");
        return;
    }

    // Validar que el usuario sea único
    if (registeredAlumnos.contains(usuario)) {
        QMessageBox::warning(this, "Error", "El nombre de usuario ya está registrado.");
        return;
    }

    // 🔹 Guardar datos en los mapas
    registeredAlumnos.insert(usuario, password);
    cuentasAlumnos.insert(usuario, cuenta);
    nombresAlumnos.insert(usuario, nombre);
    carrerasAlumnos.insert(usuario, carrera);
    clasesCursadasAlumnos.insert(usuario, clasesCursadas);
    rolesAlumnos.insert(usuario, rol);

    // 🔹 Guardar en el archivo
    guardarAlumnosEnArchivo();
    cargarAlumnosEnTabla();

    QMessageBox::information(this, "Éxito", "Nuevo alumno registrado correctamente.");


    limpiarCamposNuevoAlumno();
    ui->stackedWidget->setCurrentWidget(ui->page_6);
}

void MainWindow::limpiarCamposNuevoAlumno() {
    ui->cuentaLineEdit->clear();
    ui->nombreLineEdit->clear();
    ui->clasesCursadasLineEdit->clear();
    ui->usuarioAlumnoLineEdit->clear();
    ui->passwordAlumnoLineEdit->clear();
}

void MainWindow::cargarAlumnosEnTabla() {
    ui->tableWidgetAlumnos->setRowCount(0); // Vaciar tabla

    int row = 0;
    for (const QString &usuario : registeredAlumnos.keys()) {
        ui->tableWidgetAlumnos->insertRow(row);

        // 🔹 Asegurar que los datos se muestran en el orden correcto
        ui->tableWidgetAlumnos->setItem(row, 0, new QTableWidgetItem(cuentasAlumnos.value(usuario, ""))); // Cuenta
        ui->tableWidgetAlumnos->setItem(row, 1, new QTableWidgetItem(nombresAlumnos.value(usuario, ""))); // Nombre
        ui->tableWidgetAlumnos->setItem(row, 2, new QTableWidgetItem(carrerasAlumnos.value(usuario, ""))); // Carrera
        ui->tableWidgetAlumnos->setItem(row, 3, new QTableWidgetItem(clasesCursadasAlumnos.value(usuario, ""))); // Clases Cursadas
        ui->tableWidgetAlumnos->setItem(row, 4, new QTableWidgetItem(usuario)); // Usuario
        ui->tableWidgetAlumnos->setItem(row, 5, new QTableWidgetItem(registeredAlumnos.value(usuario, ""))); // Contraseña
        ui->tableWidgetAlumnos->setItem(row, 6, new QTableWidgetItem(rolesAlumnos.value(usuario, ""))); // Rol

        row++;
    }
}

void MainWindow::guardarAlumnosEnArchivo() {
    QFile file("alumnos.txt");
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qDebug() << "❌ No se pudo abrir alumnos.txt para escritura.";
        return;
    }

    QTextStream out(&file);

    for (auto it = registeredAlumnos.begin(); it != registeredAlumnos.end(); ++it) {
        QString usuario = it.key();
        QString password = it.value();
        QString cuenta = cuentasAlumnos.value(usuario, "");
        QString nombre = nombresAlumnos.value(usuario, "");
        QString carrera = carrerasAlumnos.value(usuario, "");
        QString clasesCursadas = clasesCursadasAlumnos.value(usuario, "");
        QString rol = "Alumno"; // 🔹 Siempre fijo

        out << cuenta << "," << nombre << "," << carrera << "," << clasesCursadas << ","
            << usuario << "," << password << "," << rol << "\n";
    }
    file.close();
}

void MainWindow::cargarAlumnosDesdeArchivo() {
    QFile file("alumnos.txt");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "❌ No se pudo abrir alumnos.txt.";
        return;
    }

    QTextStream in(&file);
    registeredAlumnos.clear();
    cuentasAlumnos.clear();
    nombresAlumnos.clear();
    carrerasAlumnos.clear();
    clasesCursadasAlumnos.clear();
    rolesAlumnos.clear();

    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();

        if (line.isEmpty()) continue;

        QStringList parts = line.split(",");

        if (parts.size() == 7) {
            QString cuenta = parts[0];
            QString nombre = parts[1];
            QString carrera = parts[2];
            QString clasesCursadas = parts[3];
            QString usuario = parts[4];
            QString password = parts[5];
            QString rol = parts[6];

            registeredAlumnos.insert(usuario, password);
            cuentasAlumnos.insert(usuario, cuenta);
            nombresAlumnos.insert(usuario, nombre);
            carrerasAlumnos.insert(usuario, carrera);
            clasesCursadasAlumnos.insert(usuario, clasesCursadas);
            rolesAlumnos.insert(usuario, rol);

            qDebug() << "✅ Alumno cargado: " << usuario;
        } else {
            qDebug() << "❌ Línea mal formada en alumnos.txt: " << line;
        }
    }
    file.close();
}



void MainWindow::on_modifyAlumnoBtn_clicked()
{
    int row = ui->tableWidgetAlumnos->currentRow();

    // ✅ Si no se ha seleccionado ninguna fila, mostrar error
    if (row == -1) {
        QMessageBox::warning(this, "Error", "Por favor, seleccione un alumno de la lista antes de modificar.");
        return;
    }

    // ✅ Obtener los datos del alumno seleccionado
    QString cuenta = ui->tableWidgetAlumnos->item(row, 0)->text();
    QString nombre = ui->tableWidgetAlumnos->item(row, 1)->text();
    QString carrera = ui->tableWidgetAlumnos->item(row, 2)->text();
    QString clasesCursadas = ui->tableWidgetAlumnos->item(row, 3)->text();
    QString usuario = ui->tableWidgetAlumnos->item(row, 4)->text();
    QString password = ui->tableWidgetAlumnos->item(row, 5)->text();
    QString rol = ui->tableWidgetAlumnos->item(row, 6)->text();

    // ✅ Llenar los campos en page_8
    ui->cuentaLineEdit_2->setText(cuenta);
    ui->nombreLineEdit_2->setText(nombre);
    ui->carreraComboBox_2->setCurrentText(carrera);
    ui->clasesCursadasLineEdit_2->setText(clasesCursadas);
    ui->usuarioAlumnoLineEdit_2->setText(usuario);
    ui->passwordAlumnoLineEdit_2->setText(password);
    ui->rolAlumnoLineEdit_2->setText(rol);

    // ✅ Hacer que el rol no sea editable
    ui->rolAlumnoLineEdit_2->setEnabled(false);

    // ✅ Guardar el usuario original para futuras modificaciones
    alumnoSeleccionado = usuario;

    // ✅ Cambiar a la vista de modificación (page_8)
    ui->stackedWidget->setCurrentWidget(ui->page_8);

}


void MainWindow::on_modifyAlumnoBtn_2_clicked()
{
    // Verificar que hay un alumno seleccionado
    if (alumnoSeleccionado.isEmpty()) {
        QMessageBox::warning(this, "Error", "No hay ningún alumno seleccionado para modificar.");
        return;
    }


    // Obtener los nuevos datos del formulario en page_8
    QString cuenta = ui->cuentaLineEdit_2->text().trimmed();
    QString nombre = ui->nombreLineEdit_2->text().trimmed();
    QString carrera = ui->carreraComboBox_2->currentText();
    QString clasesCursadas = ui->clasesCursadasLineEdit_2->text().trimmed();
    QString usuario = ui->usuarioAlumnoLineEdit_2->text().trimmed();
    QString password = ui->passwordAlumnoLineEdit_2->text().trimmed();
    QString rol = "Alumno"; // Siempre "Alumno"

    // Validaciones
    if (cuenta.isEmpty() || nombre.isEmpty() || carrera.isEmpty() || clasesCursadas.isEmpty() ||
        usuario.isEmpty() || password.isEmpty()) {
        QMessageBox::warning(this, "Error", "Todos los campos son obligatorios.");
        return;
    }

    // Validar que cuenta y clases cursadas sean números
    bool cuentaEsNumero, clasesEsNumero;
    cuenta.toInt(&cuentaEsNumero);
    clasesCursadas.toInt(&clasesEsNumero);
    if (!cuentaEsNumero || !clasesEsNumero) {
        QMessageBox::warning(this, "Error", "Cuenta y clases cursadas deben ser números.");
        return;
    }

    // Si el usuario ha cambiado, verificar que no esté repetido
    if (alumnoSeleccionado != usuario && registeredAlumnos.contains(usuario)) {
        QMessageBox::warning(this, "Error", "El nombre de usuario ya está registrado.");
        return;
    }
    QString idClase = ui->IDClaseLineEdit->text().trimmed();  // Definir idClase antes


    // 🔹 Si el username del alumno cambió, actualizarlo en `alumno_asignaciones.txt`
    if (alumnoSeleccionado != usuario || cuentasAlumnos[alumnoSeleccionado] != cuenta || nombresAlumnos[alumnoSeleccionado] != nombre || carrerasAlumnos[alumnoSeleccionado] != carrera || clasesCursadasAlumnos[alumnoSeleccionado] != clasesCursadas || registeredAlumnos[alumnoSeleccionado] != password) {
        actualizarReferenciasEnAlumnoAsignaciones(alumnoSeleccionado, usuario, cuenta, nombre, carrera, clasesCursadas, password, claseSeleccionada, idClase);
    }

    // Eliminar los datos del usuario anterior si el nombre de usuario ha cambiado
    if (alumnoSeleccionado != usuario) {
        registeredAlumnos.remove(alumnoSeleccionado);
        cuentasAlumnos.remove(alumnoSeleccionado);
        nombresAlumnos.remove(alumnoSeleccionado);
        carrerasAlumnos.remove(alumnoSeleccionado);
        clasesCursadasAlumnos.remove(alumnoSeleccionado);
        rolesAlumnos.remove(alumnoSeleccionado);
    }

    // Guardar los nuevos datos en los `QMap`
    registeredAlumnos[usuario] = password;
    cuentasAlumnos[usuario] = cuenta;
    nombresAlumnos[usuario] = nombre;
    carrerasAlumnos[usuario] = carrera;
    clasesCursadasAlumnos[usuario] = clasesCursadas;
    rolesAlumnos[usuario] = rol;

    // Guardar los cambios en `alumnos.txt`
    guardarAlumnosEnArchivo();

    // Actualizar la tabla en `page_6`
    cargarAlumnosEnTabla();

    // Regresar a la lista de alumnos
    ui->stackedWidget->setCurrentWidget(ui->page_6);

    QMessageBox::information(this, "Éxito", "Los datos del alumno se han modificado correctamente.");

}


void MainWindow::on_deleteAlumnoBtn_clicked()
{
    int row = ui->tableWidgetAlumnos->currentRow();

    // 1️⃣ Verificar si hay una fila seleccionada
    if (row == -1) {
        QMessageBox::warning(this, "Error", "Seleccione un alumno de la lista para eliminar.");
        return;
    }

    // 2️⃣ Obtener el usuario del alumno seleccionado
    QString usuario = ui->tableWidgetAlumnos->item(row, 4)->text(); // Columna de usuario

    // 3️⃣ Confirmar eliminación con el usuario
    QMessageBox::StandardButton confirm;
    confirm = QMessageBox::question(this, "Confirmar eliminación",
                                    "¿Está seguro de que desea eliminar al alumno '" + usuario + "'?",
                                    QMessageBox::Yes | QMessageBox::No);
    if (confirm == QMessageBox::No) {
        return;
    }

    // 4️⃣ Eliminar de los mapas de datos
    registeredAlumnos.remove(usuario);
    cuentasAlumnos.remove(usuario);
    nombresAlumnos.remove(usuario);
    carrerasAlumnos.remove(usuario);
    clasesCursadasAlumnos.remove(usuario);
    rolesAlumnos.remove(usuario);

    // Eliminar la fila de la tabla
    ui->tableWidgetAlumnos->removeRow(row);

    // Guardar cambios en el archivo (actualizando sin el usuario eliminado)
    guardarAlumnosEnArchivo();

    QMessageBox::information(this, "Eliminado", "El alumno '" + usuario + "' ha sido eliminado correctamente.");
}

// CLASES

void MainWindow::on_clasesBtn_clicked()
{
    ui->stackedWidget->setCurrentWidget(ui->page_9); // Mostrar lista de clases
    cargarClasesEnTabla();

}

void MainWindow::on_addClaseBtn_clicked()
{
    ui->stackedWidget->setCurrentWidget(ui->page_10); // Ir a la página de agregar clase
    ui->yearLineEdit->setText("2025"); // Fijar el año en 2025
    ui->yearLineEdit->setEnabled(false);

}


void MainWindow::on_addClaseBtn_2_clicked()
{
    // Obtener datos del formulario
    QString idClase = ui->IDClaseLineEdit->text().trimmed();
    QString nombreClase = ui->nameClassComboBox->currentText();
    QString horaClase = ui->horaClaseComboBox->currentText();
    QString unidadesValorativas = ui->unidadesValComboBox->currentText();
    QString semestre = ui->semestreComboBox->currentText();
    QString periodo = ui->periodoComboBox->currentText();
    QString year = ui->yearLineEdit->text().trimmed(); // Siempre será "2025"

    // Validaciones
    if (idClase.isEmpty() || nombreClase.isEmpty() || horaClase.isEmpty() ||
        unidadesValorativas.isEmpty() || semestre.isEmpty() || periodo.isEmpty() || year.isEmpty()) {
        QMessageBox::warning(this, "Error", "Todos los campos son obligatorios.");
        return;
    }

    // Validar que el ID de la clase sea único
    if (registeredClases.contains(idClase)) {
        QMessageBox::warning(this, "Error", "El ID de la clase ya está registrado.");
        return;
    }

    // Guardar datos en los `QMap`
    registeredClases.insert(idClase, nombreClase);
    horaClases.insert(idClase, horaClase);
    unidadesValorativasClases.insert(idClase, unidadesValorativas);
    semestreClases.insert(idClase, semestre);
    periodoClases.insert(idClase, periodo);
    yearClases.insert(idClase, year);

    // Guardar en el archivo
    guardarClasesEnArchivo();
    cargarClasesEnTabla();

    QMessageBox::information(this, "Éxito", "Nueva clase registrada correctamente.");

    limpiarCamposNuevaClase();
    ui->stackedWidget->setCurrentWidget(ui->page_9);

}

void MainWindow::limpiarCamposNuevaClase()
{
    ui->IDClaseLineEdit->clear();
    ui->nameClassComboBox->setCurrentIndex(0);
    ui->horaClaseComboBox->setCurrentIndex(0);
    ui->unidadesValComboBox->setCurrentIndex(0);
    ui->semestreComboBox->setCurrentIndex(0);
    ui->periodoComboBox->setCurrentIndex(0);
}

void MainWindow::cargarClasesEnTabla()
{
    ui->tableWidgetClases->setRowCount(0); // Vaciar tabla

    int row = 0;
    for (const QString &idClase : registeredClases.keys()) {
        ui->tableWidgetClases->insertRow(row);

        ui->tableWidgetClases->setItem(row, 0, new QTableWidgetItem(idClase)); // ID Clase
        ui->tableWidgetClases->setItem(row, 1, new QTableWidgetItem(registeredClases.value(idClase, ""))); // Nombre Clase
        ui->tableWidgetClases->setItem(row, 2, new QTableWidgetItem(horaClases.value(idClase, ""))); // Hora
        ui->tableWidgetClases->setItem(row, 3, new QTableWidgetItem(unidadesValorativasClases.value(idClase, ""))); // UV
        ui->tableWidgetClases->setItem(row, 4, new QTableWidgetItem(semestreClases.value(idClase, ""))); // Semestre
        ui->tableWidgetClases->setItem(row, 5, new QTableWidgetItem(periodoClases.value(idClase, ""))); // Periodo
        ui->tableWidgetClases->setItem(row, 6, new QTableWidgetItem(yearClases.value(idClase, ""))); // Año

        row++;
    }
}

void MainWindow::guardarClasesEnArchivo()
{
    QFile file("clases.txt");
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qDebug() << "No se pudo abrir clases.txt para escritura.";
        return;
    }

    QTextStream out(&file);

    for (auto it = registeredClases.begin(); it != registeredClases.end(); ++it) {
        QString idClase = it.key();
        QString nombreClase = it.value();
        QString horaClase = horaClases.value(idClase, "");
        QString unidadesValorativas = unidadesValorativasClases.value(idClase, "");
        QString semestre = semestreClases.value(idClase, "");
        QString periodo = periodoClases.value(idClase, "");
        QString year = yearClases.value(idClase, "");

        out << idClase << "," << nombreClase << "," << horaClase << "," << unidadesValorativas << ","
            << semestre << "," << periodo << "," << year << "\n";
    }
    file.close();
}

void MainWindow::cargarClasesDesdeArchivo()
{
    QFile file("clases.txt");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "No se pudo abrir clases.txt.";
        return;
    }

    QTextStream in(&file);
    registeredClases.clear();
    horaClases.clear();
    unidadesValorativasClases.clear();
    semestreClases.clear();
    periodoClases.clear();
    yearClases.clear();

    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();

        if (line.isEmpty()) continue;

        QStringList parts = line.split(",");

        if (parts.size() == 7) {
            QString idClase = parts[0];
            QString nombreClase = parts[1];
            QString horaClase = parts[2];
            QString unidadesValorativas = parts[3];
            QString semestre = parts[4];
            QString periodo = parts[5];
            QString year = parts[6];

            registeredClases.insert(idClase, nombreClase);
            horaClases.insert(idClase, horaClase);
            unidadesValorativasClases.insert(idClase, unidadesValorativas);
            semestreClases.insert(idClase, semestre);
            periodoClases.insert(idClase, periodo);
            yearClases.insert(idClase, year);

            qDebug() << "Clase cargada: " << idClase;
        } else {
            qDebug() << "Línea mal formada en clases.txt: " << line;
        }
    }
    file.close();
}

void MainWindow::on_modifyClassBtn_clicked()
{
    int row = ui->tableWidgetClases->currentRow();

    // Si no se ha seleccionado ninguna fila, mostrar error
    if (row == -1) {
        QMessageBox::warning(this, "Error", "Por favor, seleccione una clase de la lista antes de modificar.");
        return;
    }

    // Obtener los datos de la clase seleccionada
    QString idClase = ui->tableWidgetClases->item(row, 0)->text();
    QString nombreClase = ui->tableWidgetClases->item(row, 1)->text();
    QString horaClase = ui->tableWidgetClases->item(row, 2)->text();
    QString unidadesValorativas = ui->tableWidgetClases->item(row, 3)->text();
    QString semestre = ui->tableWidgetClases->item(row, 4)->text();
    QString periodo = ui->tableWidgetClases->item(row, 5)->text();
    QString year = ui->tableWidgetClases->item(row, 6)->text();

    // Llenar los campos en `page_11`
    ui->IDClaseLineEdit_2->setText(idClase);
    ui->nameClassComboBox_2->setCurrentText(nombreClase);
    ui->horaClaseComboBox_2->setCurrentText(horaClase);
    ui->unidadesValComboBox_2->setCurrentText(unidadesValorativas);
    ui->semestreComboBox_2->setCurrentText(semestre);
    ui->periodoComboBox_2->setCurrentText(periodo);
    ui->yearLineEdit_2->setText(year);

    // Hacer que el año no sea editable
    ui->yearLineEdit_2->setEnabled(false);

    // Guardar la clase seleccionada para modificarla posteriormente
    claseSeleccionada = idClase;

    // Cambiar a la vista de modificación (page_11)
    ui->stackedWidget->setCurrentWidget(ui->page_11);

}



void MainWindow::on_modifyClassBtn_2_clicked()
{
    // Verificar que hay una clase seleccionada
    if (claseSeleccionada.isEmpty()) {
        QMessageBox::warning(this, "Error", "No hay ninguna clase seleccionada para modificar.");
        return;
    }

    // Obtener los nuevos datos del formulario en `page_11`
    QString oldIdClase = claseSeleccionada; // Guardar el ID anterior
    QString idClase = ui->IDClaseLineEdit_2->text().trimmed();
    QString nombreClase = ui->nameClassComboBox_2->currentText();
    QString horaClase = ui->horaClaseComboBox_2->currentText();
    QString unidadesValorativas = ui->unidadesValComboBox_2->currentText();
    QString semestre = ui->semestreComboBox_2->currentText();
    QString periodo = ui->periodoComboBox_2->currentText();
    QString year = ui->yearLineEdit_2->text().trimmed(); // Año no modificable

    // Validaciones
    if (idClase.isEmpty() || nombreClase.isEmpty() || horaClase.isEmpty() ||
        unidadesValorativas.isEmpty() || semestre.isEmpty() || periodo.isEmpty() || year.isEmpty()) {
        QMessageBox::warning(this, "Error", "Todos los campos son obligatorios.");
        return;
    }

    // Si el ID de la clase ha cambiado, verificar que no esté repetido
    if (claseSeleccionada != idClase && registeredClases.contains(idClase)) {
        QMessageBox::warning(this, "Error", "El ID de la clase ya está registrado.");
        return;
    }

    // Si el ID de la clase cambió, actualizar las referencias en `asignaciones.txt`
    if (claseSeleccionada != idClase) {
        actualizarReferenciasEnAsignaciones(claseSeleccionada, idClase, true);
        actualizarReferenciasEnAlumnoAsignaciones(claseSeleccionada, idClase, nombreClase, horaClase, unidadesValorativas, semestre, periodo, claseSeleccionada, idClase); // 🔹 Ahora con los 7 parámetros

    }

    // Eliminar los datos de la clase anterior si el ID ha cambiado
    if (claseSeleccionada != idClase) {
        registeredClases.remove(claseSeleccionada);
        horaClases.remove(claseSeleccionada);
        unidadesValorativasClases.remove(claseSeleccionada);
        semestreClases.remove(claseSeleccionada);
        periodoClases.remove(claseSeleccionada);
        yearClases.remove(claseSeleccionada);
    }

    // Guardar los nuevos datos en los `QMap`
    registeredClases[idClase] = nombreClase;
    horaClases[idClase] = horaClase;
    unidadesValorativasClases[idClase] = unidadesValorativas;
    semestreClases[idClase] = semestre;
    periodoClases[idClase] = periodo;
    yearClases[idClase] = year;

    // Guardar los cambios en `clases.txt`
    guardarClasesEnArchivo();
    actualizarAsignaciones();


    // Actualizar la tabla en `page_9`
    cargarClasesEnTabla();

    // Regresar a la lista de clases
    ui->stackedWidget->setCurrentWidget(ui->page_9);

    QMessageBox::information(this, "Éxito", "Los datos de la clase se han modificado correctamente.");

}

void MainWindow::on_deleteClassBtn_clicked()
{
    int row = ui->tableWidgetClases->currentRow();

    // Verificar si hay una fila seleccionada
    if (row == -1) {
        QMessageBox::warning(this, "Error", "Seleccione una clase de la lista para eliminar.");
        return;
    }

    // Obtener el ID de la clase seleccionada
    QString idClase = ui->tableWidgetClases->item(row, 0)->text(); // Columna del ID de la clase

    // Confirmar eliminación con el usuario
    QMessageBox::StandardButton confirm;
    confirm = QMessageBox::question(this, "Confirmar eliminación",
                                    "¿Está seguro de que desea eliminar la clase '" + idClase + "'?",
                                    QMessageBox::Yes | QMessageBox::No);
    if (confirm == QMessageBox::No) {
        return;
    }

    // Eliminar de los mapas de datos
    registeredClases.remove(idClase);
    horaClases.remove(idClase);
    unidadesValorativasClases.remove(idClase);
    semestreClases.remove(idClase);
    periodoClases.remove(idClase);
    yearClases.remove(idClase);

    // Eliminar la fila de la tabla
    ui->tableWidgetClases->removeRow(row);

    // Guardar cambios en el archivo (actualizando sin la clase eliminada)
    guardarClasesEnArchivo();
    actualizarAsignaciones();

    QMessageBox::information(this, "Eliminado", "La clase '" + idClase + "' ha sido eliminada correctamente.");


}


// ASIGNAR


void MainWindow::on_asignarBtn_clicked()
{
    // Asegurar que la tabla `tableWidgetClases_2` esté vacía antes de copiar los datos
    ui->tableWidgetClases_2->setRowCount(0);
    ui->tableWidgetClases_2->setColumnCount(ui->tableWidgetClases->columnCount());

    // Copiar los datos de `tableWidgetClases` a `tableWidgetClases_2`
    for (int row = 0; row < ui->tableWidgetClases->rowCount(); ++row) {
        ui->tableWidgetClases_2->insertRow(row);
        for (int col = 0; col < ui->tableWidgetClases->columnCount(); ++col) {
            QTableWidgetItem *item = ui->tableWidgetClases->item(row, col);
            if (item) {
                ui->tableWidgetClases_2->setItem(row, col, new QTableWidgetItem(*item));
            }
        }
    }

    // Cambiar la vista a `page_12`
    ui->stackedWidget->setCurrentWidget(ui->page_12);

}


void MainWindow::on_asignarMaestroBtn_clicked()
{

    // Verificar si se ha seleccionado al menos una clase antes de avanzar
    QList<QTableWidgetItem*> selectedItems = ui->tableWidgetClases_2->selectedItems();

    if (selectedItems.isEmpty()) {
        QMessageBox::warning(this, "Error", "Debe seleccionar al menos una clase antes de asignar un maestro.");
        return;
    }

    // Asegurar que la tabla `tableWidgetTeacher_2` esté vacía antes de copiar los datos
    ui->tableWidgetTeacher_2->setRowCount(0);
    ui->tableWidgetTeacher_2->setColumnCount(ui->tableWidgetTeacher->columnCount());

    // Copiar los datos de `tableWidgetTeacher` a `tableWidgetTeacher_2`
    for (int row = 0; row < ui->tableWidgetTeacher->rowCount(); ++row) {
        ui->tableWidgetTeacher_2->insertRow(row);
        for (int col = 0; col < ui->tableWidgetTeacher->columnCount(); ++col) {
            QTableWidgetItem *item = ui->tableWidgetTeacher->item(row, col);
            if (item) {
                ui->tableWidgetTeacher_2->setItem(row, col, new QTableWidgetItem(*item));
            }
        }
    }

    // Cambiar la vista a `page_13`
    ui->stackedWidget->setCurrentWidget(ui->page_13);

}


void MainWindow::on_asignarMaestroBtn_2_clicked()
{
    // Verificar si se ha seleccionado al menos un maestro antes de asignar
    int selectedRow = ui->tableWidgetTeacher_2->currentRow();
    if (selectedRow == -1) {
        QMessageBox::warning(this, "Error", "Debe seleccionar un maestro antes de asignar las clases.");
        return;
    }

    // Obtener los valores del maestro seleccionado
    QString username = ui->tableWidgetTeacher_2->item(selectedRow, 0)->text(); // Username del maestro
    QString idMaestro = ui->tableWidgetTeacher_2->item(selectedRow, 1)->text(); // ID del maestro
    QString nombreMaestro = ui->tableWidgetTeacher_2->item(selectedRow, 2)->text(); // Nombre del maestro
    QString profesion = ui->tableWidgetTeacher_2->item(selectedRow, 3)->text(); // Profesión del maestro
    QString sueldo = ui->tableWidgetTeacher_2->item(selectedRow, 4)->text(); // Sueldo del maestro
    QString password = ui->tableWidgetTeacher_2->item(selectedRow, 5)->text(); // Contraseña del maestro
    QString rol = ui->tableWidgetTeacher_2->item(selectedRow, 6)->text(); // Rol del maestro

    // Verificar si hay clases seleccionadas
    QList<QTableWidgetItem*> selectedClasses = ui->tableWidgetClases_2->selectedItems();
    if (selectedClasses.isEmpty()) {
        QMessageBox::warning(this, "Error", "Debe seleccionar al menos una clase para asignar al maestro.");
        return;
    }

    // 📌 1️⃣ Leer todas las asignaciones previas
    QFile asignacionesFile("asignaciones.txt");
    if (!asignacionesFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "No se pudo abrir asignaciones.txt para lectura. Se creará uno nuevo.";
    }

    QTextStream in(&asignacionesFile);
    QStringList asignaciones;
    QMap<QString, QString> asignacionesMap; // Mapa: ID Clase -> Línea de asignación

    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        if (line.isEmpty()) continue;

        QStringList parts = line.split(",");
        if (parts.size() < 14) continue; // Asegurar que la línea es válida

        QString idClase = parts[0];
        asignaciones.append(line);

        if (!asignacionesMap.contains(idClase)) {
            asignacionesMap.insert(idClase, line);
        }
    }
    asignacionesFile.close();

    // 📌 2️⃣ Actualizar solo las clases seleccionadas con el nuevo maestro
    for (QTableWidgetItem *item : selectedClasses) {
        if (item->column() == 0) {
            QString classID = item->text();
            int row = item->row();

            QString nombreClase = ui->tableWidgetClases_2->item(row, 1)->text();
            QString horaClase = ui->tableWidgetClases_2->item(row, 2)->text();
            QString unidadesValorativas = ui->tableWidgetClases_2->item(row, 3)->text();
            QString semestre = ui->tableWidgetClases_2->item(row, 4)->text();
            QString periodo = ui->tableWidgetClases_2->item(row, 5)->text();
            QString year = ui->tableWidgetClases_2->item(row, 6)->text();

            QString nuevaAsignacion = classID + "," + nombreClase + "," + horaClase + "," + unidadesValorativas + "," +
                                      semestre + "," + periodo + "," + year + "," +
                                      username + "," + idMaestro + "," + nombreMaestro + "," + profesion + "," +
                                      sueldo + "," + password + "," + rol;

            asignacionesMap[classID] = nuevaAsignacion; // Actualizar la asignación
        }
    }

    // 📌 3️⃣ Escribir TODAS las asignaciones actualizadas en `asignaciones.txt`
    if (!asignacionesFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "Error", "No se pudo abrir asignaciones.txt para escritura.");
        return;
    }

    QTextStream out(&asignacionesFile);
    for (const QString &line : asignacionesMap.values()) {
        out << line << "\n";
    }
    asignacionesFile.close();

    // 📌 Mensaje de confirmación
    QMessageBox::information(this, "Éxito", "Las asignaciones han sido guardadas correctamente.");

    // Regresar a `page_12`
    ui->stackedWidget->setCurrentWidget(ui->page_12);
}


void MainWindow::on_matricularAlumnoBtn_clicked()
{
    // Verificar si se ha seleccionado al menos una clase antes de avanzar
    QList<QTableWidgetItem*> selectedItems = ui->tableWidgetClases_2->selectedItems();
    if (selectedItems.isEmpty()) {
        QMessageBox::warning(this, "Error", "Debe seleccionar al menos una clase antes de matricular un alumno.");
        return;
    }

    // Cargar alumnos desde el archivo si `tableWidgetAlumnos` está vacío
    if (ui->tableWidgetAlumnos->rowCount() == 0) {
        cargarAlumnosDesdeArchivo();
        cargarAlumnosEnTabla();
    }

    // Asegurar que `tableWidgetAlumnos_2` esté vacía antes de copiar los datos
    ui->tableWidgetAlumnos_2->setRowCount(0);
    ui->tableWidgetAlumnos_2->setColumnCount(ui->tableWidgetAlumnos->columnCount());

    // Copiar los datos de `tableWidgetAlumnos` a `tableWidgetAlumnos_2`
    for (int row = 0; row < ui->tableWidgetAlumnos->rowCount(); ++row) {
        ui->tableWidgetAlumnos_2->insertRow(row);
        for (int col = 0; col < ui->tableWidgetAlumnos->columnCount(); ++col) {
            QTableWidgetItem *item = ui->tableWidgetAlumnos->item(row, col);
            if (item) {
                ui->tableWidgetAlumnos_2->setItem(row, col, new QTableWidgetItem(*item));
            }
        }
    }

    // 🔹 Cambiar la vista a `page_14`
    ui->stackedWidget->setCurrentWidget(ui->page_14);
}



void MainWindow::on_matricularAlumnoBtn_2_clicked()
{
    int selectedStudentRow = ui->tableWidgetAlumnos_2->currentRow();
    if (selectedStudentRow == -1) {
        QMessageBox::warning(this, "Error", "Debe seleccionar un alumno antes de matricularlo en una clase.");
        return;
    }

    QString cuenta = ui->tableWidgetAlumnos_2->item(selectedStudentRow, 0)->text();
    QString nombre = ui->tableWidgetAlumnos_2->item(selectedStudentRow, 1)->text();
    QString carrera = ui->tableWidgetAlumnos_2->item(selectedStudentRow, 2)->text();
    QString clasesCursadas = ui->tableWidgetAlumnos_2->item(selectedStudentRow, 3)->text();
    QString username = ui->tableWidgetAlumnos_2->item(selectedStudentRow, 4)->text();
    QString password = ui->tableWidgetAlumnos_2->item(selectedStudentRow, 5)->text();
    QString rol = ui->tableWidgetAlumnos_2->item(selectedStudentRow, 6)->text();

    QList<QTableWidgetItem*> selectedClasses = ui->tableWidgetClases_2->selectedItems();
    if (selectedClasses.isEmpty()) {
        QMessageBox::warning(this, "Error", "Debe seleccionar al menos una clase para matricular al alumno.");
        return;
    }

    QFile asignacionesFile("alumno_asignaciones.txt");
    if (!asignacionesFile.open(QIODevice::Append | QIODevice::Text)) {
        QMessageBox::warning(this, "Error", "No se pudo abrir alumno_asignaciones.txt para escritura.");
        return;
    }

    QTextStream out(&asignacionesFile);

    for (QTableWidgetItem *item : selectedClasses) {
        if (item->column() == 0) {
            QString idClase = item->text();
            int row = item->row();

            QString nombreClase = ui->tableWidgetClases_2->item(row, 1)->text();
            QString horaClase = ui->tableWidgetClases_2->item(row, 2)->text();
            QString unidadesValorativas = ui->tableWidgetClases_2->item(row, 3)->text();
            QString semestre = ui->tableWidgetClases_2->item(row, 4)->text();
            QString periodo = ui->tableWidgetClases_2->item(row, 5)->text();
            QString year = ui->tableWidgetClases_2->item(row, 6)->text();

            out << idClase << "," << nombreClase << "," << horaClase << "," << unidadesValorativas << ","
                << semestre << "," << periodo << "," << year << ","
                << cuenta << "," << nombre << "," << carrera << "," << clasesCursadas << ","
                << username << "," << password << "," << rol << "\n";
        }
    }

    asignacionesFile.close();

    QMessageBox::information(this, "Éxito", "El alumno ha sido matriculado correctamente en la(s) clase(s).");

    ui->stackedWidget->setCurrentWidget(ui->page_12);
}


void MainWindow::actualizarAsignaciones()
{
    // 🔹 Leer todas las clases desde `clases.txt`
    QFile clasesFile("clases.txt");
    if (!clasesFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "Error", "No se pudo abrir clases.txt.");
        return;
    }
    QTextStream clasesIn(&clasesFile);
    QMap<QString, QString> clasesData; // Mapa: ID Clase -> Datos de la clase

    while (!clasesIn.atEnd()) {
        QString line = clasesIn.readLine().trimmed();
        if (!line.isEmpty()) {
            QStringList parts = line.split(",");
            if (parts.size() >= 7) {
                QString idClase = parts[0]; // ID de la clase
                clasesData[idClase] = line; // Guardar toda la línea
            }
        }
    }
    clasesFile.close();

    // 🔹 Leer todos los maestros desde `maestros.txt`
    QFile maestrosFile("maestros.txt");
    if (!maestrosFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "Error", "No se pudo abrir maestros.txt.");
        return;
    }
    QTextStream maestrosIn(&maestrosFile);
    QMap<QString, QString> maestrosData; // Mapa: Username -> Datos del maestro

    while (!maestrosIn.atEnd()) {
        QString line = maestrosIn.readLine().trimmed();
        if (!line.isEmpty()) {
            QStringList parts = line.split(",");
            if (parts.size() == 7) {
                QString username = parts[0]; // Username del maestro
                maestrosData[username] = line; // Guardar toda la línea del maestro
            }
        }
    }
    maestrosFile.close();

    // 🔹 Leer `asignaciones.txt` para ver qué clases tienen maestro asignado
    QFile asignacionesFile("asignaciones.txt");
    if (!asignacionesFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "Error", "No se pudo abrir asignaciones.txt.");
        return;
    }
    QTextStream asignacionesIn(&asignacionesFile);
    QMap<QString, QString> clasesAsignadas; // Mapa: ID Clase -> Username del maestro

    while (!asignacionesIn.atEnd()) {
        QString line = asignacionesIn.readLine().trimmed();
        if (!line.isEmpty()) {
            QStringList parts = line.split(",");
            if (parts.size() > 7) {
                QString idClase = parts[0];  // ID de la clase
                QString usernameMaestro = parts[7]; // Username del maestro
                clasesAsignadas[idClase] = usernameMaestro;
            }
        }
    }
    asignacionesFile.close();

    // 🔹 Leer `alumnos.txt` para obtener la información más actualizada de los alumnos
    QFile alumnosFile("alumnos.txt");
    if (!alumnosFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "Error", "No se pudo abrir alumnos.txt.");
        return;
    }
    QTextStream alumnosIn(&alumnosFile);
    QMap<QString, QStringList> alumnosData; // Mapa: Username Alumno -> Datos del alumno

    while (!alumnosIn.atEnd()) {
        QString line = alumnosIn.readLine().trimmed();
        if (!line.isEmpty()) {
            QStringList parts = line.split(",");
            if (parts.size() == 7) {
                QString username = parts[4]; // Username del alumno
                alumnosData[username] = parts; // Guardar toda la información del alumno
            }
        }
    }
    alumnosFile.close();

    // 🔹 Leer `alumno_asignaciones.txt` para obtener las asignaciones de alumnos a clases
    QFile alumnoAsignacionesFile("alumno_asignaciones.txt");
    if (!alumnoAsignacionesFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "Error", "No se pudo abrir alumno_asignaciones.txt.");
        return;
    }
    QTextStream alumnosAsignacionesIn(&alumnoAsignacionesFile);
    QMap<QString, QStringList> alumnosAsignados; // Mapa: Username Alumno -> Lista de Clases

    while (!alumnosAsignacionesIn.atEnd()) {
        QString line = alumnosAsignacionesIn.readLine().trimmed();
        if (!line.isEmpty()) {
            QStringList parts = line.split(",");
            if (parts.size() > 7) {
                QString idClase = parts[0];  // ID de la clase
                QString usernameAlumno = parts[11];  // Username del alumno
                alumnosAsignados[usernameAlumno].append(idClase); // Guardar relación alumno-clase
            }
        }
    }
    alumnoAsignacionesFile.close();

    // 🔹 Generar nuevo archivo `asignaciones.txt`
    if (!asignacionesFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "Error", "No se pudo abrir asignaciones.txt para escritura.");
        return;
    }

    QTextStream out(&asignacionesFile);

    // 🔹 Recorrer todas las clases y escribirlas en `asignaciones.txt`
    for (const QString &idClase : clasesData.keys()) {
        QString claseInfo = clasesData[idClase]; // Datos de la clase

        if (clasesAsignadas.contains(idClase)) {
            QString usernameMaestro = clasesAsignadas[idClase];

            if (maestrosData.contains(usernameMaestro)) {
                out << claseInfo << "," << maestrosData[usernameMaestro] << "\n"; // Agregar maestro actualizado
            } else {
                out << claseInfo << ",SIN MAESTRO ASIGNADO\n"; // Si el maestro fue eliminado
            }
        } else {
            out << claseInfo << ",SIN MAESTRO ASIGNADO\n"; // Si no tenía maestro asignado
        }
    }
    asignacionesFile.close();

    // 🔹 Generar nuevo archivo `alumno_asignaciones.txt`
    if (!alumnoAsignacionesFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "Error", "No se pudo abrir alumno_asignaciones.txt para escritura.");
        return;
    }

    QTextStream outAlumnos(&alumnoAsignacionesFile);

    // 🔹 Recorrer todos los alumnos y actualizar sus asignaciones con la información más actualizada
    for (const QString &usernameAlumno : alumnosAsignados.keys()) {
        QStringList clases = alumnosAsignados[usernameAlumno];

        if (alumnosData.contains(usernameAlumno)) { // Verificar si el alumno existe en `alumnos.txt`
            QStringList alumnoInfo = alumnosData[usernameAlumno];

            for (const QString &idClase : clases) {
                if (clasesData.contains(idClase)) { // Asegurar que la clase sigue existiendo
                    outAlumnos << clasesData[idClase] << ","
                               << alumnoInfo[0] << "," // Cuenta
                               << alumnoInfo[1] << "," // Nombre
                               << alumnoInfo[2] << "," // Carrera
                               << alumnoInfo[3] << "," // Clases Cursadas
                               << alumnoInfo[4] << "," // Username
                               << alumnoInfo[5] << "," // Contraseña
                               << "Alumno\n";
                }
            }
        }
    }
    alumnoAsignacionesFile.close();

    QMessageBox::information(this, "Éxito", "Las asignaciones de alumnos y maestros se han actualizado correctamente.");
}



void MainWindow::actualizarReferenciasEnAsignaciones(QString oldValue, QString newValue, bool isClass)
{
    QFile asignacionesFile("asignaciones.txt");
    if (!asignacionesFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "Error", "No se pudo abrir asignaciones.txt.");
        return;
    }

    QTextStream in(&asignacionesFile);
    QStringList asignaciones;
    bool changesMade = false;

    while (!in.atEnd()) {
        QString line = in.readLine();
        QStringList parts = line.split(",");

        if (parts.size() > 7) {
            if (isClass && parts[0] == oldValue) { // Si es una clase, cambiar el ID de la clase
                parts[0] = newValue;
                changesMade = true;
            } else if (!isClass && parts[7] == oldValue) { // Si es un maestro, cambiar el username
                parts[7] = newValue;
                changesMade = true;
            }
        }

        asignaciones.append(parts.join(","));
    }
    asignacionesFile.close();

    // No sobreescribir el archivo si no hubo cambios
    if (!changesMade) {
        return;
    }

    if (!asignacionesFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "Error", "No se pudo abrir asignaciones.txt para escritura.");
        return;
    }

    QTextStream out(&asignacionesFile);
    for (const QString &line : asignaciones) {
        out << line << "\n";
    }
    asignacionesFile.close();
}

void MainWindow::actualizarReferenciasEnAlumnoAsignaciones(QString oldUsername, QString newUsername, QString cuenta, QString nombre, QString carrera, QString clasesCursadas, QString password, QString oldIdClase, QString newIdClase)
{
    QFile asignacionesFile("alumno_asignaciones.txt");
    if (!asignacionesFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "Error", "No se pudo abrir alumno_asignaciones.txt.");
        return;
    }

    QTextStream in(&asignacionesFile);
    QStringList asignaciones;
    bool changesMade = false;

    while (!in.atEnd()) {
        QString line = in.readLine();
        QStringList parts = line.split(",");

        if (parts.size() > 7) {
            if (parts[0] == oldIdClase) {  // 🔹 Buscar el ID de la clase en la primera columna y actualizarlo
                parts[0] = newIdClase;
                changesMade = true;
            }

            if (parts[11] == oldUsername) {  // Encontrar al usuario anterior
                parts[11] = newUsername; // Actualizar username
                parts[7] = cuenta; // Actualizar cuenta
                parts[8] = nombre; // Actualizar nombre
                parts[9] = carrera; // Actualizar carrera
                parts[10] = clasesCursadas; // Actualizar clases cursadas
                parts[12] = password; // Actualizar contraseña
                changesMade = true;
            }
        }

        asignaciones.append(parts.join(","));
    }
    asignacionesFile.close();

    if (!changesMade) {
        return;
    }

    if (!asignacionesFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "Error", "No se pudo abrir alumno_asignaciones.txt para escritura.");
        return;
    }

    QTextStream out(&asignacionesFile);
    for (const QString &line : asignaciones) {
        out << line << "\n";
    }
    asignacionesFile.close();
}








