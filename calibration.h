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

    void run();

    void askOpenCamera();
    void askTakePhoto();
    void calibration();

public slots :
    void imageReception(ASI_IMG_TYPE format, const int width, const int height, const QImage frame);

    void startCalibration(int max, int min, AsiCamera::ControlValue controlvalue);
    void endCalibration();

    void oppenedCameraCalibration();
    void closedCameraCalibration();

    void controlValueCalibration(AsiCamera::ControlValue controlvalue);

signals :
    void sigOpenCameraMeasure();
    void sigTakePhotoMeasure();
    void sigEndCalibration();

    void sigPixelSaturation(int saturation);
    void sigNewControlValue(AsiCamera::ControlValue controlvalue);

private:

    QImage *m_image;
    AsiCamera::ControlValue m_controlvalue;

    QSemaphore *m_semOpenCam;
    QSemaphore *m_semGetPhoto;
    QSemaphore *m_semCalibration;

    int m_maxPixel;
    int m_minPixel;

    bool m_error;
    bool m_startCalibration;
    bool m_exposureGood;
    bool m_phase;
    int m_numberPhase;
    long m_calibrationExposure;

};

#endif // CALIBRATION_H
