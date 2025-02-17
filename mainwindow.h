#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QWidget>
#include <QSet>  // Para manejar IDs y usuarios únicos
#include <QString>
#include <QFile>
#include <QTextStream>
#include <QMap>
#include "asignaciones.h"
#include "maestros.h" // 🔹 Incluir la clase maestros
#include "alumno_asignaciones.h"




namespace Ui {
class MainWindow;
}

class MainWindow : public QWidget
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void setWelcomeMessage(const QString &username, const QString &userType);
    // Guardar los maestros creados (username -> password)
    static QMap<QString, QString> registeredTeachers;
    static QMap<QString, QString> registeredTeacherIDs; // 🔹 Ahora es un QMap con ID y Usuario
    static QMap<QString, QString> nombresMaestros; // usuario -> nombre
    static QMap<QString, QString> profesionesMaestros; // usuario -> profesión
    static QMap<QString, QString> sueldosMaestros; // usuario -> sueldo
    static QMap<QString, QString> rolesMaestros; // usuario -> rol
    void cargarMaestrosDesdeArchivo();
    void configurarTablaMaestros();  // 🔹 Configurar columnas y tabla
    void cargarMaestrosEnTabla();

    static QMap<QString, QString> registeredAlumnos;      // Usuario → Contraseña
    static QMap<QString, QString> cuentasAlumnos;        // Usuario → Cuenta
    static QMap<QString, QString> nombresAlumnos;        // Usuario → Nombre
    static QMap<QString, QString> carrerasAlumnos;       // Usuario → Carrera
    static QMap<QString, QString> clasesCursadasAlumnos; // Usuario → Clases Cursadas
    static QMap<QString, QString> rolesAlumnos;

    // Clases
    static QMap<QString, QString> registeredClases; // ID Clase -> Nombre Clase
    static QMap<QString, QString> horaClases; // ID Clase -> Hora
    static QMap<QString, QString> unidadesValorativasClases; // ID Clase -> Unidades Valorativas
    static QMap<QString, QString> semestreClases; // ID Clase -> Semestre
    static QMap<QString, QString> periodoClases; // ID Clase -> Periodo
    static QMap<QString, QString> yearClases; // ID Clase -> Año

    void cargarClasesEnTabla();
    void guardarClasesEnArchivo();
    void cargarClasesDesdeArchivo();
    void limpiarCamposNuevaClase();
    QString claseSeleccionada;
    void actualizarReferenciasEnAlumnoAsignaciones(QString oldUsername, QString newUsername, QString cuenta, QString nombre, QString carrera, QString clasesCursadas, QString password, QString oldIdClase, QString newIdClase);




private slots:

    void on_crearRegistroUserBtn_clicked();

    void on_quitBtn_clicked();
    void on_crearMaestroBtn_clicked(); // Guardar datos del nuevo maestro
    void guardarMaestrosEnArchivo();


    void on_miCuentaBtn_clicked();

    void on_goToPage_2_clicked();

    void on_maestrosBtn_clicked();

    void on_addTeacherBtn_clicked();

    void on_modifyTeacherBtn_clicked();

    void on_modificarMaestroBtn_clicked();

    void on_deleteTeacherBtn_clicked();

    void on_addAlumnoBtn_clicked();

    void on_crearAlumnoBtn_clicked();

    void on_alumnosBtn_clicked();

    void on_modifyAlumnoBtn_clicked();

    void on_modifyAlumnoBtn_2_clicked();

    void on_deleteAlumnoBtn_clicked();

    void on_clasesBtn_clicked();

    void on_addClaseBtn_2_clicked();

    void on_modifyClassBtn_2_clicked();

    void on_addClaseBtn_clicked();

    void on_modifyClassBtn_clicked();

    void on_deleteClassBtn_clicked();

    void on_asignarBtn_clicked();

    void on_asignarMaestroBtn_clicked();

    void on_asignarMaestroBtn_2_clicked();

    void on_matricularAlumnoBtn_clicked();

    void on_matricularAlumnoBtn_2_clicked();

private:
    Ui::MainWindow *ui;
    QString welcomeMessage; // Guarda el mensaje de bienvenida
    QString username;
    QString userType;

    // **Conjuntos para IDs y Usuarios únicos**
    QSet<QString> idsRegistrados;
    QSet<QString> usuariosRegistrados;
    void limpiarCamposNuevoMaestro();

    void cargarAlumnosEnTabla();
    void guardarAlumnosEnArchivo();
    void cargarAlumnosDesdeArchivo();
    void limpiarCamposNuevoAlumno();
    QString alumnoSeleccionado;

    maestros* teacherWindow;

    // **Métodos auxiliares**
    bool esSueldoValido(const QString &sueldo); // Validar sueldo
    void actualizarAsignaciones(); // 🔹 Método para actualizar asignaciones
    void actualizarReferenciasEnAsignaciones(QString oldValue, QString newValue, bool isClass);




};



#endif // MAINWINDOW_H
