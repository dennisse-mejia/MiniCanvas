#ifndef MAESTROS_H
#define MAESTROS_H

#include <QWidget>
#include <QTableWidget>
#include <QVBoxLayout>
#include <QScrollArea>


namespace Ui {
class maestros;
}

class maestros : public QWidget
{
    Q_OBJECT

public:
    explicit maestros(QWidget *parent = nullptr);
    ~maestros();
    void setWelcomeMessage(const QString &username);
    void llenarDatosMaestro(const QString &username);
    QString getSelectedTeacher(); // 🔹 Método para obtener el maestro seleccionado



private slots:
    void on_quitTeacherBtn_clicked();

    void on_examsBtn_clicked();

    void on_myAccTeacher_clicked();

    void on_newExamBtn_clicked();


    void on_configExamBtn_clicked();
    void cargarClasesMaestro();
    void on_saveBtn_clicked();


    void on_modifyExamBtn_clicked();
    void actualizarExamenesDesdeAsignaciones();

    void on_continueModify_clicked();

    void on_configExamBtn_2_clicked();
    void on_saveExamConfig_clicked();


private:
    Ui::maestros *ui;
    QString username;
    QString usernameSeleccionado; // 🔹 Para rastrear el maestro seleccionado
    int obtenerPuntajeTotalDeClase(const QString &classID);
    bool validarDatosExamen(QDateTime selectedDateTime, QString durationText, QString scoreText, QString idClase, QString fechaHoraOriginal);
    int obtenerPuntajeTotalDeClaseExcept(const QString &classID,
                                         const QString &nombreClase,
                                         const QString &fechaHoraExcluida);



};

#endif // MAESTROS_H
