#include "maestros.h"
#include "ui_maestros.h"
#include "loginwindow.h"
#include <QVBoxLayout>
#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>
#include <QScrollArea>
#include <QRadioButton>
#include <QCheckBox>
#include <QTextEdit>
#include <QMessageBox>



maestros::maestros(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::maestros)
{
    ui->setupUi(this);

    ui->stackedWidget->setCurrentWidget(ui->page);



}

maestros::~maestros()
{
    delete ui;
}




void maestros::setWelcomeMessage(const QString &username)
{
    this->username = username;

    // Establecer el nombre de usuario en el label correcto
    ui->usernameLabel->setText(username);

    // Asegurar que el tipo de usuario se muestre como "Maestro"
    ui->userTypeTeacherMyAcc->setText("Maestro");
}

QString maestros::getSelectedTeacher()
{
    return usernameSeleccionado; // 🔹 Devolver el maestro seleccionado
}

void maestros::on_quitTeacherBtn_clicked()
{
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

void maestros::actualizarExamenesDesdeAsignaciones() {
    QFile asignacionesFile("asignaciones.txt");

    if (!asignacionesFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "❌ ERROR: No se pudo abrir asignaciones.txt";
        return;
    }

    // 🔹 Mapa { ID antiguo -> { ID nuevo, Nombre nuevo } }
    QMap<QString, QPair<QString, QString>> cambiosDeClase;
    QMap<QString, QString> idANombreClase;

    QTextStream asignacionesStream(&asignacionesFile);
    QSet<QString> idsNuevos;

    while (!asignacionesStream.atEnd()) {
        QString line = asignacionesStream.readLine().trimmed();
        if (line.isEmpty()) continue;

        QStringList parts = line.split(",");
        if (parts.size() < 14) continue; // Validación mínima

        QString idClaseNuevo = parts[0].trimmed();
        QString nombreClaseNuevo = parts[1].trimmed();

        // Guardamos la relación ID - Nombre
        idANombreClase[idClaseNuevo] = nombreClaseNuevo;
        idsNuevos.insert(idClaseNuevo);

        qDebug() << "✅ Clase en asignaciones.txt → ID:" << idClaseNuevo << ", Nombre:" << nombreClaseNuevo;
    }
    asignacionesFile.close();

    // 🔹 Leer `examenes.txt` y aplicar cambios en ID y/o Nombre de clase si es necesario
    QFile examenesFile("examenes.txt");

    if (!examenesFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "❌ ERROR: No se pudo abrir examenes.txt";
        return;
    }

    QList<QString> examenesActualizados;
    QTextStream examenesStream(&examenesFile);
    bool esPrimeraLinea = true;
    QString idClaseActual = "";

    while (!examenesStream.atEnd()) {
        QString line = examenesStream.readLine().trimmed();
        if (line.isEmpty()) continue;

        if (esPrimeraLinea) {
            QStringList parts = line.split(",");
            if (parts.size() >= 6) {
                idClaseActual = parts[0].trimmed();
                QString nombreClaseActual = parts[1].trimmed();

                QString nuevoIdClase = idClaseActual;
                QString nuevoNombreClase = nombreClaseActual;

                // 🔹 Si el ID no está en la lista de IDs actuales, intentamos encontrar su reemplazo
                if (!idsNuevos.contains(idClaseActual)) {
                    for (auto it = idANombreClase.begin(); it != idANombreClase.end(); ++it) {
                        QString posibleNuevoId = it.key();
                        QString posibleNuevoNombre = it.value();

                        if (nombreClaseActual == posibleNuevoNombre) {
                            qDebug() << "🔄 CAMBIO DETECTADO: ID en examenes.txt (" << idClaseActual
                                     << ") cambiado a (" << posibleNuevoId << ") con mismo nombre (" << posibleNuevoNombre << ")";
                            nuevoIdClase = posibleNuevoId;
                            break;
                        }
                    }
                }

                // 🔹 Si el ID se mantiene pero el nombre ha cambiado en asignaciones.txt, actualizar solo el nombre
                if (idANombreClase.contains(nuevoIdClase) && idANombreClase[nuevoIdClase] != nombreClaseActual) {
                    qDebug() << "🔄 CAMBIO DETECTADO: Nombre en examenes.txt (" << nombreClaseActual
                             << ") cambiado a (" << idANombreClase[nuevoIdClase] << ") para ID (" << nuevoIdClase << ")";
                    nuevoNombreClase = idANombreClase[nuevoIdClase];
                }

                parts[0] = nuevoIdClase;
                parts[1] = nuevoNombreClase;
                line = parts.join(",");
            }
            esPrimeraLinea = false;
        } else if (line.contains(",")) {
            // 🔹 Es una línea de pregunta, no modificar
        } else {
            esPrimeraLinea = true;
        }

        examenesActualizados.append(line);
    }
    examenesFile.close();

    // 🔹 Escribir los datos actualizados en `examenes.txt`
    if (examenesFile.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        QTextStream out(&examenesFile);
        for (const QString &line : examenesActualizados) {
            out << line << "\n";
        }
        examenesFile.close();
        qDebug() << "✅ EXÁMENES ACTUALIZADOS CORRECTAMENTE.";
    } else {
        qDebug() << "❌ ERROR: No se pudo escribir en examenes.txt";
    }
}




void maestros::cargarClasesMaestro()
{
    // Asegurar que la tabla esté vacía antes de llenarla
    ui->tableWidgetClases_Maestros->setRowCount(0);

    QFile file("asignaciones.txt");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "Error: No se pudo abrir asignaciones.txt";
        return;
    }

    QTextStream in(&file);

    int row = 0;
    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        if (line.isEmpty()) continue;

        QStringList parts = line.split(",");

        // Verificar que la línea tenga suficientes elementos
        if (parts.size() < 14) continue;

        QString idClase = parts[0];
        QString nombreClase = parts[1];
        QString horaClase = parts[2];
        QString unidadesValorativas = parts[3];
        QString semestre = parts[4];
        QString periodo = parts[5];
        QString year = parts[6];
        QString usernameMaestro = parts[7]; // Maestro asignado

        // Mostrar solo las clases del maestro actual
        if (usernameMaestro == username) {
            ui->tableWidgetClases_Maestros->insertRow(row);
            ui->tableWidgetClases_Maestros->setItem(row, 0, new QTableWidgetItem(idClase));
            ui->tableWidgetClases_Maestros->setItem(row, 1, new QTableWidgetItem(nombreClase));
            ui->tableWidgetClases_Maestros->setItem(row, 2, new QTableWidgetItem(horaClase));
            ui->tableWidgetClases_Maestros->setItem(row, 3, new QTableWidgetItem(unidadesValorativas));
            ui->tableWidgetClases_Maestros->setItem(row, 4, new QTableWidgetItem(semestre));
            ui->tableWidgetClases_Maestros->setItem(row, 5, new QTableWidgetItem(periodo));
            ui->tableWidgetClases_Maestros->setItem(row, 6, new QTableWidgetItem(year));

            row++;
        }
    }

    file.close();
    actualizarExamenesDesdeAsignaciones();
}



void maestros::on_examsBtn_clicked()
{
    cargarClasesMaestro(); // Cargar solo las clases asignadas al maestro

    ui->stackedWidget->setCurrentWidget(ui->page_2);


}


void maestros::on_myAccTeacher_clicked()
{
    ui->stackedWidget->setCurrentWidget(ui->page);

}


void maestros::on_newExamBtn_clicked()
{

    // Limpiar los valores de fecha, duración y puntaje
    ui->dateTimeEdit->setDateTime(QDateTime::currentDateTime());
    ui->minutesLineEdit->clear();
    ui->puntajeLineEdit->clear();

    // Obtener la fila seleccionada en tableWidgetClases_Maestros
    int selectedRow = ui->tableWidgetClases_Maestros->currentRow();

    // Verificar si no se ha seleccionado ninguna fila
    if (selectedRow == -1) {
        QMessageBox::warning(this, "Error", "Debe seleccionar una clase antes de continuar.");
        return;
    }
    ui->stackedWidget->setCurrentWidget(ui->page_3);

}

int maestros::obtenerPuntajeTotalDeClase(const QString &classID)
{
    QFile examFile("examenes.txt");
    if (!examFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return 0; // Si no se puede abrir, asumimos que no hay puntajes registrados
    }

    QTextStream in(&examFile);
    int totalPuntaje = 0;

    while (!in.atEnd()) {
        QString line = in.readLine();
        QStringList parts = line.split(",");

        if (parts.size() >= 5 && parts[0] == classID) {
            totalPuntaje += parts[4].toInt(); // Sumar puntaje de los exámenes de la clase
        }
    }

    examFile.close();
    return totalPuntaje;
}

int maestros::obtenerPuntajeTotalDeClaseExcept(const QString &classID,
                                               const QString &nombreClase,
                                               const QString &fechaHoraExcluida)
{
    QFile examFile("examenes.txt");
    if (!examFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return 0;
    }

    QTextStream in(&examFile);
    int totalPuntaje = 0;

    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        if (line.isEmpty()) continue;

        QStringList parts = line.split(",");
        // Mínimo 6 columnas: 0:idClase,1:nombreClase,2:fechaHora,3:dur,4:puntaje,5:maestro
        if (parts.size() >= 6) {
            QString cID         = parts[0].trimmed();
            QString cNombre     = parts[1].trimmed();
            QString cFechaHora  = parts[2].trimmed();
            QString cPuntajeStr = parts[4].trimmed();

            // Sumamos solo si coincide la clase y no es el examen “excluido”
            if (cID == classID && cNombre == nombreClase && cFechaHora != fechaHoraExcluida) {
                totalPuntaje += cPuntajeStr.toInt();
            }
        }
    }
    examFile.close();
    return totalPuntaje;
}



void maestros::on_configExamBtn_clicked()
{



    // 📌 1️⃣ Verificar que la fecha y hora seleccionada sea mayor a la actual
    QDateTime selectedDateTime = ui->dateTimeEdit->dateTime();
    QDateTime currentDateTime = QDateTime::currentDateTime();

    QDateTime minAllowedTime = currentDateTime.addSecs(2 * 60); // Agregar 5 minutos
    if (selectedDateTime < minAllowedTime) {
        QMessageBox::warning(this, "Error", "La hora del examen debe ser al menos 2 minutos después de la hora actual.");
        return;
    }


    // 📌 2️⃣ Verificar que el campo de minutos contenga solo números y no esté vacío
    QString minutesText = ui->minutesLineEdit->text().trimmed();
    bool isMinutesNumber;
    int minutes = minutesText.toInt(&isMinutesNumber);

    if (minutesText.isEmpty() || !isMinutesNumber || minutes <= 0) {
        QMessageBox::warning(this, "Error", "Ingrese un valor válido para la duración del examen (solo números positivos).");
        return;
    }

    // 📌 3️⃣ Verificar que el puntaje del examen sea mayor que 0 y menor o igual a 100
    QString puntajeText = ui->puntajeLineEdit->text().trimmed();
    bool isPuntajeNumber;
    int puntaje = puntajeText.toInt(&isPuntajeNumber);

    if (puntajeText.isEmpty() || !isPuntajeNumber || puntaje <= 0 || puntaje > 100) {
        QMessageBox::warning(this, "Error", "Ingrese un puntaje válido (entre 1 y 100).");
        return;
    }

    // 📌 4️⃣ Verificar que la suma de puntajes de la clase no supere 100
    QString selectedClass = ui->tableWidgetClases_Maestros->item(ui->tableWidgetClases_Maestros->currentRow(), 0)->text();

    int totalPuntaje = obtenerPuntajeTotalDeClase(selectedClass);
    if (totalPuntaje + puntaje > 100) {
        QMessageBox::warning(this, "Error", "El puntaje total de la clase no puede exceder 100.");
        return;
    }

    // 📌 Antes de cambiar de página, limpiar el layout viejo si existe
    if (ui->page_4->layout()) {
        QLayoutItem *item;
        while ((item = ui->page_4->layout()->takeAt(0)) != nullptr) {
            if (QWidget *widget = item->widget()) {
                widget->deleteLater(); // Borra los widgets existentes
            }
            delete item; // Borra el layoutItem
        }
        delete ui->page_4->layout(); // También eliminar el layout anterior
    }



    ui->stackedWidget->setCurrentWidget(ui->page_4);

    // // 🔹 Verificar si ya se ha inicializado la interfaz (evita duplicados)
    // static bool firstLoad = true;
    // if (!firstLoad) return;
    // firstLoad = false;  // Evita inicialización múltiple

    // 🔹 Crear `QScrollArea` para contener las preguntas
    QScrollArea *scrollArea = new QScrollArea(ui->page_4);
    scrollArea->setWidgetResizable(true);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff); // 🔹 Desactiva el scroll horizontal
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);    // 🔹 Activa el scroll vertical si es necesario

    scrollArea->setGeometry(50, 50, 800, 500);

    // 🔹 Crear `QWidget` dentro del `QScrollArea`
    QWidget *scrollWidget = new QWidget();
    scrollWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    scrollWidget->setMinimumWidth(scrollArea->width());
    QVBoxLayout *layout = new QVBoxLayout(scrollWidget);
    scrollWidget->setLayout(layout);
    scrollArea->setWidget(scrollWidget);
    scrollArea->setStyleSheet("QScrollArea { background-color: #FAFAFB; border: none; }");
    scrollWidget->setStyleSheet("QWidget { background-color: #FAFAFB; }");

    // 🔹 Agregar `QLabel` con la imagen del título dentro del `scrollWidget` (solo una vez)
    QLabel *titleLabel = new QLabel();
    QPixmap titlePixmap(":/Resources/Preguntas.png");

    // 🔹 Escalar imagen si es necesario (ajustar ancho sin deformar)
    titleLabel->setPixmap(titlePixmap.scaled(438, 44, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    titleLabel->setAlignment(Qt::AlignCenter);

    layout->addWidget(titleLabel);
    layout->insertWidget(0, titleLabel, 0, Qt::AlignTop); // 🔹 Fijar el título arriba, sin moverse


    scrollWidget->setStyleSheet(R"(

  QWidget {
        font-family: 'Arial Rounded MT Bold';
        font-size: 14px;
        color: #1A2B57;
        background-color: #FAFAFB;
        }

    QLabel, QLineEdit, QComboBox, QRadioButton, QCheckBox, QPushButton {
        font-family: 'Arial Rounded MT Bold';
        font-size: 14px;
        color: #1A2B57;
    }

    QLineEdit {
        border: 2px solid #1A2B57;
        padding: 6px;
        border-radius: 5px;
        background-color: white;
    }

    QComboBox {
        border: 2px solid #1A2B57;
        padding: 6px;
        border-radius: 5px;
        background-color: white;
    }

    /* 🔹 Botones con color `#5EACD3` */
    QPushButton {
        background-color: #5EACD3;
        color: white;
        border-radius: 5px;
        padding: 6px;
        font-weight: bold;
        border: none;
    }

    QPushButton:hover {
        background-color: #4B97BE;
    }

    QPushButton:pressed {
        background-color: #3A85A9;
    }

    /* 🔹 Checkboxes y Radio Buttons visibles */
    QCheckBox::indicator, QRadioButton::indicator {
        width: 18px;
        height: 18px;
        border: 2px solid #1A2B57;
        background-color: white;
        border-radius: 4px;
    }

    QCheckBox::indicator:checked, QRadioButton::indicator:checked {
        background-color: #1A2B57;
        border: 2px solid #1A2B57;
    }

    QCheckBox::indicator:hover, QRadioButton::indicator:hover {
        border: 2px solid #2C3E70;
    }

)");



    // 🔹 Crear contenedor para los botones en la parte inferior
    QWidget *buttonsWidget = new QWidget(ui->page_4);
    QHBoxLayout *buttonsLayout = new QHBoxLayout(buttonsWidget);

    // 🔹 Espaciador antes del primer botón para empujarlo a la derecha
    QSpacerItem *leftSpacer = new QSpacerItem(100, 0, QSizePolicy::Expanding, QSizePolicy::Minimum);

    // 🔹 Botón "+ New Question"
    QPushButton *addQuestionBtn = new QPushButton("", buttonsWidget);
    addQuestionBtn->setFixedSize(165, 45);

    // 🔹 Espaciador entre los botones para ajustar la distancia
    QSpacerItem *middleSpacer = new QSpacerItem(362, 0, QSizePolicy::Fixed, QSizePolicy::Minimum);

    // 🔹 Botón "Guardar"
    QPushButton *saveBtn = new QPushButton("", buttonsWidget);
    saveBtn->setFixedSize(165, 45);

    connect(saveBtn, &QPushButton::clicked, this, &maestros::on_saveBtn_clicked);


    // 🔹 Espaciador después del segundo botón para empujarlo a la izquierda
    QSpacerItem *rightSpacer = new QSpacerItem(100, 0, QSizePolicy::Expanding, QSizePolicy::Minimum);

    // 🔹 Agregar elementos al layout de los botones
    buttonsLayout->addItem(leftSpacer);
    buttonsLayout->addWidget(addQuestionBtn);
    buttonsLayout->addItem(middleSpacer);
    buttonsLayout->addWidget(saveBtn);
    buttonsLayout->addItem(rightSpacer);

    buttonsWidget->setLayout(buttonsLayout);

    // 🔹 Agregar `QScrollArea` y los botones al layout principal de `page_4`
    QVBoxLayout *mainLayout = new QVBoxLayout();
    mainLayout->addWidget(scrollArea);
    mainLayout->addWidget(buttonsWidget, 0, Qt::AlignBottom);
    ui->page_4->setLayout(mainLayout);


    // 🔹 Conectar botón para agregar preguntas dinámicamente
    connect(addQuestionBtn, &QPushButton::clicked, this, [layout, this]() {
        QWidget *questionWidget = new QWidget();
        QHBoxLayout *questionLayout = new QHBoxLayout(questionWidget);

        // 🟢 Campo de pregunta
        QTextEdit *questionInput = new QTextEdit();
        questionInput->setPlaceholderText("Ingrese su pregunta aqui...");
        questionInput->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        questionInput->setMinimumHeight(50);  // Ajusta la altura para mejor visualización
        questionInput->setMaximumHeight(80);  // Evita que crezca demasiado
        questionInput->setWordWrapMode(QTextOption::WordWrap); // Habilita salto de línea automático


        QComboBox *questionType = new QComboBox();
        questionType->addItem("Verdadero/Falso");
        questionType->addItem("Selec. Multiple");
        questionType->addItem("Enumeracion");

        QPushButton *deleteBtn = new QPushButton();
        deleteBtn->setFixedSize(30, 30);  // Tamaño fijo del botón
        deleteBtn->setIcon(QIcon(":/Resources/trash.png"));  // Ruta de la imagen
        deleteBtn->setIconSize(QSize(25, 25));
        deleteBtn->setStyleSheet("QPushButton { border: none; background: transparent; }");

        // 🔹 Crear `QStackedWidget` para cambiar los fields dinámicamente
        QStackedWidget *stackedWidget = new QStackedWidget();

        // 🟢 Página 1: True/False con botón borrar
        QWidget *trueFalsePage = new QWidget();
        QHBoxLayout *trueFalseLayout = new QHBoxLayout(trueFalsePage);
        QRadioButton *trueOption = new QRadioButton("True");
        QRadioButton *falseOption = new QRadioButton("False");


        trueFalseLayout->addWidget(trueOption);
        trueFalseLayout->addWidget(falseOption);
        stackedWidget->addWidget(trueFalsePage);

        // 🟢 Página 2: Multiple Choice
        QWidget *multipleChoicePage = new QWidget();
        QVBoxLayout *multiChoiceLayout = new QVBoxLayout(multipleChoicePage);
        QPushButton *addMultiOptionBtn = new QPushButton("+ Agregar Opcion");

        auto addMultipleChoiceOption = [multiChoiceLayout]() {
            QWidget *optionWidget = new QWidget();
            QHBoxLayout *optionLayout = new QHBoxLayout(optionWidget);

            QCheckBox *optionCheck = new QCheckBox();
            //optionCheck->setChecked(false);
            QLineEdit *optionInput = new QLineEdit();
            optionInput->setMinimumWidth(150);

            QPushButton *deleteOptionBtn = new QPushButton("Borrar");

            // 🔹 Eliminar la opción correctamente
            connect(deleteOptionBtn, &QPushButton::clicked, optionWidget, [optionWidget, multiChoiceLayout]() {
                multiChoiceLayout->removeWidget(optionWidget);
                delete optionWidget;
            });

            optionLayout->addWidget(optionCheck);
            optionLayout->addWidget(optionInput);
            optionLayout->addWidget(deleteOptionBtn);
            optionWidget->setLayout(optionLayout);

            multiChoiceLayout->insertWidget(multiChoiceLayout->count() - 1, optionWidget);
        };

        connect(addMultiOptionBtn, &QPushButton::clicked, [=]() { addMultipleChoiceOption(); });

        for (int i = 0; i < 3; i++) addMultipleChoiceOption();

        multiChoiceLayout->addWidget(addMultiOptionBtn);
        stackedWidget->addWidget(multipleChoicePage);

        // 🟢 Página 3: Enumeration
        QWidget *enumerationPage = new QWidget();
        QVBoxLayout *enumLayout = new QVBoxLayout(enumerationPage);
        QPushButton *addEnumOptionBtn = new QPushButton("+ Agregar Opcion");

        auto addEnumerationOption = [enumLayout]() {
            QWidget *enumOptionWidget = new QWidget();
            QHBoxLayout *enumOptionLayout = new QHBoxLayout(enumOptionWidget);

            QLineEdit *enumInput = new QLineEdit();
            enumInput->setMinimumWidth(150);
            QPushButton *deleteEnumBtn = new QPushButton("Borrar");

            // 🔹 Eliminar correctamente el campo de texto
            connect(deleteEnumBtn, &QPushButton::clicked, enumOptionWidget, [enumOptionWidget, enumLayout]() {
                enumLayout->removeWidget(enumOptionWidget);
                delete enumOptionWidget;
            });

            enumOptionLayout->addWidget(enumInput);
            enumOptionLayout->addWidget(deleteEnumBtn);
            enumOptionWidget->setLayout(enumOptionLayout);

            enumLayout->insertWidget(enumLayout->count() - 1, enumOptionWidget);
        };

        connect(addEnumOptionBtn, &QPushButton::clicked, [=]() { addEnumerationOption(); });

        for (int i = 0; i < 3; i++) addEnumerationOption();

        enumLayout->addWidget(addEnumOptionBtn);
        stackedWidget->addWidget(enumerationPage);

        // 🔹 Conectar `QComboBox` con `QStackedWidget`
        connect(questionType, QOverload<int>::of(&QComboBox::currentIndexChanged), stackedWidget, &QStackedWidget::setCurrentIndex);

        // 🔹 Conectar botón eliminar para borrar toda la pregunta
        connect(deleteBtn, &QPushButton::clicked, questionWidget, &QWidget::deleteLater);

        // 🔹 Agregar widgets al layout de la pregunta
        questionLayout->addWidget(questionInput);
        questionLayout->addWidget(questionType);
        questionLayout->addWidget(stackedWidget);
        questionLayout->addWidget(deleteBtn);
        questionWidget->setLayout(questionLayout);

        // 🔹 Agregar pregunta a `QVBoxLayout`
        layout->addWidget(questionWidget);
    });
}


void maestros::on_saveBtn_clicked()
{
    // 📌 1️⃣ Buscar el contenedor donde se agregan las preguntas
    QScrollArea *scrollArea = ui->page_4->findChild<QScrollArea *>();
    if (!scrollArea) {
        QMessageBox::warning(this, "Error", "No se encontró el área de preguntas.");
        return;
    }

    QWidget *scrollWidget = scrollArea->widget();
    if (!scrollWidget) {
        QMessageBox::warning(this, "Error", "No se encontró el contenedor de preguntas.");
        return;
    }

    QVBoxLayout *layout = qobject_cast<QVBoxLayout *>(scrollWidget->layout());
    if (!layout) {
        QMessageBox::warning(this, "Error", "No se pudo acceder al layout de preguntas.");
        return;
    }

    // 📌 2️⃣ Verificar que haya preguntas
    if (layout->count() == 0) {
        QMessageBox::warning(this, "Error", "Debe agregar al menos una pregunta antes de guardar el examen.");
        return;
    }

    // 📌 3️⃣ Obtener los datos del examen
    QString selectedClass = ui->tableWidgetClases_Maestros->item(ui->tableWidgetClases_Maestros->currentRow(), 0)->text();
    QString className = ui->tableWidgetClases_Maestros->item(ui->tableWidgetClases_Maestros->currentRow(), 1)->text();
    QString examDate = ui->dateTimeEdit->dateTime().toString("yyyy-MM-dd HH:mm:ss");
    QString examDuration = ui->minutesLineEdit->text().trimmed();
    QString examScore = ui->puntajeLineEdit->text().trimmed();
    QString maestroUsername = username;

    // 📌 4️⃣ Permitir múltiples exámenes por clase eliminando la restricción de unicidad
    QFile examFile("examenes.txt");

    if (examFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&examFile);
        bool existeExamen = false;

        while (!in.atEnd()) {
            QString line = in.readLine();

            // 🔹 Verificar si la clase ya tiene exámenes (pero NO bloquear la creación de nuevos)
            if (line.startsWith(selectedClass + "," + className + ",")) {
                existeExamen = true;
                break; // Solo verificamos la existencia, pero no bloqueamos
            }
        }
        examFile.close();
    }


    // 📌 5️⃣ Validar todas las preguntas antes de abrir el archivo para escritura
    QSet<QString> preguntaRegistradas;
    QStringList examData;

    for (int i = 0; i < layout->count(); ++i) {
        QWidget *widget = layout->itemAt(i)->widget();
        if (!widget) continue;

        QTextEdit *questionInput = widget->findChild<QTextEdit *>();
        QComboBox *questionType = widget->findChild<QComboBox *>();
        QStackedWidget *stackedWidget = widget->findChild<QStackedWidget *>();

        if (!questionInput || !questionType || !stackedWidget) continue;

        QString questionText = questionInput->toPlainText().trimmed();
        QString type = questionType->currentText();
        QString answer = "";

        if (questionText.isEmpty()) {
            QMessageBox::warning(this, "Error", "No puede haber preguntas vacías.");
            return;
        }

        preguntaRegistradas.insert(questionText);

        // 📌 Validar respuestas según el tipo de pregunta
        if (type == "Verdadero/Falso") {
            QWidget *trueFalsePage = stackedWidget->widget(0);
            QList<QRadioButton *> radioButtons = trueFalsePage->findChildren<QRadioButton *>();

            bool isChecked = false;
            for (QRadioButton *radio : radioButtons) {
                if (radio->isChecked()) {
                    answer = radio->text();
                    isChecked = true;
                    break;
                }
            }

            if (!isChecked) {
                QMessageBox::warning(this, "Error", "Debe seleccionar una opción en la pregunta de Verdadero/Falso.");
                return;
            }

        } else if (type == "Selec. Multiple") {
            QWidget *multipleChoicePage = stackedWidget->widget(1);
            QList<QCheckBox *> checkboxes = multipleChoicePage->findChildren<QCheckBox *>();
            QList<QLineEdit *> options = multipleChoicePage->findChildren<QLineEdit *>();

            int filledOptions = 0;
            int correctOptions = 0;
            bool hasEmptyField = false;

            for (int i = 0; i < options.size(); ++i) {
                if (!options[i]->text().isEmpty()) {
                    answer += options[i]->text() + ":" + (checkboxes[i]->isChecked() ? "Correcto" : "Incorrecto") + ";";
                    filledOptions++;
                    if (checkboxes[i]->isChecked()) correctOptions++;
                } else {
                    hasEmptyField = true;
                }
            }

            if (hasEmptyField) {
                QMessageBox::warning(this, "Error", "Debe llenar o eliminar los campos vacíos en Selección Múltiple.");
                return;
            }

            if (correctOptions == 0) {
                QMessageBox::warning(this, "Error", "Debe marcar al menos una opción como correcta en Selección Múltiple.");
                return;
            }

        } else if (type == "Enumeracion") {
            QWidget *enumerationPage = stackedWidget->widget(2);
            QList<QLineEdit *> options = enumerationPage->findChildren<QLineEdit *>();

            int filledOptions = 0;
            bool hasEmptyField = false;

            for (QLineEdit *option : options) {
                if (!option->text().isEmpty()) {
                    answer += option->text() + ";";
                    filledOptions++;
                } else {
                    hasEmptyField = true;
                }
            }

            if (hasEmptyField) {
                QMessageBox::warning(this, "Error", "Debe llenar o eliminar los campos vacíos en Enumeración.");
                return;
            }
        }

        // Guardamos la pregunta en la lista temporal
        examData.append(questionText + "," + type + "," + answer);
    }

    // 📌 6️⃣ Ahora abrimos el archivo para escritura (solo si todo está validado)
    if (!examFile.open(QIODevice::Append | QIODevice::Text)) {
        QMessageBox::warning(this, "Error", "No se pudo abrir examenes.txt para escritura.");
        return;
    }

    QTextStream out(&examFile);
    out << selectedClass << "," << className << "," << examDate << "," << examDuration << "," << examScore << "," << maestroUsername << "\n";

    // 📌 7️⃣ Escribimos las preguntas en el archivo
    for (const QString &data : examData) {
        out << data << "\n";
    }

    examFile.close();

    // 📌 8️⃣ Mensaje de éxito
    QMessageBox::information(this, "Éxito", "El examen ha sido guardado correctamente.");

    // 📌 Regresar a `page_2` después de guardar
    ui->stackedWidget->setCurrentWidget(ui->page_2);
}






void maestros::on_modifyExamBtn_clicked()
{
    // 🔹 Obtener la clase seleccionada en page_2 (tableWidgetClases_Maestros)
    int selectedRow = ui->tableWidgetClases_Maestros->currentRow();

    if (selectedRow == -1) {
        QMessageBox::warning(this, "Error", "Por favor, selecciona una clase antes de continuar.");
        return;
    }

    QString idClaseSeleccionada = ui->tableWidgetClases_Maestros->item(selectedRow, 0)->text().trimmed();
    qDebug() << "📌 Clase seleccionada: " << idClaseSeleccionada;

    // 🔹 Cargar los exámenes desde examenes.txt para esta clase
    QFile examenesFile("examenes.txt");

    if (!examenesFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::critical(this, "Error", "No se pudo abrir examenes.txt");
        return;
    }

    QTextStream examenesStream(&examenesFile);
    ui->tableWidgetExams->setRowCount(0); // Limpiar tabla

    //bool esPrimeraLinea = true;
    int row = 0;
    bool examenesEncontrados = false; // Para verificar si encontramos exámenes

    while (!examenesStream.atEnd()) {
        QString line = examenesStream.readLine().trimmed();
        if (line.isEmpty()) continue;

        QStringList parts = line.split(",");

        // 🔹 Verificar si la línea es de un examen (mínimo 6 columnas, sin contar el username del maestro)
        if (parts.size() >= 6) {
            QString idClaseExamen = parts[0].trimmed();

            qDebug() << "🔎 ID en examenes.txt:" << idClaseExamen;

            if (idClaseExamen == idClaseSeleccionada) {
                examenesEncontrados = true; // Se encontró al menos un examen para esta clase

                // 🔹 Agregar examen a tableWidgetExams (sin el username del maestro)
                ui->tableWidgetExams->insertRow(row);

                for (int col = 0; col < 5; ++col) { // Tomar solo los primeros 5 valores
                    ui->tableWidgetExams->setItem(row, col, new QTableWidgetItem(parts[col]));
                }

                row++;
            }
        }
    }

    examenesFile.close();

    // 🔹 Si no se encontraron exámenes, mostrar mensaje y detener ejecución
    if (!examenesEncontrados) {
        QMessageBox::information(this, "Aviso", "No hay exámenes registrados para esta clase.");
        qDebug() << "⚠️ No se encontraron exámenes para: " << idClaseSeleccionada;
        return;  // 🚨 Evita que avance a page_5 si no hay exámenes
    }

    // 🔹 Ajustar tamaño de las columnas después de llenar la tabla
    ui->tableWidgetExams->setColumnWidth(0, 110);  // ID Clase
    ui->tableWidgetExams->setColumnWidth(1, 180); // Nombre Clase
    ui->tableWidgetExams->setColumnWidth(2, 200); // Fecha/Hora
    ui->tableWidgetExams->setColumnWidth(3, 110); // Duración
    ui->tableWidgetExams->setColumnWidth(4, 80);  // Puntaje

    // 🔹 Ir a page_5
    ui->stackedWidget->setCurrentWidget(ui->page_5);
}



void maestros::on_continueModify_clicked()
{
    int selectedRow = ui->tableWidgetExams->currentRow();

    if (selectedRow == -1) {
        QMessageBox::warning(this, "Error", "Por favor, selecciona un examen antes de continuar.");
        return;
    }

    // Obtener datos del examen seleccionado
    QString idClase = ui->tableWidgetExams->item(selectedRow, 0)->text().trimmed();
    QString nombreClase = ui->tableWidgetExams->item(selectedRow, 1)->text().trimmed();
    QString fechaHora = ui->tableWidgetExams->item(selectedRow, 2)->text().trimmed();
    QString duracion = ui->tableWidgetExams->item(selectedRow, 3)->text().trimmed();
    QString puntaje = ui->tableWidgetExams->item(selectedRow, 4)->text().trimmed();

    // Convertir la fecha y hora a QDateTime
    QDateTime fechaHoraConvertida = QDateTime::fromString(fechaHora, "yyyy-MM-dd HH:mm:ss");
    QDateTime currentDateTime = QDateTime::currentDateTime();
    QDateTime minAllowedTime = currentDateTime.addSecs(2 * 60); // 5 minutos después de la hora actual

    // 📌 1️⃣ Verificar primero si la fecha/hora del examen YA PASÓ
    if (fechaHoraConvertida < currentDateTime) {
        QMessageBox::warning(this, "Error", "No puedes modificar un examen cuya fecha y hora ya han pasado.");
        return;
    }

    // 📌 2️⃣ Luego verificar que la nueva hora del examen sea al menos 5 minutos después de la actual
    if (fechaHoraConvertida < minAllowedTime) {
        QMessageBox::warning(this, "Error", "La nueva hora del examen debe ser al menos 2 minutos después de la hora actual.");
        return;
    }

    // 📌 3️⃣ Verificar que la duración contenga solo números y sea positiva
    bool isMinutesNumber;
    int minutes = duracion.toInt(&isMinutesNumber);

    if (duracion.isEmpty() || !isMinutesNumber || minutes <= 0) {
        QMessageBox::warning(this, "Error", "Ingrese un valor válido para la duración del examen (solo números positivos).");
        return;
    }

    // 📌 4️⃣ Verificar que el puntaje esté entre 1 y 100
    bool isPuntajeNumber;
    int puntajeValue = puntaje.toInt(&isPuntajeNumber);

    if (puntaje.isEmpty() || !isPuntajeNumber || puntajeValue <= 0 || puntajeValue > 100) {
        QMessageBox::warning(this, "Error", "Ingrese un puntaje válido (entre 1 y 100).");
        return;
    }

    // 🔹 Llenar los campos en page_6
    ui->dateTimeEdit_2->setDateTime(fechaHoraConvertida); // Establecer fecha y hora
    ui->minutesLineEdit_2->setText(duracion); // Duración en minutos
    ui->puntajeLineEdit_2->setText(puntaje);  // Puntaje

    // 🔹 Configurar la restricción de tiempo en `dateTimeEdit_2`
    ui->dateTimeEdit_2->setMinimumDateTime(minAllowedTime);

    // Cambiar a page_6
    ui->stackedWidget->setCurrentWidget(ui->page_6);
}



void maestros::on_configExamBtn_2_clicked()
{
    int selectedRow = ui->tableWidgetExams->currentRow();
    if (selectedRow == -1) {
        QMessageBox::warning(this, "Error", "Por favor, selecciona un examen antes de continuar.");
        return;
    }

    // 1) Guardar la fecha/hora VIEJA (clave para coincidir con examenes.txt).
    QString oldFechaHora = ui->tableWidgetExams->item(selectedRow, 2)->text().trimmed();

    // 2) Tomar los valores NUEVOS de page_6 (sin aplicarlos aún a la tabla)
    QString newFechaHora = ui->dateTimeEdit_2->dateTime().toString("yyyy-MM-dd HH:mm:ss");
    QString newDuracion  = ui->minutesLineEdit_2->text().trimmed();
    QString newPuntaje   = ui->puntajeLineEdit_2->text().trimmed();

    // 3) (OPCIONAL) Comentar o eliminar la parte que modifica la fecha/hora en la tabla
    //    para no perder la "clave" vieja.
    /*
    ui->tableWidgetExams->setItem(selectedRow, 2, new QTableWidgetItem(newFechaHora));
    ui->tableWidgetExams->setItem(selectedRow, 3, new QTableWidgetItem(newDuracion));
    ui->tableWidgetExams->setItem(selectedRow, 4, new QTableWidgetItem(newPuntaje));
    */

    // 4) Mantenemos idClase, nombreClase y la fechaHora VIEJA para buscar
    QString idClase     = ui->tableWidgetExams->item(selectedRow, 0)->text().trimmed();
    QString nombreClase = ui->tableWidgetExams->item(selectedRow, 1)->text().trimmed();
    // En lugar de leer "fechaHora" de la columna 2, usamos la VIEJA
    QString fechaHora   = oldFechaHora;

    // 5) Convertimos el nuevo puntaje a int y validamos
    bool isPuntajeNumber = false;
    int nuevoPuntajeInt  = newPuntaje.toInt(&isPuntajeNumber);
    if (!isPuntajeNumber || nuevoPuntajeInt < 1 || nuevoPuntajeInt > 100) {
        QMessageBox::warning(this, "Error", "Ingrese un puntaje válido (entre 1 y 100).");
        return;
    }

    // 6) Llamar a "obtenerPuntajeTotalDeClaseExcept" para excluir este examen (por su oldFechaHora)
    int totalSinEste = obtenerPuntajeTotalDeClaseExcept(idClase, nombreClase, oldFechaHora);

    // 7) Sumamos el nuevo puntaje
    int nuevoTotal = totalSinEste + nuevoPuntajeInt;
    if (nuevoTotal > 100) {
        QMessageBox::warning(this, "Error",
                             "El puntaje total de los exámenes en esta clase no puede exceder 100."
                             );
        return;
    }

    // Antes de crear tu scrollArea:
    QLayout *oldLayout = ui->page_7->layout();
    if (oldLayout) {
        QLayoutItem *item;
        while ((item = oldLayout->takeAt(0)) != nullptr) {
            if (item->widget() != nullptr) {
                item->widget()->deleteLater();
            }
            delete item;
        }
        delete oldLayout;
    }


    // ==================================================================================
    //                          CREACIÓN DE LA INTERFAZ (QScrollArea)
    // ==================================================================================

    // 🔹 Crear QScrollArea para contener las preguntas
    QScrollArea *scrollArea = new QScrollArea(ui->page_7);
    scrollArea->setWidgetResizable(true);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scrollArea->setGeometry(50, 50, 800, 500);

    // 🔹 Crear QWidget dentro del QScrollArea
    QWidget *scrollWidget = new QWidget();
    scrollWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    scrollWidget->setMinimumWidth(scrollArea->width());

    // 🔹 Layout vertical para todas las preguntas
    QVBoxLayout *layout = new QVBoxLayout(scrollWidget);
    scrollWidget->setLayout(layout);
    scrollArea->setWidget(scrollWidget);

    // 🔹 Estilos
    scrollArea->setStyleSheet("QScrollArea { background-color: #FAFAFB; border: none; }");
    scrollWidget->setStyleSheet("QWidget { background-color: #FAFAFB; }");

    // 🔹 Agregar QLabel con la imagen de “Preguntas”
    QLabel *titleLabel = new QLabel();
    QPixmap titlePixmap(":/Resources/Preguntas.png");
    titleLabel->setPixmap(titlePixmap.scaled(438, 44, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    titleLabel->setAlignment(Qt::AlignCenter);

    layout->addWidget(titleLabel);
    layout->insertWidget(0, titleLabel, 0, Qt::AlignTop); // Fijar el título arriba

    // Estilos para widgets dentro de scrollWidget
    scrollWidget->setStyleSheet(R"(
        QWidget {
            font-family: 'Arial Rounded MT Bold';
            font-size: 14px;
            color: #1A2B57;
            background-color: #FAFAFB;
        }
        QLabel, QLineEdit, QComboBox, QRadioButton, QCheckBox, QPushButton {
            font-family: 'Arial Rounded MT Bold';
            font-size: 14px;
            color: #1A2B57;
        }
        QLineEdit {
            border: 2px solid #1A2B57;
            padding: 6px;
            border-radius: 5px;
            background-color: white;
        }
        QComboBox {
            border: 2px solid #1A2B57;
            padding: 6px;
            border-radius: 5px;
            background-color: white;
        }
        QPushButton {
            background-color: #5EACD3;
            color: white;
            border-radius: 5px;
            padding: 6px;
            font-weight: bold;
            border: none;
        }
        QPushButton:hover {
            background-color: #4B97BE;
        }
        QPushButton:pressed {
            background-color: #3A85A9;
        }
        QCheckBox::indicator, QRadioButton::indicator {
            width: 18px;
            height: 18px;
            border: 2px solid #1A2B57;
            background-color: white;
            border-radius: 4px;
        }
        QCheckBox::indicator:checked, QRadioButton::indicator:checked {
            background-color: #1A2B57;
            border: 2px solid #1A2B57;
        }
        QCheckBox::indicator:hover, QRadioButton::indicator:hover {
            border: 2px solid #2C3E70;
        }
    )");

    // ==================================================================================
    //               CREACIÓN DEL “WIDGET DE BOTONES” (parte inferior)
    // ==================================================================================

    QWidget *buttonsWidget = new QWidget(ui->page_7);
    QHBoxLayout *buttonsLayout = new QHBoxLayout(buttonsWidget);

    QSpacerItem *leftSpacer   = new QSpacerItem(100, 0, QSizePolicy::Expanding, QSizePolicy::Minimum);
    QSpacerItem *middleSpacer = new QSpacerItem(362, 0, QSizePolicy::Fixed, QSizePolicy::Minimum);
    QSpacerItem *rightSpacer  = new QSpacerItem(100, 0, QSizePolicy::Expanding, QSizePolicy::Minimum);

    // Botón "+ New Question"
    QPushButton *addQuestionBtn = new QPushButton("", buttonsWidget);
    addQuestionBtn->setFixedSize(165, 45);
    addQuestionBtn->setText("+ New Question"); // Texto opcional

    // Botón "Guardar"
    QPushButton *saveBtn = new QPushButton("", buttonsWidget);
    saveBtn->setFixedSize(165, 45);
    saveBtn->setText("Guardar");

    connect(saveBtn, &QPushButton::clicked, this, &maestros::on_saveExamConfig_clicked);


    // Agregar botones al layout
    buttonsLayout->addItem(leftSpacer);
    buttonsLayout->addWidget(addQuestionBtn);
    buttonsLayout->addItem(middleSpacer);
    buttonsLayout->addWidget(saveBtn);
    buttonsLayout->addItem(rightSpacer);

    buttonsWidget->setLayout(buttonsLayout);

    // Layout principal de page_7
    QVBoxLayout *mainLayout = new QVBoxLayout();
    mainLayout->addWidget(scrollArea);
    mainLayout->addWidget(buttonsWidget, 0, Qt::AlignBottom);
    ui->page_7->setLayout(mainLayout);

    // ==================================================================================
    //       LÓGICA PARA LEER PREGUNTAS DESDE `examenes.txt` Y CREAR WIDGETS
    // ==================================================================================

    QFile examFile("examenes.txt");
    if (!examFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "Error", "No se pudo abrir examenes.txt");
        return;
    }

    QTextStream in(&examFile);

    bool isExamFound = false;   // Para saber si ya encontramos la línea del examen
    bool readingQuestions = false; // Para saber si estamos leyendo preguntas de este examen

    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        if (line.isEmpty()) continue;  // Omite líneas vacías

        QStringList parts = line.split(",");
        // -----------------------------------------------------------------
        // 1) Si NO hemos encontrado todavía el examen, lo buscamos
        // -----------------------------------------------------------------
        if (!readingQuestions) {
            // Se asume que la línea del examen tiene al menos 6 columnas:
            // [0] idClase, [1] nombreClase, [2] fechaHora, [3] duracion, [4] puntaje, [5..] ...
            if (parts.size() >= 6 &&
                parts[0] == idClase &&
                parts[1] == nombreClase &&
                parts[2] == fechaHora)
            {
                // Coincide el examen seleccionado
                isExamFound = true;
                readingQuestions = true;  // A partir de la siguiente línea, leemos preguntas
                continue;
            }
        }
        // -----------------------------------------------------------------
        // 2) Si estamos en modo lectura de preguntas
        // -----------------------------------------------------------------
        else {
            // Si volvemos a encontrar una línea que tenga al menos 6 columnas,
            // asumimos que es la definición de OTRO examen => terminamos
            if (parts.size() >= 6) {
                break;
            }
            // Verificamos si es una línea con al menos 3 columnas para una pregunta
            // [0] TextoPregunta, [1] Tipo (Verdadero/Falso, Selec. Multiple, etc.), [2] RespuestaCorrecta
            // Si tu archivo tiene más columnas (por ejemplo para las opciones), adáptalo.
            if (parts.size() < 3) {
                // Si no cumple, rompemos. Podrías adaptar esta condición si tu archivo
                // maneja otras variantes.
                break;
            }

            // Extraer datos de la pregunta
            QString questionText = parts[0].trimmed();
            QString questionType = parts[1].trimmed();
            QString correctAns   = parts[2].trimmed();

            // ---------------------------------------------------------------------
            // Crear un "questionWidget" como en tu lambda de addQuestionBtn,
            // pero ahora *automáticamente* al leer el archivo
            // ---------------------------------------------------------------------
            QWidget *questionWidget = new QWidget();
            QHBoxLayout *questionLayout = new QHBoxLayout(questionWidget);

            // Campo de texto (pregunta)
            QTextEdit *questionInput = new QTextEdit();
            questionInput->setPlaceholderText("Ingrese su pregunta aqui...");
            questionInput->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
            questionInput->setMinimumHeight(50);
            questionInput->setMaximumHeight(80);
            questionInput->setWordWrapMode(QTextOption::WordWrap);
            questionInput->setText(questionText);  // Rellenar con el texto leído

            // ComboBox para tipo de pregunta
            QComboBox *comboType = new QComboBox();
            comboType->addItem("Verdadero/Falso");   // index 0
            comboType->addItem("Selec. Multiple");   // index 1
            comboType->addItem("Enumeracion");       // index 2

            // Botón borrar pregunta
            QPushButton *deleteBtn = new QPushButton();
            deleteBtn->setFixedSize(30, 30);
            deleteBtn->setIcon(QIcon(":/Resources/trash.png"));
            deleteBtn->setIconSize(QSize(25, 25));
            deleteBtn->setStyleSheet("QPushButton { border: none; background: transparent; }");

            // QStackedWidget para cambiar el "contenido" según el tipo de pregunta
            QStackedWidget *stackedWidget = new QStackedWidget();

            // 1) Verdadero/Falso (página 0)
            QWidget *trueFalsePage = new QWidget();
            QHBoxLayout *trueFalseLayout = new QHBoxLayout(trueFalsePage);
            QRadioButton *trueOption  = new QRadioButton("True");
            QRadioButton *falseOption = new QRadioButton("False");
            trueFalseLayout->addWidget(trueOption);
            trueFalseLayout->addWidget(falseOption);
            trueFalsePage->setLayout(trueFalseLayout);
            stackedWidget->addWidget(trueFalsePage);

            // Si la respuesta correcta era "True" o "False", podrías marcarlo:
            if (correctAns.compare("True", Qt::CaseInsensitive) == 0) {
                trueOption->setChecked(true);
            } else if (correctAns.compare("False", Qt::CaseInsensitive) == 0) {
                falseOption->setChecked(true);
            }

            // 2) Selección Múltiple (página 1)
            QWidget *multipleChoicePage = new QWidget();
            QVBoxLayout *multiChoiceLayout = new QVBoxLayout(multipleChoicePage);
            QPushButton *addMultiOptionBtn = new QPushButton("+ Agregar Opcion");

            // Función lambda para crear una opción en "Selec. Multiple"
            auto addMultipleChoiceOption = [multiChoiceLayout]() {
                QWidget *optionWidget = new QWidget();
                QHBoxLayout *optionLayout = new QHBoxLayout(optionWidget);

                QCheckBox *optionCheck = new QCheckBox();
                QLineEdit *optionInput = new QLineEdit();
                optionInput->setMinimumWidth(150);

                QPushButton *deleteOptionBtn = new QPushButton("Borrar");

                // Eliminar opción
                connect(deleteOptionBtn, &QPushButton::clicked, optionWidget,
                        [optionWidget, multiChoiceLayout]() {
                            multiChoiceLayout->removeWidget(optionWidget);
                            optionWidget->deleteLater();
                        });

                optionLayout->addWidget(optionCheck);
                optionLayout->addWidget(optionInput);
                optionLayout->addWidget(deleteOptionBtn);
                optionWidget->setLayout(optionLayout);

                // Insertar antes del botón "+ Agregar Opcion"
                multiChoiceLayout->insertWidget(multiChoiceLayout->count() - 1, optionWidget);
            };

            connect(addMultiOptionBtn, &QPushButton::clicked, [=]() { addMultipleChoiceOption(); });

            // Añadimos 3 opciones por defecto (o según las que hayas guardado en el archivo)
            // for (int i = 0; i < 3; i++) {
            //     addMultipleChoiceOption();
            // }

            //multiChoiceLayout->addWidget(addMultiOptionBtn);
            stackedWidget->addWidget(multipleChoicePage);

            // 3) Enumeración (página 2)
            QWidget *enumerationPage = new QWidget();
            QVBoxLayout *enumLayout = new QVBoxLayout(enumerationPage);
            QPushButton *addEnumOptionBtn = new QPushButton("+ Agregar Opcion");

            auto addEnumerationOption = [enumLayout]() {
                QWidget *enumOptionWidget = new QWidget();
                QHBoxLayout *enumOptionLayout = new QHBoxLayout(enumOptionWidget);

                QLineEdit *enumInput = new QLineEdit();
                enumInput->setMinimumWidth(150);
                QPushButton *deleteEnumBtn = new QPushButton("Borrar");

                connect(deleteEnumBtn, &QPushButton::clicked, enumOptionWidget,
                        [enumOptionWidget, enumLayout]() {
                            enumLayout->removeWidget(enumOptionWidget);
                            enumOptionWidget->deleteLater();
                        });

                enumOptionLayout->addWidget(enumInput);
                enumOptionLayout->addWidget(deleteEnumBtn);
                enumOptionWidget->setLayout(enumOptionLayout);

                enumLayout->insertWidget(enumLayout->count() - 1, enumOptionWidget);
            };

            connect(addEnumOptionBtn, &QPushButton::clicked, [=]() { addEnumerationOption(); });

            // Añade 3 campos por defecto (o ajusta según tu archivo)
            // for (int i = 0; i < 3; i++) {
            //     addEnumerationOption();
            // }

            // enumLayout->addWidget(addEnumOptionBtn);
            stackedWidget->addWidget(enumerationPage);

            // (IMPORTANTE) Conectar el comboBox con el stackedWidget
            connect(comboType, QOverload<int>::of(&QComboBox::currentIndexChanged),
                    stackedWidget, &QStackedWidget::setCurrentIndex);

            // (IMPORTANTE) Forzar la página en stackedWidget según el "questionType"
            if (questionType.compare("Verdadero/Falso", Qt::CaseInsensitive) == 0) {
                comboType->setCurrentIndex(0);
                stackedWidget->setCurrentIndex(0);
            }
            else if (questionType.compare("Selec. Multiple", Qt::CaseInsensitive) == 0) {
                comboType->setCurrentIndex(1);
                stackedWidget->setCurrentIndex(1);

                QStringList optionParts = correctAns.split(";", Qt::SkipEmptyParts);

                // (2) Por cada trozo, crear un widget
                for (const QString &part : optionParts) {
                    // p.e. "CORRECTO:Correcto"
                    QStringList sub = part.split(":", Qt::KeepEmptyParts);
                    if (sub.size() == 2) {
                        QString textoOpcion = sub[0].trimmed(); // "CORRECTO"
                        QString indicador   = sub[1].trimmed(); // "Correcto" o "Incorrecto"

                        // Creamos la opción
                        QWidget *optionWidget = new QWidget();
                        QHBoxLayout *optionLayout = new QHBoxLayout(optionWidget);

                        QCheckBox *optionCheck = new QCheckBox();
                        QLineEdit *optionInput = new QLineEdit();
                        optionInput->setMinimumWidth(150);

                        QPushButton *deleteOptionBtn = new QPushButton("Borrar");
                        connect(deleteOptionBtn, &QPushButton::clicked, optionWidget,
                                [optionWidget, multiChoiceLayout]() {
                                    multiChoiceLayout->removeWidget(optionWidget);
                                    optionWidget->deleteLater();
                                });

                        // Rellenar
                        optionInput->setText(textoOpcion);
                        if (indicador.compare("Correcto", Qt::CaseInsensitive) == 0) {
                            optionCheck->setChecked(true);
                        } else {
                            optionCheck->setChecked(false);
                        }

                        optionLayout->addWidget(optionCheck);
                        optionLayout->addWidget(optionInput);
                        optionLayout->addWidget(deleteOptionBtn);
                        optionWidget->setLayout(optionLayout);

                        // Insertar antes del botón + Agregar Opcion
                        multiChoiceLayout->insertWidget(multiChoiceLayout->count() - 1, optionWidget);
                    }
                }

                // Finalmente, al final del layout, añadimos el botón "+ Agregar Opcion"
                multiChoiceLayout->addWidget(addMultiOptionBtn);
            }
            else if (questionType.compare("Enumeracion", Qt::CaseInsensitive) == 0) {
                comboType->setCurrentIndex(2);
                stackedWidget->setCurrentIndex(2);
                // Parsear correctAns por ";"
                // Ej: "UNO;DOS;"
                QStringList items = correctAns.split(";", Qt::SkipEmptyParts);
                for (const QString &it : items) {
                    // p.e. "UNO"
                    QString texto = it.trimmed();
                    if (!texto.isEmpty()) {
                        QWidget *enumOptionWidget = new QWidget();
                        QHBoxLayout *enumOptionLayout = new QHBoxLayout(enumOptionWidget);

                        QLineEdit *enumInput = new QLineEdit();
                        enumInput->setMinimumWidth(150);
                        enumInput->setText(texto);

                        QPushButton *deleteEnumBtn = new QPushButton("Borrar");
                        connect(deleteEnumBtn, &QPushButton::clicked, enumOptionWidget,
                                [enumOptionWidget, enumLayout]() {
                                    enumLayout->removeWidget(enumOptionWidget);
                                    enumOptionWidget->deleteLater();
                                });

                        enumOptionLayout->addWidget(enumInput);
                        enumOptionLayout->addWidget(deleteEnumBtn);
                        enumOptionWidget->setLayout(enumOptionLayout);

                        // Insertar antes del botón "+ Agregar Opcion"
                        enumLayout->insertWidget(enumLayout->count() - 1, enumOptionWidget);
                    }
                }

                // Finalmente, metemos el botón "+ Agregar Opcion"
                enumLayout->addWidget(addEnumOptionBtn);
            }

            // Borrar la pregunta completa al dar click en “eliminar”
            connect(deleteBtn, &QPushButton::clicked, questionWidget, &QWidget::deleteLater);

            // Agregar los widgets al layout de la pregunta
            questionLayout->addWidget(questionInput);
            questionLayout->addWidget(comboType);
            questionLayout->addWidget(stackedWidget);
            questionLayout->addWidget(deleteBtn);

            questionWidget->setLayout(questionLayout);
            layout->addWidget(questionWidget);
        }

    }

    examFile.close();

    // ==================================================================================
    //     CONECTAR EL BOTÓN “+ NEW QUESTION” PARA AGREGAR PREGUNTAS MANUALMENTE
    // ==================================================================================
    // (Solo se hace una vez, fuera del bucle, para no sobre-conectarlo.)
    connect(addQuestionBtn, &QPushButton::clicked, this, [layout, this]()
            {
                QWidget *questionWidget = new QWidget();
                QHBoxLayout *questionLayout = new QHBoxLayout(questionWidget);

                // Texto de la pregunta
                QTextEdit *questionInput = new QTextEdit();
                questionInput->setPlaceholderText("Ingrese su pregunta aqui...");
                questionInput->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
                questionInput->setMinimumHeight(50);
                questionInput->setMaximumHeight(80);
                questionInput->setWordWrapMode(QTextOption::WordWrap);

                // Tipo de pregunta
                QComboBox *questionType = new QComboBox();
                questionType->addItem("Verdadero/Falso");
                questionType->addItem("Selec. Multiple");
                questionType->addItem("Enumeracion");

                // Botón eliminar
                QPushButton *deleteBtn = new QPushButton();
                deleteBtn->setFixedSize(30, 30);
                deleteBtn->setIcon(QIcon(":/Resources/trash.png"));
                deleteBtn->setIconSize(QSize(25, 25));
                deleteBtn->setStyleSheet("QPushButton { border: none; background: transparent; }");

                // QStackedWidget para cambiar interfaz según tipo de pregunta
                QStackedWidget *stackedWidget = new QStackedWidget();

                // Verdadero/Falso
                QWidget *trueFalsePage = new QWidget();
                QHBoxLayout *trueFalseLayout = new QHBoxLayout(trueFalsePage);
                QRadioButton *trueOption = new QRadioButton("True");
                QRadioButton *falseOption = new QRadioButton("False");
                trueFalseLayout->addWidget(trueOption);
                trueFalseLayout->addWidget(falseOption);
                stackedWidget->addWidget(trueFalsePage);

                // Selección Múltiple
                QWidget *multipleChoicePage = new QWidget();
                QVBoxLayout *multiChoiceLayout = new QVBoxLayout(multipleChoicePage);
                QPushButton *addMultiOptionBtn = new QPushButton("+ Agregar Opcion");

                auto addMultipleChoiceOption = [multiChoiceLayout]() {
                    QWidget *optionWidget = new QWidget();
                    QHBoxLayout *optionLayout = new QHBoxLayout(optionWidget);

                    QCheckBox *optionCheck = new QCheckBox();
                    QLineEdit *optionInput = new QLineEdit();
                    optionInput->setMinimumWidth(150);

                    QPushButton *deleteOptionBtn = new QPushButton("Borrar");
                    connect(deleteOptionBtn, &QPushButton::clicked, optionWidget, [optionWidget, multiChoiceLayout]() {
                        multiChoiceLayout->removeWidget(optionWidget);
                        optionWidget->deleteLater();
                    });

                    optionLayout->addWidget(optionCheck);
                    optionLayout->addWidget(optionInput);
                    optionLayout->addWidget(deleteOptionBtn);
                    optionWidget->setLayout(optionLayout);

                    multiChoiceLayout->insertWidget(multiChoiceLayout->count() - 1, optionWidget);
                };

                connect(addMultiOptionBtn, &QPushButton::clicked, [=]() { addMultipleChoiceOption(); });

                // Por defecto, 3 opciones
                for (int i = 0; i < 3; i++) {
                    addMultipleChoiceOption();
                }

                multiChoiceLayout->addWidget(addMultiOptionBtn);
                stackedWidget->addWidget(multipleChoicePage);

                // Enumeración
                QWidget *enumerationPage = new QWidget();
                QVBoxLayout *enumLayout = new QVBoxLayout(enumerationPage);
                QPushButton *addEnumOptionBtn = new QPushButton("+ Agregar Opcion");

                auto addEnumerationOption = [enumLayout]() {
                    QWidget *enumOptionWidget = new QWidget();
                    QHBoxLayout *enumOptionLayout = new QHBoxLayout(enumOptionWidget);

                    QLineEdit *enumInput = new QLineEdit();
                    enumInput->setMinimumWidth(150);
                    QPushButton *deleteEnumBtn = new QPushButton("Borrar");

                    connect(deleteEnumBtn, &QPushButton::clicked, enumOptionWidget, [enumOptionWidget, enumLayout]() {
                        enumLayout->removeWidget(enumOptionWidget);
                        enumOptionWidget->deleteLater();
                    });

                    enumOptionLayout->addWidget(enumInput);
                    enumOptionLayout->addWidget(deleteEnumBtn);
                    enumOptionWidget->setLayout(enumOptionLayout);

                    enumLayout->insertWidget(enumLayout->count() - 1, enumOptionWidget);
                };

                connect(addEnumOptionBtn, &QPushButton::clicked, [=]() { addEnumerationOption(); });

                // 3 campos por defecto
                for (int i = 0; i < 3; i++) {
                    addEnumerationOption();
                }

                enumLayout->addWidget(addEnumOptionBtn);
                stackedWidget->addWidget(enumerationPage);

                // Cambiar página del stackedWidget según el comboBox
                connect(questionType, QOverload<int>::of(&QComboBox::currentIndexChanged),
                        stackedWidget, &QStackedWidget::setCurrentIndex);

                // Eliminar la pregunta completa
                connect(deleteBtn, &QPushButton::clicked, questionWidget, &QWidget::deleteLater);

                // Agregar al layout
                questionLayout->addWidget(questionInput);
                questionLayout->addWidget(questionType);
                questionLayout->addWidget(stackedWidget);
                questionLayout->addWidget(deleteBtn);

                questionWidget->setLayout(questionLayout);
                layout->addWidget(questionWidget);
            });

    // Finalmente, mostramos la página 7
    ui->stackedWidget->setCurrentWidget(ui->page_7);
}

void maestros::on_saveExamConfig_clicked()
{
    // ============================================================================
    // 0. Verificar que haya un examen seleccionado (si estás editando un examen existente)
    // ============================================================================
    int selectedRow = ui->tableWidgetExams->currentRow();
    if (selectedRow == -1) {
        QMessageBox::warning(this, "Error", "No hay ningún examen seleccionado para guardar/actualizar.");
        return;
    }

    // ============================================================================
    // 1. Buscar el contenedor donde se agregan las preguntas
    //    (ajusta el objeto 'page_7' o 'page_4' según dónde tengas tus preguntas)
    // ============================================================================
    QScrollArea *scrollArea = ui->page_7->findChild<QScrollArea *>();
    if (!scrollArea) {
        QMessageBox::warning(this, "Error", "No se encontró el área de preguntas en page_7.");
        return;
    }

    QWidget *scrollWidget = scrollArea->widget();
    if (!scrollWidget) {
        QMessageBox::warning(this, "Error", "No se encontró el contenedor de preguntas.");
        return;
    }

    QVBoxLayout *layout = qobject_cast<QVBoxLayout *>(scrollWidget->layout());
    if (!layout) {
        QMessageBox::warning(this, "Error", "No se pudo acceder al layout de preguntas.");
        return;
    }

    // ============================================================================
    // 2. Verificar que haya al menos una pregunta
    // ============================================================================
    if (layout->count() == 0) {
        QMessageBox::warning(this, "Error", "Debe agregar al menos una pregunta antes de guardar.");
        return;
    }

    // ============================================================================
    // 3. Obtener los datos del examen actual
    // ============================================================================
    QString idClase     = ui->tableWidgetExams->item(selectedRow, 0)->text().trimmed();
    QString nombreClase = ui->tableWidgetExams->item(selectedRow, 1)->text().trimmed();
    QString fechaHora   = ui->tableWidgetExams->item(selectedRow, 2)->text().trimmed();
    QString duracion    = ui->tableWidgetExams->item(selectedRow, 3)->text().trimmed();
    QString puntaje     = ui->tableWidgetExams->item(selectedRow, 4)->text().trimmed();

    // Si el usuario tecleó nuevos valores en los lineEdit (por ejemplo, cambio de puntaje o duración),
    // toma esos en lugar de lo que está en la tabla, por ejemplo:
    // QString examDate = ui->dateTimeEdit->dateTime().toString("yyyy-MM-dd HH:mm:ss");
    // QString examDuration = ui->minutesLineEdit->text().trimmed();
    // QString examScore = ui->puntajeLineEdit->text().trimmed();
    // Pero aquí, para el ejemplo, uso directamente lo de la tabla.

    // ============================================================================
    // 4. Validar el puntaje nuevo y comprobar que la suma no exceda 100
    // ============================================================================
    bool isNumber = false;
    int nuevoPuntaje = puntaje.toInt(&isNumber);
    if (!isNumber || nuevoPuntaje < 1 || nuevoPuntaje > 100) {
        QMessageBox::warning(this, "Error", "El puntaje debe ser un número válido entre 1 y 100.");
        return;
    }

    // a) Obtener el puntaje total de la clase (función que ya tienes)
    int puntajeTotalClase = obtenerPuntajeTotalDeClase(idClase);

    // b) Obtener el puntaje anterior de *este* examen para recalcular
    //    (leerlo de la tabla o del archivo; aquí lo asumo en la variable 'puntajeAnterior')
    int puntajeAnterior = puntaje.toInt(); // De la tabla, p. ej.

    // c) Calcular el nuevo total
    int nuevoTotal = puntajeTotalClase - puntajeAnterior + nuevoPuntaje;
    if (nuevoTotal > 100) {
        QMessageBox::warning(this, "Error", "La suma de puntajes de los exámenes en esta clase supera 100.");
        return;
    }

    // ============================================================================
    // 5. Validar las preguntas
    // ============================================================================
    QStringList nuevasLineasPreguntas;  // Aquí almacenamos cada línea "pregunta" que escribiremos

    for (int i = 0; i < layout->count(); ++i) {
        QWidget *widget = layout->itemAt(i)->widget();
        if (!widget) continue;

        // Encontrar los componentes de la pregunta
        QTextEdit   *questionInput = widget->findChild<QTextEdit   *>();
        QComboBox   *questionType  = widget->findChild<QComboBox   *>();
        QStackedWidget *stackedWidget = widget->findChild<QStackedWidget *>();

        if (!questionInput || !questionType || !stackedWidget) continue;

        QString questionText = questionInput->toPlainText().trimmed();
        QString type         = questionType->currentText();
        QString answer       = "";

        if (questionText.isEmpty()) {
            QMessageBox::warning(this, "Error", "No puede haber preguntas vacías.");
            return;
        }

        // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
        // Validar según el tipo:
        // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
        if (type == "Verdadero/Falso") {
            QWidget *trueFalsePage = stackedWidget->widget(0);
            QList<QRadioButton*> radioButtons = trueFalsePage->findChildren<QRadioButton*>();

            bool isChecked = false;
            for (QRadioButton *rb : radioButtons) {
                if (rb->isChecked()) {
                    answer = rb->text(); // "True" o "False"
                    isChecked = true;
                    break;
                }
            }

            if (!isChecked) {
                QMessageBox::warning(this, "Error",
                                     "Debe seleccionar True o False en la pregunta de Verdadero/Falso.");
                return;
            }

        } else if (type == "Selec. Multiple") {
            QWidget *multipleChoicePage = stackedWidget->widget(1);
            QList<QCheckBox *> checkboxes = multipleChoicePage->findChildren<QCheckBox *>();
            QList<QLineEdit *> options    = multipleChoicePage->findChildren<QLineEdit *>();

            if (checkboxes.size() != options.size()) {
                QMessageBox::warning(this, "Error",
                                     "Error interno: el número de checkboxes no coincide con el número de lineEdits.");
                return;
            }

            bool alMenosUnaCorrecta = false;
            for (int k = 0; k < options.size(); ++k) {
                QString optText = options[k]->text().trimmed();
                if (!optText.isEmpty()) {
                    bool isCorrect = checkboxes[k]->isChecked();
                    // Ejemplo de codificación "Opción:Correcto;" o "Opción:Incorrecto;"
                    answer += optText + ":" + (isCorrect ? "Correcto" : "Incorrecto") + ";";
                    if (isCorrect) {
                        alMenosUnaCorrecta = true;
                    }
                } else {
                    QMessageBox::warning(this, "Error",
                                         "Hay campos vacíos en la pregunta de Selección Múltiple.");
                    return;
                }
            }

            if (!alMenosUnaCorrecta) {
                QMessageBox::warning(this, "Error",
                                     "Debe haber al menos una opción marcada como correcta en Selección Múltiple.");
                return;
            }

        } else if (type == "Enumeracion") {
            QWidget *enumerationPage = stackedWidget->widget(2);
            QList<QLineEdit*> enumOptions = enumerationPage->findChildren<QLineEdit*>();

            for (QLineEdit *opt : enumOptions) {
                QString optText = opt->text().trimmed();
                if (!optText.isEmpty()) {
                    answer += optText + ";";
                } else {
                    QMessageBox::warning(this, "Error",
                                         "Hay campos vacíos en la pregunta de Enumeración.");
                    return;
                }
            }
        }

        // Construimos la línea que representará esta pregunta (formato adaptado a tu .txt):
        // Ej.: "TextoPregunta,TipoPregunta,Respuesta..."
        nuevasLineasPreguntas << (questionText + "," + type + "," + answer);
    }

    // ============================================================================
    // 6. Reescribir el archivo examenes.txt (ACTUALIZAR)
    //    - Leemos todas las líneas
    //    - Omitimos las que pertenecen a ESTE examen (idClase,nombreClase,fechaHora,...)
    //    - Al final, escribimos las nuevas del examen actualizado
    // ============================================================================
    QFile examFile("examenes.txt");
    if (!examFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "Error", "No se pudo abrir examenes.txt en modo lectura.");
        return;
    }

    QTextStream in(&examFile);
    QStringList todasLasLineas;
    while (!in.atEnd()) {
        QString line = in.readLine();
        todasLasLineas << line;
    }
    examFile.close();

    // Forma de identificar la línea principal del examen:
    // Normalmente es:  idClase,nombreClase,fechaHora,duracion,puntaje,maestroUsername
    // Así que, para considerarla "la misma", chequeamos que empiece con
    // "idClase,nombreClase,fechaHora" (o con 5+ columnas que coincidan).
    //
    // Asumimos que el exam principal está en la primera linea
    // y luego sus preguntas en líneas con < 6 columnas.
    // Este es un ejemplo: adáptalo a tu criterio de reconocimiento.

    QString cabeceraExamenActual = idClase + "," + nombreClase + "," + fechaHora;
    QStringList lineasFiltradas;

    bool saltandoPreguntasAnteriores = false;

    for (int i = 0; i < todasLasLineas.size(); i++) {
        QString linea = todasLasLineas[i].trimmed();
        if (linea.isEmpty()) {
            continue;
        }

        QStringList partes = linea.split(",");

        // Detectar si es la cabecera EXACTA de este examen:
        // Mínimo 6 columnas: 0:idClase,1:nombreClase,2:fechaHora,3:duracion,4:puntaje,5:username
        if (partes.size() >= 6 &&
            partes[0] == idClase &&
            partes[1] == nombreClase &&
            partes[2] == fechaHora)
        {
            // Encontramos la cabecera del examen que se está editando => no copiar
            // y a partir de la siguiente, ignorar sus preguntas
            saltandoPreguntasAnteriores = true;
            continue;
        }

        // Si ya estamos saltando preguntas, paramos de saltar cuando aparezca
        // otra cabecera (>=6 columnas).
        if (saltandoPreguntasAnteriores) {
            if (partes.size() >= 6) {
                // Esta línea es la cabecera de OTRO examen => ya no saltamos
                saltandoPreguntasAnteriores = false;
                // Copiamos esta línea (porque es la cabecera de un examen distinto)
                lineasFiltradas << linea;
            }
            else {
                // Son preguntas del examen anterior => no copiar
            }
        } else {
            // Si NO estamos saltando, copiamos
            lineasFiltradas << linea;
        }
    }

    // Ahora reabrimos el archivo para escritura
    if (!examFile.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        QMessageBox::warning(this, "Error", "No se pudo abrir examenes.txt en modo escritura.");
        return;
    }

    QTextStream out(&examFile);

    // 6.a) Escribir las líneas que NO pertenecían a este examen (las que se conservaron)
    for (const QString &l : lineasFiltradas) {
        if (!l.trimmed().isEmpty()) {
            out << l << "\n";
        }
    }

    // 6.b) Escribir la cabecera nueva del examen actualizado
    // Asegúrate de usar la info actualizada: p.ej. duracion y puntaje, si las cambiaste
    // Ejemplo: idClase, nombreClase, fechaHora, duracion, nuevoPuntaje, username
    QString usernameMaestro = username; // Ajusta si tu variable se llama distinto
    out << idClase << "," << nombreClase << "," << fechaHora << ","
        << duracion << "," << nuevoPuntaje << "," << usernameMaestro << "\n";

    // 6.c) Escribir todas las preguntas nuevas de este examen
    for (const QString &preg : nuevasLineasPreguntas) {
        out << preg << "\n";
    }

    examFile.close();

    // ============================================================================
    // 7. Mensaje de éxito y navegación
    // ============================================================================
    QMessageBox::information(this, "Éxito", "El examen se ha actualizado correctamente.");

    // Si quieres volver a otra página (por ejemplo, page_2):
    ui->stackedWidget->setCurrentWidget(ui->page_2);
}

