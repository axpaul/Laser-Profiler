#ifndef CALIBRATION_H
#define CALIBRATION_H

#include <QObject>
#include <QWidget>
#include <QThread>
#include <QObject>
#include <QImage>
#include <QSemaphore>
#include <QMutex>
#include <QDebug>
#include <QDateTime>

#include "lib/ASICamera2.h"
#include "asicamera.h"

class Calibration : public QThread
{
    Q_OBJECT
public:
    Calibration();

    void askOpenCamera();
    void askTakePhoto();
    void calibration(QImage frame);

public slots :
    void imageReception(ASI_IMG_TYPE format, const int width, const int height, const QImage frame);

    void startCalibration(int max, int min, float step, QString dir);
    void endCalibration();

    void oppenedCameraCalibration();
    void closedCameraCalibration();

signals :
    void sigOpenCameraMeasure();
    void sigTakePhotoMeasure();

private:

    QImage *m_image;
    AsiCamera::ControlValue m_controlvalue;

    QSemaphore *m_semOpenCam;
    QSemaphore *m_semGetPhoto;

    bool m_error;

    bool m_calibration;
    long m_calibrationExposure;

};

#endif // CALIBRATION_H
