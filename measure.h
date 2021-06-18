#ifndef MEASURE_H
#define MEASURE_H

#include <QThread>
#include <QObject>
#include <QImage>
#include <QSemaphore>
#include <QMutex>
#include <QFile>
#include <QDir>
#include <QDebug>
#include <QDateTime>
#include "lib/ASICamera2.h"
#include "asicamera.h"

class Measure : public QThread
{

  Q_OBJECT

public:
    Measure();
    ~Measure();

    void run();

    void saveImage();
    void askOpenCamera();
    void askTakePhoto();
    void sendPosition();
    void propretyImage();

public slots :
    void imageReception(ASI_IMG_TYPE format, const int width, const int height, const QImage frame);

    void startMeasure(int max, int min, float step, QString dir);
    void endMeasure();

    void oppenedCameraMeasure();
    void closedCameraMeasure();
    void positionReception(const double positionActu);
    void controlValueMeasure(const AsiCamera::ControlValue controlvalue);

signals :

    void sigSendPositionMeasure(const float position);
    void sigOpenCameraMeasure();
    void sigTakePhotoMeasure();

    void sigSendStateMeasure(const int pourcentage);

    void sigEndMeasure();


private :

    bool m_startMeasure;
    bool m_NewPosition;
    bool m_newImage;

    double m_positionMax;
    double m_positionMin;

    double m_positionActu;

    int m_counterMax;
    int m_counter;

    float m_step;
    bool m_error;

    QString m_mainDir;
    QString m_imageDir;

    QSemaphore *m_semOpenCam;
    QSemaphore *m_semGetPhoto;
    QSemaphore *m_semSendPosition;
    QMutex m_mut;

    QImage *m_image;
    AsiCamera::ControlValue m_controlvalue;

    QFile m_dirImage;
    QDir m_mainFolder;
    QDir m_imageFolder;

};

#endif // MEASURE_H
