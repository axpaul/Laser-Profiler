#include "mainwindow.h"
#include "ui_mainwindow.h"

Q_DECLARE_METATYPE(ASI_ERROR_CODE);

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{

    qRegisterMetaType<ASI_ERROR_CODE>();
    ui->setupUi(this);

    ui->progressBar->setRange(0,100);

    console = ui->console_Window;

    m_infoHome = new QMessageBox(this);

    m_infoHome->setWindowModality(Qt::ApplicationModal);
    m_infoHome->setDefaultButton(QMessageBox::Ok);
    m_infoHome->setWindowTitle("Wait home position");
    m_infoHome->setText("Wait for the camera to return to its initial position. "
"\nIf the camera is in the initial position press ''Ok''. ");

    m_camera = new AsiCamera();
    m_cameraRun = false;
    m_cameraControlChange = false;
    m_videoRun = false;
    m_startMeasure = false;

    m_photo = new QPixmap();
    m_scene = new QGraphicsScene;

    m_imageScene = new ImageScene;
    ui->graphicsView->setScene(m_imageScene);
    ui->graphicsView->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    ui->graphicsView->centerOn(0,0);
    ui->graphicsView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->graphicsView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    m_camera->start();

    m_serial = new SerialPort;
    m_serialRun = false;

    m_serial->start();

    m_motor = new Motor;

    m_motor->start();

    m_measure = new Measure;

    m_measure->start();

    initActionsConnectionsPrio();

    m_settings = new SettingsDialog;
    setSerialSettings();

    m_connection = new QString;
    m_status = new QLabel;

    ui->statusbar->addWidget(m_status);

    initActionsConnections();

    motorbuttonDisactivate();

    qDebug() << "[" << QDateTime::currentDateTime().toString("dd-MM-yyyy_HH.mm.ss") << "][MAINWINDOW] " << QThread::currentThread();
}

MainWindow::~MainWindow(){

    closeSerialPort();

    if (m_cameraRun == true)
        m_camera->askClose();

    /*if (!m_image->isNull())
        delete m_image;*/

    delete m_photo;
    delete m_scene;
    delete m_infoHome;
    delete m_settings;
    delete m_connection;
    delete ui;
}

/* Connect function sender/signal */

void MainWindow::initActionsConnectionsPrio(){

    connect(this, SIGNAL(setSerialSettingsSig(SerialPort::Settings)), m_serial, SLOT(settingUpdate(SerialPort::Settings)));
    connect(m_serial, SIGNAL(errorEmit(QString)), this, SLOT(handleErrorShow(QString)));

}

void MainWindow::initActionsConnections(){

    connect(ui->actionQuit, &QAction::triggered, this, &MainWindow::close);
    connect(ui->actionAbout, &QAction::triggered, this, &MainWindow::about);
    connect(ui->actionConfigure, &QAction::triggered, m_settings, &SettingsDialog::showSetting); // set setting serial
    connect(m_settings, &SettingsDialog::applyParameter, this, &MainWindow::setSerialSettings);

    connect(ui->actionConnect, &QAction::triggered, this, &MainWindow::openSerialPort);
    connect(ui->actionDisconnect, &QAction::triggered, this, &MainWindow::closeSerialPort);

    connect(m_serial, &SerialPort::serialOpenned, this, &MainWindow::opennedSerial);
    //connect(m_serial, SIGNAL(serialOpenned(SerialPort::Settings)), this, SLOT(opennedSerial(SerialPort::Settings)));
    connect(m_serial, &SerialPort::serialClosed, this, &MainWindow::closedSerial);

    connect(m_serial, &SerialPort::serialOpenned, m_motor, &Motor::initMotor);
    connect(m_serial, &SerialPort::serialClosed, m_motor, &Motor::closeSerial);
    connect(m_serial, &SerialPort::errorEmit, m_motor, &Motor::errorSerial);
    connect(m_serial, &SerialPort::dataSend, m_motor, &Motor::serialsendMessage);
    connect(m_motor, &Motor::sendToCmd, m_serial, &SerialPort::pushStack);
    connect(m_motor, &Motor::motorState, this, &MainWindow::showStateMotor);

    connect(m_motor, &Motor::doHome, this, &MainWindow::applyHome);
    connect(this, &MainWindow::sendPosition, m_motor, &Motor::setPosition);
    connect(ui->button_Home, &QPushButton::clicked, m_motor, &Motor::setHome);
    connect(this, &MainWindow::setHomeMeasure, m_motor, &Motor::setHome);
    connect(ui->button_Position, &QPushButton::clicked, this, &MainWindow::applyPosition);

    connect(ui->button_Send, &QPushButton::clicked, this, &MainWindow::cmdToSend);

    connect(this, &MainWindow::sendCommandSerial, m_serial, &SerialPort::pushStack);

    connect(ui->actionClearConsole, &QAction::triggered, ui->console_Window, &QPlainTextEdit::clear);

    connect(m_camera, &AsiCamera::sigClosedCamera, this, &MainWindow::closedCamera);
    connect(m_camera, &AsiCamera::sigOpennedCamera, this, &MainWindow::opennedCamera);

    connect(ui->actionConnect_ZWO,  &QAction::triggered, this, &MainWindow::openCamera);
    connect(ui->actionDisconect_ZWO,  &QAction::triggered, this, &MainWindow::closeCamera);
    connect(ui->actionPhoto, &QAction::triggered, this, &MainWindow::askTakeImage);
    connect(m_camera, &AsiCamera::sigErrorCamera, this, &MainWindow::errorCamera);
    connect(m_camera, SIGNAL(sigInfoCamera(ASI_CAMERA_INFO)), this, SLOT(cameraInfo(ASI_CAMERA_INFO)));
    connect(m_camera, &AsiCamera::sigControlCap, this, &MainWindow::controlCap);
    connect(m_camera, &AsiCamera::sigControlFomat, this, &MainWindow::controlFomat);

    connect(ui->Button_apply_camera, &QPushButton::clicked, this, &MainWindow::allSettings);

    connect(this, &MainWindow::sigAllSettings, m_camera, &AsiCamera::allSettings);

    connect(m_camera, &AsiCamera::sigImageReception, this, &MainWindow::showImage);
    connect(ui->actionVideo, &QAction::triggered, m_camera, &AsiCamera::askVideo);
    connect(ui->actionVideo, &QAction::triggered, this, &MainWindow::buttonVideoActivate);

    connect(ui->button_Start_Measure, &QPushButton::clicked, this, &MainWindow::startMeasure);
    connect(this, &MainWindow::sendStartMeasure, m_measure, &Measure::startMeasure);
    connect(m_measure, &Measure::sigSendPositionMeasure, m_motor, &Motor::setPosition);
    connect(m_motor, &Motor::endMove, m_measure, &Measure::positionReception);
    connect(m_measure, &Measure::sigEndMeasure, this, &MainWindow::endMeasure);
    connect(m_measure, &Measure::sigSendStateMeasure, this, &MainWindow::statutMeasure);

    connect(m_measure, &Measure::sigOpenCameraMeasure, m_camera, &AsiCamera::askConnect);
    connect(m_camera, &AsiCamera::sigOpennedCamera, m_measure, &Measure::oppenedCameraMeasure);
    connect(m_measure, &Measure::sigTakePhotoMeasure, m_camera, &AsiCamera::askPhoto);
    connect(m_camera, &AsiCamera::sigClosedCamera, m_measure, &Measure::closedCameraMeasure);
    connect(m_camera, &AsiCamera::sigImageReception, m_measure, &Measure::imageReception);

    connect(m_camera, &AsiCamera::sigControlValueAlone, m_measure, &Measure::controlValueMeasure);
    connect(m_camera, &AsiCamera::sigcontrolValue, this, &MainWindow::controlValue);

    connect(ui->button_calibration, &QPushButton::clicked, this, &MainWindow::startCalibration);
}

/* MainWindow Information */

void MainWindow::about(){
    QString textAbout;
    textAbout.asprintf("Serial Interface v2/v3 Interface\nVersion : %.1f", VERSION_SERIAL);
    QMessageBox::about(this,"About", textAbout);
}

void MainWindow::showStatusMessage(const QString &stringConnection)
{
    QString message;

    if (stringConnection != "" && stringConnection != *m_connection)
    {
        *m_connection = stringConnection;
    }

    message = QString("%1").arg(*m_connection);

    m_status->setText(message);
}

/* Functions settings systems */

void MainWindow::settingShow(){
    m_settings->show();
}

void MainWindow::setSerialSettings() {
    emit setSerialSettingsSig(m_settings->settings());
}

SerialPort::Settings MainWindow::getSerialInfo() {
    return m_serial->settingsInfo();
}

/* Function open/close serial */

void MainWindow::opennedSerial(SerialPort::Settings p) {
    qDebug() << "[" << QDateTime::currentDateTime().toString("dd-MM-yyyy_HH.mm.ss") << "][MAINWINDOW] Serial openned";
    m_serialRun = true;
    showStatusMessage(QString("Connected to %1 : %2, %3, %4, %5, %6")
                      .arg(p.name).arg(p.stringBaudRate).arg(p.stringDataBits)
                      .arg(p.stringParity).arg(p.stringStopBits).arg(p.stringFlowControl));

    motorbuttonActivate();

}

void MainWindow::closedSerial() {
    qDebug() << "[" << QDateTime::currentDateTime().toString("dd-MM-yyyy_HH.mm.ss")<< "][MAINWINDOW] Serial closed";
    m_serialRun = false;
    m_serial->clearStack();
    showStatusMessage(QString("Disconnected"));

    motorbuttonDisactivate();
}

void MainWindow::openSerialPort() {

    qDebug() << "[" << QDateTime::currentDateTime().toString("dd-MM-yyyy_HH.mm.ss")<< "][MAINWINDOW] Send Serial open";

    m_serial->setSerialRun(true);

    //motorNobuttonAll();
}

void MainWindow::closeSerialPort() {
    qDebug() << "[" << QDateTime::currentDateTime().toString("dd-MM-yyyy_HH.mm.ss")<< "][MAINWINDOW] Send Serial close";
    m_serial->setSerialRun(false);

    motorNobuttonAll();
}

/* Error function */

void MainWindow::handleErrorShow(QString error){
   QMessageBox::critical(this, QString("Error"), error);
}

QString MainWindow::getSerialError() {
    return m_serial->serialError();
}

/* Write function */

void MainWindow::cmdToSend()
{
   QByteArray dataToSend((ui->lineEdit_cmd->text()).toLocal8Bit());
   dataToSend.append("\r");
   qDebug() << "[" << QDateTime::currentDateTime().toString("dd-MM-yyyy_HH.mm.ss")<< "][MAINWINDOW] Send data : " << Qt::hex << dataToSend.toHex();
   QByteArray dataToShow("Send -> ");
   ui->lineEdit_cmd->clear();
   console->putData(dataToShow.append(dataToSend));
   emit sendCommandSerial(dataToSend);
}

/* Motor */

void MainWindow::showStateMotor(const bool state, const double position)
{
    ui->lcdNumber->display(position);

    if (state == true)
        ui->label_state->setText("Motor States : Run");
    else
        ui->label_state->setText("Motor States : Still");
}

void MainWindow::applyHome()
{
    m_infoHome->show();
}

void MainWindow::applyPosition()
{
    float position = ui->SpinBox_Position->value();
    emit sendPosition(position);
}

// Camera control

void MainWindow::errorCamera(ASI_ERROR_CODE error)
{
    switch (error)
    {
    case ASI_SUCCESS:
        break;

    case ASI_ERROR_INVALID_INDEX:
        QMessageBox::warning(this, "Erreur Camera", "No camera connected or index value out of boundary");
        break;

    case ASI_ERROR_INVALID_ID:
        QMessageBox::warning(this, "Erreur Camera", "Invalid ID");
        break;

    case ASI_ERROR_INVALID_CONTROL_TYPE:
        QMessageBox::warning(this, "Erreur Camera", "Invalid control type");
        break;

    case ASI_ERROR_CAMERA_CLOSED:
        QMessageBox::warning(this,"Erreur Camera", "Invalid control type");
        break;

    case ASI_ERROR_CAMERA_REMOVED:
        QMessageBox::warning(this,"Erreur Camera", "Failed to find the camera, maybe the camera has been removed");
        break;

    case ASI_ERROR_INVALID_PATH:
        QMessageBox::warning(this,"Erreur Camera", "Cannot find the path of the file");
        break;

    case ASI_ERROR_INVALID_FILEFORMAT:
        QMessageBox::warning(this,"Erreur Camera", "Invalid file format");
        break;

    case ASI_ERROR_INVALID_SIZE:
        QMessageBox::warning(this, "Erreur Camera", "Wrong video format size");
        break;

    case ASI_ERROR_INVALID_IMGTYPE:
        QMessageBox::warning(this, "Erreur Camera", "Unsupported image format");
        break;

    case ASI_ERROR_OUTOF_BOUNDARY:
        QMessageBox::warning(this, "Erreur Camera", "The starpos is ouside the image boundary");
        break;

    case ASI_ERROR_TIMEOUT:
        QMessageBox::warning(this, "Erreur Camera", "Timeout");
        break;

    case ASI_ERROR_INVALID_SEQUENCE:
        QMessageBox::warning(this, "Erreur Camera", "Stop capture first");
        break;

    case ASI_ERROR_BUFFER_TOO_SMALL:
        QMessageBox::warning(this, "Erreur Camera", "Buffer size is not enought");
        break;

    case ASI_ERROR_VIDEO_MODE_ACTIVE:
        QMessageBox::warning(this, "Erreur Camera", "Video Mode active error");
        break;

    case ASI_ERROR_EXPOSURE_IN_PROGRESS:
        QMessageBox::warning(this, "Erreur Camera", "Error exposure in progress");
        break;

    case ASI_ERROR_GENERAL_ERROR:
        QMessageBox::warning(this, "Erreur Camera", "General error, eg: value is out of valid range");
        break;

    case ASI_ERROR_END:
        QMessageBox::warning(this, "Erreur Camera", "General error, eg: value is out of valid range");
        break;

    case ASI_ERROR_INVALID_MODE:
        QMessageBox::warning(this, "Erreur Camera", "General error, eg: Mode Camera are invalid");
        break;
    }
}

void MainWindow::opennedCamera(bool state)
{
    if (state == true){
        qDebug() << "[" << QDateTime::currentDateTime().toString("dd-MM-yyyy_HH.mm.ss") << "][MAINWINDOW] Camera openned";
        m_cameraRun = true;

        if (m_startMeasure == false)
            buttonCameraActivate();

        if (m_cameraControlChange)
            emit sigAllSettings(ui->spinBox_width->value(), ui->spinBox_Height->value(), 1, ui->spinBox_start_PosX->value(), ui->spinBox_start_PosY->value(), m_format, m_controlValue);
    }
    else{

    }

}

void MainWindow::closedCamera(bool state)
{
    if (state == true){
    qDebug() << "[" << QDateTime::currentDateTime().toString("dd-MM-yyyy_HH.mm.ss") << "][MAINWINDOW] Camera closed";

    m_cameraRun = false;

    if (m_startMeasure == false)
        buttonCameraDesactivate();   
    }

    if(m_videoRun){
        m_videoRun = false;
        buttonVideoDesactivate();
    }
}

void MainWindow::openCamera(){
    qDebug() << "[" << QDateTime::currentDateTime().toString("dd-MM-yyyy_HH.mm.ss")<< "][MAINWINDOW] Send open camera";
    m_camera->askConnect();
}

void MainWindow::closeCamera()
{
    qDebug() << "[" << QDateTime::currentDateTime().toString("dd-MM-yyyy_HH.mm.ss")<< "][MAINWINDOW] Send close camera";
    m_camera->askClose();
}

void MainWindow::askTakeImage()
{
    qDebug() << "[" << QDateTime::currentDateTime().toString("dd-MM-yyyy_HH.mm.ss")<< "][MAINWINDOW] Ask image";
    m_camera->askPhoto();
}

void MainWindow::cameraInfo(ASI_CAMERA_INFO cameraInfo){

    ui->label_Resolution->setText(QString("Resolution : %1x%2").arg(cameraInfo.MaxWidth).arg(cameraInfo.MaxHeight));
    ui->spinBox_Height->setMaximum(cameraInfo.MaxHeight);
    ui->spinBox_width->setMaximum(cameraInfo.MaxWidth);
}

void MainWindow::controlCap(const AsiCamera::AllControlCAP controlCap, const AsiCamera::ControlIndex index, const int piNumberOfControls)
{

    if (ui->checkBox_Camera_Parameters->isChecked())
    {
        m_index = index;
        m_numberOfControls = piNumberOfControls;

        for (int i = 0 ; i < piNumberOfControls ; i++)
        {

            if (i == index.ind_gain)
            {
                ui->spinBox_gain->setMinimum(controlCap.cc_gain.MinValue);
                ui->spinBox_gain->setMaximum(controlCap.cc_gain.MaxValue);
                ui->spinBox_gain->setValue(controlCap.cc_gain.DefaultValue);

                ui->checkBox_gain->setChecked(controlCap.ac_gain);

            }
            else if (i == index.ind_exposure)
            {
                ui->spinBox_exposure->setMinimum(controlCap.cc_exposure.MinValue);
                ui->spinBox_exposure->setMaximum(controlCap.cc_exposure.MaxValue);
                ui->spinBox_exposure->setValue(controlCap.cc_exposure.DefaultValue);

                ui->checkBox_exposure->setChecked(controlCap.ac_exposure);
            }
            else if (i == index.ind_offset)
            {
                ui->spinBox_offset->setMinimum(controlCap.cc_offset.MinValue);
                ui->spinBox_offset->setMaximum(controlCap.cc_offset.MaxValue);
                ui->spinBox_offset->setValue(controlCap.cc_offset.DefaultValue);

            }
            else if (i == index.ind_bandWidth)
            {
                ui->spinBox_bandWidh->setMinimum(controlCap.cc_bandWidh.MinValue);
                ui->spinBox_bandWidh->setMaximum(controlCap.cc_bandWidh.MaxValue);
                ui->spinBox_bandWidh->setValue(controlCap.cc_bandWidh.DefaultValue);

                ui->checkBox_bandWidh->setChecked(controlCap.ac_bandWidh);
            }
            else if (i == index.ind_temperature)
            {
                ui->lcdNumber_temperature->display(int(controlCap.cc_temperature.DefaultValue));
            }
            else if (i == index.ind_flip)
            {
                ui->spinBox_flip->setMinimum(controlCap.cc_flip.MinValue);
                ui->spinBox_flip->setMaximum(controlCap.cc_flip.MaxValue);
                ui->spinBox_flip->setValue(controlCap.cc_flip.DefaultValue);
            }
            else if (i == index.ind_autoExpMaxGain)
            {
                ui->spinBox_autoExpMaxGain->setMinimum(controlCap.cc_autoExpMaxGain.MinValue);
                ui->spinBox_autoExpMaxGain->setMaximum(controlCap.cc_autoExpMaxGain.MaxValue);
                ui->spinBox_autoExpMaxGain->setValue(controlCap.cc_autoExpMaxGain.DefaultValue);
            }
            else if (i == index.ind_autoExpMaxExpMS)
            {
                ui->spinBox_autoExpMaxExpMS->setMinimum(controlCap.cc_autoExpMaxExpMS.MinValue);
                ui->spinBox_autoExpMaxExpMS->setMaximum(controlCap.cc_autoExpMaxExpMS.MaxValue);
                ui->spinBox_autoExpMaxExpMS->setValue(controlCap.cc_autoExpMaxExpMS.DefaultValue);
            }
            else if (i == index.ind_autoExpTargetBrightness)
            {
                ui->spinBox_autoExpTargetBrightness->setMinimum(controlCap.cc_autoExpTargetBrightness.MinValue);
                ui->spinBox_autoExpTargetBrightness->setMaximum(controlCap.cc_autoExpTargetBrightness.MaxValue);
                ui->spinBox_autoExpTargetBrightness->setValue(controlCap.cc_autoExpTargetBrightness.DefaultValue);
            }
            else if (i == index.ind_harwareBin)
            {
                ui->spinBox_harwareBin->setMinimum(controlCap.cc_harwareBin.MinValue);
                ui->spinBox_harwareBin->setMaximum(controlCap.cc_harwareBin.MaxValue);
                ui->spinBox_harwareBin->setValue(controlCap.cc_harwareBin.DefaultValue);
            }
            else if (i == index.ind_highSpeedMode)
            {
                ui->spinBox_highSpeedMode->setMinimum(controlCap.cc_highSpeedMode.MinValue);
                ui->spinBox_highSpeedMode->setMaximum(controlCap.cc_highSpeedMode.MaxValue);
                ui->spinBox_highSpeedMode->setValue(controlCap.cc_highSpeedMode.DefaultValue);
            }

        }
     }

    qDebug() << "[" << QDateTime::currentDateTime().toString("dd-MM-yyyy_HH.mm.ss") << "][MAINWINDOW] Control capture settings receive";
}

void MainWindow::controlFomat(const int Width, const int Height, const int Bin, const int startX, const int startY, const ASI_IMG_TYPE format){

    if(ui->checkBox_Camera_Parameters->isChecked())
    {
        ui->spinBox_Height->setValue(Height);
        ui->spinBox_width->setValue(Width);
        ui->spinBox_start_PosX->setValue(startX);
        ui->spinBox_start_PosX->setValue(startY);

        if(format == ASI_IMG_RAW8)
            ui->comboBox_format->setCurrentIndex(0);
        if(format == ASI_IMG_RAW16)
            ui->comboBox_format->setCurrentIndex(1);
    }

    qDebug() << "[" << QDateTime::currentDateTime().toString("dd-MM-yyyy_HH.mm.ss") << "][MAINWINDOW] Control format receive";
}

void MainWindow::controlValue(const AsiCamera::ControlValue controlvalue, const AsiCamera::ControlIndex index, const int piNumberOfControls)
{

    if(ui->checkBox_Camera_Parameters->isChecked()){

        for (int i = 0 ; i < piNumberOfControls ; i++)
        {

            if (i == index.ind_gain)
            {
                ui->spinBox_gain->setValue(controlvalue.val_gain);
                ui->checkBox_gain->setChecked(controlvalue.auto_gain);

            }
            else if (i == index.ind_exposure)
            {
                ui->spinBox_exposure->setValue(controlvalue.val_exposure);
                ui->checkBox_exposure->setChecked(controlvalue.auto_exposure);
            }
            else if (i == index.ind_offset)
            {
                ui->spinBox_offset->setValue(controlvalue.val_offset);
            }
            else if (i == index.ind_bandWidth)
            {
                ui->spinBox_bandWidh->setValue(controlvalue.val_bandWidth);
                ui->checkBox_bandWidh->setChecked(controlvalue.auto_bandWidth);
            }
            else if (i == index.ind_temperature)
            {
                ui->lcdNumber_temperature->display(int(controlvalue.val_temperature));
            }
            else if (i == index.ind_flip)
            {
                ui->spinBox_flip->setValue(controlvalue.val_flip);
            }
            else if (i == index.ind_autoExpMaxGain)
            {
                ui->spinBox_autoExpMaxGain->setValue(controlvalue.val_autoExpMaxGain);
            }
            else if (i == index.ind_autoExpMaxExpMS)
            {
                ui->spinBox_autoExpMaxExpMS->setValue(controlvalue.val_autoExpMaxExpMS);
            }
            else if (i == index.ind_autoExpTargetBrightness)
            {
                ui->spinBox_autoExpTargetBrightness->setValue(controlvalue.val_autoExpTargetBrightness);
            }
            else if (i == index.ind_harwareBin)
            {
                ui->spinBox_harwareBin->setValue(controlvalue.val_harwareBin);
            }
            else if (i == index.ind_highSpeedMode)
            {
                ui->spinBox_highSpeedMode->setValue(controlvalue.val_highSpeedMode);
            }

        }

        qDebug() << "[" << QDateTime::currentDateTime().toString("dd-MM-yyyy_HH.mm.ss") << "][MAINWINDOW] Receive actual settings of Camera";

    }

}

void MainWindow::allSettings(){

/*
    if(ui->comboBox_format->currentIndex() == 0)
        m_format = ASI_IMG_RAW8;
    else
        m_format = ASI_IMG_RAW16;

    for (int i = 0 ; i < m_numberOfControls ; i++)
    {

        if (i == m_index.ind_gain)
        {
            m_controlValue.val_gain = ui->spinBox_gain->value();

            if(ui->checkBox_gain->checkState())
                m_controlValue.auto_gain = ASI_TRUE;
            else
                m_controlValue.auto_gain = ASI_FALSE;

        }
        else if (i == m_index.ind_exposure)
        {
            m_controlValue.val_exposure = ui->spinBox_exposure->value();

            if(ui->checkBox_exposure->checkState())
                m_controlValue.auto_exposure = ASI_TRUE;
            else
                m_controlValue.auto_exposure = ASI_FALSE;
        }
        else if (i == m_index.ind_offset)
        {
            m_controlValue.val_offset = ui->spinBox_offset->value();

        }
        else if (i == m_index.ind_bandWidth)
        {
            m_controlValue.val_bandWidth = ui->spinBox_bandWidh->value();

            if(ui->checkBox_bandWidh->checkState())
                m_controlValue.auto_bandWidth = ASI_TRUE;
            else
                m_controlValue.auto_bandWidth = ASI_FALSE;

        }
        else if (i == m_index.ind_temperature)
        {

        }
        else if (i == m_index.ind_flip)
        {
            m_controlValue.val_flip = ui->spinBox_flip->value();
        }
        else if (i == m_index.ind_autoExpMaxGain)
        {
            m_controlValue.val_autoExpMaxGain = ui->spinBox_autoExpMaxGain->value();
        }
        else if (i == m_index.ind_autoExpMaxExpMS)
        {
            m_controlValue.val_autoExpMaxExpMS = ui->spinBox_autoExpMaxExpMS->value();
        }
        else if (i == m_index.ind_autoExpTargetBrightness)
        {
            m_controlValue.val_autoExpTargetBrightness = ui->spinBox_autoExpTargetBrightness->value();
        }
        else if (i == m_index.ind_harwareBin)
        {
            m_controlValue.val_harwareBin = ui->spinBox_harwareBin->value();
        }
        else if (i == m_index.ind_highSpeedMode)
        {
            m_controlValue.val_highSpeedMode = ui->spinBox_highSpeedMode->value();
        }

    }*/

    setSettingsCamera();
    m_cameraControlChange = true;

    if (m_cameraRun)
         emit sigAllSettings(ui->spinBox_width->value(), ui->spinBox_Height->value(), 1, ui->spinBox_start_PosX->value(), ui->spinBox_start_PosY->value(), m_format, m_controlValue);

    qDebug() << "[" << QDateTime::currentDateTime().toString("dd-MM-yyyy_HH.mm.ss") << "][MAINWINDOW] Send Settings Camera";
}

void MainWindow::setSettingsCamera(){

    if(ui->comboBox_format->currentIndex() == 0){
        m_format = ASI_IMG_RAW8;
    }
    else{
        m_format = ASI_IMG_RAW16;
    }

        m_controlValue.val_gain = ui->spinBox_gain->value();

        if(ui->checkBox_gain->checkState()){
            m_controlValue.auto_gain = ASI_TRUE;
        }
        else{
            m_controlValue.auto_gain = ASI_FALSE;
        }

        m_controlValue.val_exposure = ui->spinBox_exposure->value();

        if(ui->checkBox_exposure->checkState()){
            m_controlValue.auto_exposure = ASI_TRUE;
        }
        else{
            m_controlValue.auto_exposure = ASI_FALSE;
        }

        m_controlValue.val_offset = ui->spinBox_offset->value();
        m_controlValue.val_bandWidth = ui->spinBox_bandWidh->value();

        if(ui->checkBox_bandWidh->checkState()){
            m_controlValue.auto_bandWidth = ASI_TRUE;
        }
        else{
            m_controlValue.auto_bandWidth = ASI_FALSE;
        }

        m_controlValue.val_flip = ui->spinBox_flip->value();
        m_controlValue.val_autoExpMaxGain = ui->spinBox_autoExpMaxGain->value();
        m_controlValue.val_autoExpMaxExpMS = ui->spinBox_autoExpMaxExpMS->value();
        m_controlValue.val_autoExpTargetBrightness = ui->spinBox_autoExpTargetBrightness->value();
        m_controlValue.val_harwareBin = ui->spinBox_harwareBin->value();
        m_controlValue.val_highSpeedMode = ui->spinBox_highSpeedMode->value();

        m_width = ui->spinBox_width->value();
        m_height = ui->spinBox_Height->value();
        m_bin = 1;
        m_startX = ui->spinBox_start_PosX->value();
        m_startY = ui->spinBox_start_PosY->value();

        qDebug() << "[" << QDateTime::currentDateTime().toString("dd-MM-yyyy_HH.mm.ss") << "][MAINWINDOW] Set settings camera";
}


void MainWindow::showImage(ASI_IMG_TYPE format, const int width, const int height, const QImage frame)
{

    if (format == ASI_IMG_RAW8)
    {
        m_image = new QImage(width, height, QImage::Format_Grayscale8);
    }
    else if(format == ASI_IMG_RAW16)
    {
        m_image = new QImage(width, height, QImage::Format_Grayscale16);
    }
    else
    {
        QMessageBox::critical(this, "Erreur format d'image", "Le format de l'image n'est pas dans la configuration initial.");
    }

    *m_image = frame;

    m_photo->convertFromImage(*m_image,Qt::AutoColor);
    m_photo->devicePixelRatioFScale();

    m_imageScene->loadImage(ui->checkBox_grid->isChecked(),*m_photo);

    /*m_scene->setMinimumRenderSize(0);
    m_scene->clear();
    m_scene->addPixmap(*m_photo);*/


    ui->graphicsView->setScene(m_imageScene);

    qDebug() << "[" << QDateTime::currentDateTime().toString("dd-MM-yyyy_HH.mm.ss") << "][MAINWINDOW] New Image";

    //m_calibration = true;

    delete m_image;

}

// Measure

void MainWindow::startMeasure()
{

    m_startMeasure = true;
    QString dir = "";
    dir = QFileDialog::getExistingDirectory(this, "Open Directory", "/home", QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

    allSettings();

    ui->checkBox_Camera_Parameters->setCheckable(true);
    ui->checkBox_Camera_Parameters->setEnabled(false);

    buttonMeasureActive();
    motorNobuttonAll();

    qDebug() << "[" << QDateTime::currentDateTime().toString("dd-MM-yyyy_HH.mm.ss") << "][MAINWINDOW] Start Measure";
    emit sendStartMeasure(ui->spinBox_Max->value(), ui->spinBox_Min->value(), ui->spinBox_measure->value(), dir);
}

void MainWindow::endMeasure()
{
    buttonMesureDisactive();
    motorbuttonActivate();
    m_startMeasure = false;
    ui->checkBox_Camera_Parameters->setCheckable(false);
    ui->checkBox_Camera_Parameters->setEnabled(false);
    ui->progressBar->setValue(0);
    emit setHomeMeasure();

    qDebug() << "[" << QDateTime::currentDateTime().toString("dd-MM-yyyy_HH.mm.ss") << "][MAINWINDOW] Stop Measure";
}

void MainWindow::statutMeasure(const int pourcentage){

    qDebug() << "[" << QDateTime::currentDateTime().toString("dd-MM-yyyy_HH.mm.ss") << "][MAINWINDOW] State of mesure : " <<pourcentage ;
    ui->progressBar->setValue(pourcentage);
}

// Calibration

void MainWindow::startCalibration(){
    //m_calibration = true;
    ui->label_CalibrationState->setText("Calibration States : calibration is in progress");

    allSettings();
}

//Button control

void MainWindow::buttonVideoActivate(){

    m_videoRun = true;

    ui->actionConnect_ZWO->setEnabled(false);
    ui->actionDisconect_ZWO->setEnabled(true);
    ui->actionPhoto->setEnabled(false);
    ui->actionVideo->setEnabled(false);
    ui->button_Start_Measure->setEnabled(false);
    ui->spinBox_Height->setEnabled(false);
    ui->spinBox_width->setEnabled(false);
    ui->spinBox_start_PosX->setEnabled(false);
    ui->spinBox_start_PosY->setEnabled(false);
    ui->comboBox_format->setEnabled(false);

    ui->button_calibration->setEnabled(true);

}

void MainWindow::buttonVideoDesactivate(){

    ui->actionConnect_ZWO->setEnabled(true);
    ui->actionDisconect_ZWO->setEnabled(false);
    ui->actionPhoto->setEnabled(false);
    ui->actionVideo->setEnabled(false);
    ui->button_Start_Measure->setEnabled(true);
    ui->button_calibration->setEnabled(false);
    ui->spinBox_Height->setEnabled(true);
    ui->spinBox_width->setEnabled(true);
    ui->spinBox_start_PosX->setEnabled(true);
    ui->spinBox_start_PosY->setEnabled(true);
    ui->comboBox_format->setEnabled(true);

}

void MainWindow::motorbuttonActivate()
{
    ui->actionConnect->setEnabled(false);
    ui->actionDisconnect->setEnabled(true);
    ui->button_Home->setEnabled(true);
    ui->button_Position->setEnabled(true);
    ui->button_Home->setEnabled(true);
    ui->button_Send->setEnabled(true);
    ui->button_Start_Measure->setEnabled(true);
}

void MainWindow::motorbuttonDisactivate()
{
    ui->actionConnect->setEnabled(true);
    ui->actionDisconnect->setEnabled(false);
    ui->button_Home->setEnabled(false);
    ui->button_Position->setEnabled(false);
    ui->button_Home->setEnabled(false);
    ui->button_Send->setEnabled(false);
    ui->button_Start_Measure->setEnabled(false);
}

void MainWindow::motorNobuttonAll()
{
    ui->actionConnect->setEnabled(false);
    ui->actionDisconnect->setEnabled(false);
    ui->button_Home->setEnabled(false);
    ui->button_Position->setEnabled(false);
    ui->button_Home->setEnabled(false);
    ui->button_Send->setEnabled(false);
    ui->button_Start_Measure->setEnabled(false);
}

void MainWindow::buttonCameraActivate(){

        /*ui->spinBox_Height->setEnabled(true);
        ui->spinBox_width->setEnabled(true);
        ui->spinBox_start_PosX->setEnabled(true);
        ui->spinBox_start_PosY->setEnabled(true);
        ui->spinBox_gain->setEnabled(true);
        ui->spinBox_exposure->setEnabled(true);
        ui->spinBox_offset->setEnabled(true);
        ui->spinBox_bandWidh->setEnabled(true);
        ui->lcdNumber_temperature->setEnabled(true);
        ui->spinBox_flip->setEnabled(true);
        ui->spinBox_autoExpMaxGain->setEnabled(true);
        ui->spinBox_autoExpMaxExpMS->setEnabled(true);
        ui->spinBox_autoExpTargetBrightness->setEnabled(true);
        ui->spinBox_harwareBin->setEnabled(true);
        ui->spinBox_highSpeedMode->setEnabled(true);
        ui->Button_apply_camera->setEnabled(true);
        ui->checkBox_gain->setEnabled(true);
        ui->checkBox_bandWidh->setEnabled(true);
        ui->checkBox_exposure->setEnabled(true);
        ui->checkBox_Camera_Parameters->setEnabled(true);*/

        ui->actionConnect_ZWO->setEnabled(false);
        ui->actionDisconect_ZWO->setEnabled(true);
        ui->actionPhoto->setEnabled(true);
        ui->actionVideo->setEnabled(true);

}

void MainWindow::buttonCameraDesactivate(){


        /*ui->spinBox_Height->setEnabled(false);
        ui->spinBox_width->setEnabled(false);
        ui->spinBox_start_PosX->setEnabled(false);
        ui->spinBox_start_PosY->setEnabled(false);
        ui->spinBox_gain->setEnabled(false);
        ui->spinBox_exposure->setEnabled(false);
        ui->spinBox_offset->setEnabled(false);
        ui->spinBox_bandWidh->setEnabled(false);
        ui->lcdNumber_temperature->setEnabled(false);
        ui->spinBox_flip->setEnabled(false);
        ui->spinBox_autoExpMaxGain->setEnabled(false);
        ui->spinBox_autoExpMaxExpMS->setEnabled(false);
        ui->spinBox_autoExpTargetBrightness->setEnabled(false);
        ui->spinBox_harwareBin->setEnabled(false);
        ui->spinBox_highSpeedMode->setEnabled(false);
        ui->Button_apply_camera->setEnabled(false);
        ui->checkBox_gain->setEnabled(false);
        ui->checkBox_bandWidh->setEnabled(false);
        ui->checkBox_exposure->setEnabled(false);
        ui->checkBox_Camera_Parameters->setEnabled(true);*/

        ui->actionConnect_ZWO->setEnabled(true);
        ui->actionDisconect_ZWO->setEnabled(false);
        ui->actionPhoto->setEnabled(false);
        ui->actionVideo->setEnabled(false);

}

void MainWindow::buttonMeasureActive(){

    ui->spinBox_Height->setEnabled(false);
    ui->spinBox_width->setEnabled(false);
    ui->spinBox_start_PosX->setEnabled(false);
    ui->spinBox_start_PosY->setEnabled(false);
    ui->spinBox_gain->setEnabled(false);
    ui->spinBox_exposure->setEnabled(false);
    ui->spinBox_offset->setEnabled(false);
    ui->spinBox_bandWidh->setEnabled(false);
    ui->lcdNumber_temperature->setEnabled(false);
    ui->spinBox_flip->setEnabled(false);
    ui->spinBox_autoExpMaxGain->setEnabled(false);
    ui->spinBox_autoExpMaxExpMS->setEnabled(false);
    ui->spinBox_autoExpTargetBrightness->setEnabled(false);
    ui->spinBox_harwareBin->setEnabled(false);
    ui->spinBox_highSpeedMode->setEnabled(false);
    ui->Button_apply_camera->setEnabled(false);
    ui->checkBox_gain->setEnabled(false);
    ui->checkBox_bandWidh->setEnabled(false);
    ui->checkBox_exposure->setEnabled(false);
    ui->checkBox_Camera_Parameters->setEnabled(true);

    ui->actionConnect_ZWO->setEnabled(false);
    ui->actionDisconect_ZWO->setEnabled(false);
    ui->actionPhoto->setEnabled(false);
    ui->actionVideo->setEnabled(false);

}

void MainWindow::buttonMesureDisactive(){

    ui->spinBox_Height->setEnabled(true);
    ui->spinBox_width->setEnabled(true);
    ui->spinBox_start_PosX->setEnabled(true);
    ui->spinBox_start_PosY->setEnabled(true);
    ui->spinBox_gain->setEnabled(true);
    ui->spinBox_exposure->setEnabled(true);
    ui->spinBox_offset->setEnabled(true);
    ui->spinBox_bandWidh->setEnabled(true);
    ui->lcdNumber_temperature->setEnabled(true);
    ui->spinBox_flip->setEnabled(true);
    ui->spinBox_autoExpMaxGain->setEnabled(true);
    ui->spinBox_autoExpMaxExpMS->setEnabled(true);
    ui->spinBox_autoExpTargetBrightness->setEnabled(true);
    ui->spinBox_harwareBin->setEnabled(true);
    ui->spinBox_highSpeedMode->setEnabled(true);
    ui->Button_apply_camera->setEnabled(true);
    ui->checkBox_gain->setEnabled(true);
    ui->checkBox_bandWidh->setEnabled(true);
    ui->checkBox_exposure->setEnabled(true);
    ui->checkBox_Camera_Parameters->setEnabled(true);

    ui->actionConnect_ZWO->setEnabled(true);
    ui->actionDisconect_ZWO->setEnabled(false);
    ui->actionPhoto->setEnabled(false);
    ui->actionVideo->setEnabled(false);

}
