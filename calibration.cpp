#include "calibration.h"

Q_DECLARE_METATYPE(AsiCamera::ControlValue);

Calibration::Calibration()
{

}

void Calibration::run()
{
    m_semOpenCam = new QSemaphore;
    m_semGetPhoto = new QSemaphore;
    m_semCalibration = new QSemaphore;

    m_image = new QImage;

    m_error = false;
    m_exposureGood = false;
    m_startCalibration = false;

    m_maxPixel = 0;
    m_minPixel = 0;

    qDebug() << "[" << QDateTime::currentDateTime().toString("dd-MM-yyyy_HH.mm.ss") << "][CALIBRATION] " << QThread::currentThread();
    qRegisterMetaType<AsiCamera::ControlValue>();

    while(1)
    {
        if(m_startCalibration){

            if(!m_exposureGood)
            {
                askOpenCamera();
                m_semOpenCam->acquire(1);

                askTakePhoto();
                m_semGetPhoto->acquire(2);

                calibration();
                m_semCalibration->acquire(1);
            }
            else{
                emit sigEndCalibration();
            }
        }
        else
            QThread::msleep(100);
    }
}

void Calibration::askTakePhoto(){

    emit sigTakePhotoCalibration();
    qDebug() << "[" << QDateTime::currentDateTime().toString("dd-MM-yyyy_HH.mm.ss") << "][[CALIBRATION] Send command to take a photo" ;

}

void Calibration::askOpenCamera(){

    emit sigOpenCameraCalibration();
    qDebug() << "[" << QDateTime::currentDateTime().toString("dd-MM-yyyy_HH.mm.ss") << "][[CALIBRATION] Send command to open camera";
}

void Calibration::calibration(){

    QColor color;
    int r = 0, g = 0, b = 0;
    int pixelValue_act = 0;
    int saturation = 0;
    bool newPhase = m_phase;


    for (int i = 0; i < m_image->width(); i++)
    {
        for (int j = 0; j < m_image->height(); j++)
        {
            color = QColor(m_image->pixel(i,j));
            color.getRgb(&r, &g, &b);
            pixelValue_act = qGray(r,g,b);

            if(pixelValue_act > saturation)
            {
                saturation = pixelValue_act;
                emit sigPixelSaturation(saturation);
            }
        }
    }

    if(saturation >= m_minPixel && saturation <= m_maxPixel){
           m_startCalibration = false;
           m_exposureGood = true;

           qDebug() << "[" << QDateTime::currentDateTime().toString("dd-MM-yyyy_HH.mm.ss") << "][[CALIBRATION] Calibration Ok";
     }

     else if(saturation > m_maxPixel){
           m_calibrationExposure = m_controlvalue.val_exposure/2;

           if(m_calibrationExposure >= 0){
               m_controlvalue.val_exposure = m_calibrationExposure;

               emit sigNewControlValue(m_controlvalue);
               newPhase = true;
           }
           else{
               m_startCalibration = false;
               m_exposureGood = true;
               m_error = true;

               qDebug() << "[" << QDateTime::currentDateTime().toString("dd-MM-yyyy_HH.mm.ss") << "][[CALIBRATION] Calibration error";
           }
     }
     else if (saturation < m_minPixel)
     {
           m_calibrationExposure = m_controlvalue.val_exposure * 1.2;
           m_controlvalue.val_exposure = m_calibrationExposure;

           emit sigNewControlValue(m_controlvalue);
           newPhase = false;
     }

    if (m_phase != newPhase)
        m_numberPhase++;

    m_semCalibration->release(1);

}

void Calibration::imageReception(ASI_IMG_TYPE format, const int width, const int height, const QImage frame){

    if(m_startCalibration){

       // m_mut.lock();
        qDebug() << "[" << QDateTime::currentDateTime().toString("dd-MM-yyyy_HH.mm.ss") << "][[MEASURE] image receive";

        if (format == ASI_IMG_RAW8)
        {
            m_image = new QImage(width, height, QImage::Format_Grayscale8);
        }
        else if(format == ASI_IMG_RAW16)
        {
            m_image = new QImage(width, height, QImage::Format_Grayscale16);
        }

        *m_image = frame;

        m_semGetPhoto->release(1);

       // m_mut.unlock();
    }
}

void Calibration::startCalibration(int max, int min, AsiCamera::ControlValue controlvalue){

    m_startCalibration = true;
    m_calibrationExposure = false;
    m_exposureGood = false;
    m_numberPhase = 0;

    m_maxPixel = max;
    m_minPixel = min;

    m_controlvalue = controlvalue;

    qDebug() << "[" << QDateTime::currentDateTime().toString("dd-MM-yyyy_HH.mm.ss") << "][[CALIBRATION] Start Calibration";
}

void Calibration::endCalibration(){

    m_startCalibration = false;
    m_calibrationExposure = false;
    m_exposureGood = false;

    qDebug() << "[" << QDateTime::currentDateTime().toString("dd-MM-yyyy_HH.mm.ss") << "][[CALIBRATION] End Calibration";

    emit sigEndCalibration();
}

void Calibration::oppenedCameraCalibration()
{
    if(m_startCalibration)
    {
        qDebug() << "[" << QDateTime::currentDateTime().toString("dd-MM-yyyy_HH.mm.ss") << "][[MEASURE] Camera opened";
        m_semOpenCam->release(1);
    }
}

void Calibration::closedCameraCalibration()
{
    if(m_startCalibration)
    {
        qDebug() << "[" << QDateTime::currentDateTime().toString("dd-MM-yyyy_HH.mm.ss") << "][[MEASURE] Camera closed";
        m_semGetPhoto->release(1);
    }
}

void Calibration::controlValueCalibration(AsiCamera::ControlValue controlvalue)
{
    if(m_startCalibration){
    m_controlvalue = controlvalue;
    qDebug() << "[" << QDateTime::currentDateTime().toString("dd-MM-yyyy_HH.mm.ss") << "][[MEASURE] Control value receive";
    }
}

void Calibration::errorCam()
{

}
