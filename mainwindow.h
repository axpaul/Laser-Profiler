#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSerialPort>
#include <QMessageBox>
#include <QLabel>
#include <QtDebug>
#include <QWidget>
#include <QPixmap>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QFile>
#include <QFileDialog>

#include "console.h"
#include "serialport.h"
#include "settingsdialog.h"
#include "motor.h"
#include "asicamera.h"
#include "measure.h"
#include "imagescene.h"

#define VERSION_SERIAL 1.2f

QT_BEGIN_NAMESPACE

namespace Ui { class MainWindow; }

QT_END_NAMESPACE

class SerialPort;
class Console;
class Motor;
class AsiCamera;
class Measure;
class ImageScene;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    SerialPort::Settings getSerialInfo();
    QString getSerialError();    

public slots :

    // Serial

    void handleErrorShow(QString error);
    void settingShow();
    void setSerialSettings();

    void cmdToSend();

    // Motor

    void opennedSerial(SerialPort::Settings p);
    void closedSerial();

    void openSerialPort();
    void closeSerialPort();

    void showStateMotor(const bool state, const double position);
    void applyHome();
    void applyPosition();

    // Camera

    void errorCamera(ASI_ERROR_CODE error);

    void openCamera();
    void closeCamera();
    void askTakeImage();

    void opennedCamera(bool state);
    void closedCamera(bool state);
    void imageRecept();

    void cameraInfo(const ASI_CAMERA_INFO cameraInfo);

    void controlCap(const AsiCamera::AllControlCAP controlCap, const AsiCamera::ControlIndex index, const int piNumberOfControls);
    void controlFomat(const int iWidth, const int Height, const int Bin, const int startX, const int startY, const ASI_IMG_TYPE format);
    void controlValue(const AsiCamera::ControlValue controlvalue, const AsiCamera::ControlIndex index, const int piNumberOfControls);
    void allSettings();
    void setSettingsCamera();

    void showImage(ASI_IMG_TYPE format, const int width, const int height, const QImage frame);

    void buttonVideoActivate();

    // Measure

    void startMeasure();
    void endMeasure();
    void statutMeasure(const int pourcentage);

private slots :
    void about();


signals:

    void setSerialSettingsSig(SerialPort::Settings);
    void serialOppened(SerialPort::Settings p);
    void serialClosed();
    void sendCommandSerial(QByteArray data);
    void sendPosition(const float position);

    void sigAllSettings(const int iWidth, const int Height, const int Bin, const int startX, const int startY, const ASI_IMG_TYPE format, const AsiCamera::ControlValue controlValue);

       // Measure

    void sendStartMeasure(int max, int min, float step, QString dir);
    void setHomeMeasure();

private:
    void initActionsConnections();
    void initActionsConnectionsPrio();

    void motorbuttonActivate();
    void motorbuttonDisactivate();
    void motorNobuttonAll();

    void buttonCameraActivate();
    void buttonCameraDesactivate();

    void buttonVideoDesactivate();

    void buttonMeasureActive();
    void buttonMesureDisactive();

    void showStatusMessage(const QString &stringConnection);

     Ui::MainWindow *ui;

     QObject *m_parent;

     QLabel *m_status = nullptr;
     QString *m_connection;
     QString *m_versionSW;

     SettingsDialog *m_settings = nullptr;

     SerialPort *m_serial;
     bool m_serialRun;
     bool m_videoRun;

     Console *console;

     Motor *m_motor;

     QMessageBox *m_infoHome;

     // Attribut Camera

     AsiCamera *m_camera;
     bool m_cameraRun;

     AsiCamera::ControlIndex m_index;
     int m_numberOfControls;


     int m_width;
     int m_height;
     int m_bin;
     int m_startX;
     int m_startY;
     ASI_IMG_TYPE m_format;
     AsiCamera::ControlValue m_controlValue;
     bool m_cameraControlChange;

     QImage *m_image;
     QPixmap *m_photo;
     QGraphicsScene *m_scene;

     ImageScene *m_imageScene;

     Measure *m_measure;
     bool m_startMeasure;

};
#endif // MAINWINDOW_H
