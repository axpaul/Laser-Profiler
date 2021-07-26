#include "measure.h"

Q_DECLARE_METATYPE(AsiCamera::ControlValue);

Measure::Measure()
{

}

Measure::~Measure()
{

}

void Measure::run()
{
    m_semOpenCam = new QSemaphore;
    m_semGetPhoto = new QSemaphore;
    m_semSendPosition = new QSemaphore;

    m_image = new QImage;

    m_error = false;
    m_firstImage = false;

    qDebug() << "[" << QDateTime::currentDateTime().toString("dd-MM-yyyy_HH.mm.ss") << "][MEASURE] " << QThread::currentThread();
    qRegisterMetaType<AsiCamera::ControlValue>();

    m_startMeasure = false;

    double pourcentage = 0;

    while(1)
    {
        m_counter = 0;
        m_counterMax = 0;
        m_positionActu = 0;

        if(m_startMeasure){

            m_counterMax = (qRound(m_positionMax*10)-qRound(m_positionMin*10))/(qRound(m_step*10));

            //Wait moving start position

            emit sigSendPositionMeasure(m_positionMin);
            m_semSendPosition->acquire(1);

            //take a measure

            for(m_counter = 1 ; m_counter <= m_counterMax ; m_counter++)
            {
                askOpenCamera();
                m_semOpenCam->acquire(1);

                askTakePhoto();
                m_semGetPhoto->acquire(2);

                sendPosition();
                m_semSendPosition->acquire(1);

               pourcentage = qRound((double(m_counter)/double(m_counterMax))*10)*10;
               emit sigSendStateMeasure(int(pourcentage));
            }

            askOpenCamera();
            m_semOpenCam->acquire(1);

            askTakePhoto();
            m_semGetPhoto->acquire(2);

            endMeasure();

        }
        else
            QThread::msleep(100);
    }
}

void Measure::askTakePhoto()
{
    emit sigTakePhotoMeasure();
    qDebug() << "[" << QDateTime::currentDateTime().toString("dd-MM-yyyy_HH.mm.ss") << "][[CALIBRATION] Send a take photo";
}

void Measure::askOpenCamera()
{
    emit sigOpenCameraMeasure();

     qDebug() << "[" << QDateTime::currentDateTime().toString("dd-MM-yyyy_HH.mm.ss") << "][[MEASURE] Send a open camera";
}

void Measure::sendPosition(){

    emit sigSendPositionMeasure(((qRound(m_step*m_counter*10))+qRound(m_positionMin*10))/10.0);

    qDebug() << "[" << QDateTime::currentDateTime().toString("dd-MM-yyyy_HH.mm.ss") << "][[MEASURE] Send a new move";
}

void Measure::startMeasure(int max, int min, float step, QString dir){

    m_positionMax = max;
    m_positionMin = min;
    m_step = step;
    m_mainDir = dir;

    QString Images = "/Images";
    QString ImagesCalibrates = "/Images_to_calibrate";
    m_imageDir = Images.prepend(dir);
    m_imageCalibrateDir = ImagesCalibrates.prepend(dir);


    m_mainFolder = QDir(dir);
    m_mainFolder.mkdir("Images");
    m_mainFolder.mkdir("Images_to_calibrate");

    //m_imageFolder = QDir(Images.prepend(dir));
    //m_dossierImagesCalibrate = new QDir(ImagesCalibrates.prepend(dir));

    m_startMeasure = true;
    m_firstImage = true;

    qDebug() << "[" << QDateTime::currentDateTime().toString("dd-MM-yyyy_HH.mm.ss") << "][[MEASURE] Start Measure";
}

void Measure::imageReception(ASI_IMG_TYPE format, const int width, const int height, const QImage frame)
{

    if(m_startMeasure){

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

        propretyImage();
        m_semGetPhoto->release(1);

       // m_mut.unlock();
    }

}

void Measure::oppenedCameraMeasure()
{
    if(m_startMeasure) 
    {
        qDebug() << "[" << QDateTime::currentDateTime().toString("dd-MM-yyyy_HH.mm.ss") << "][[MEASURE] Camera opened";
        m_semOpenCam->release(1);
    }

}

void Measure::closedCameraMeasure()
{
    if(m_startMeasure)
    {
        qDebug() << "[" << QDateTime::currentDateTime().toString("dd-MM-yyyy_HH.mm.ss") << "][[MEASURE] Camera closed";
        m_semGetPhoto->release(1);
    }

}

void Measure::positionReception(const double position)
{

    if(m_startMeasure)
    {
        m_positionActu = position;
        qDebug() << "[" << QDateTime::currentDateTime().toString("dd-MM-yyyy_HH.mm.ss") << "][[MEASURE] Move ending";
        m_semSendPosition->release(1);
    }
}

void Measure::endMeasure()
{
    m_startMeasure = false;

    qDebug() << "[" << QDateTime::currentDateTime().toString("dd-MM-yyyy_HH.mm.ss") << "][[MEASURE] End Measure";

    emit sigEndMeasure();
}

void Measure::controlValueMeasure(AsiCamera::ControlValue controlvalue)
{
    if(m_startMeasure){
    m_controlvalue = controlvalue;
    qDebug() << "[" << QDateTime::currentDateTime().toString("dd-MM-yyyy_HH.mm.ss") << "][[MEASURE] Control value receive";
    }
}

void Measure::propretyImage()
{
    QString setProprityImage;

    char cpt[5];
    QString stringCounter;

    sprintf(cpt, "%04d", (m_counter));
    stringCounter = cpt;

    QString nameImage = QString("/capture_#%1_%2us.png").arg(stringCounter).arg(m_controlvalue.val_exposure);
    QString nameFile = QString("/capture_#%1_%2us_CameraSettings.txt").arg(stringCounter).arg(m_controlvalue.val_exposure);

    setProprityImage = QString("Position : %1 \n"
             "Gain : %2 \n"
             "Exposure : %3 \n"
             "Gamma : %4 \n"
             "Offset : %5 \n"
             "BandWidth : %6 \n"
             "Temperature : %7 \n"
             "Flip : %8 \n"
             "AutoExpMaxGain : %9 \n"
             "AutoExpMaxExpMS : %10 \n"
             "AutoExpTargetBrightness : %11 \n"
             "HarwareBin : %12 \n"
             "HighSpeedMode : %13 \n")
            .arg(m_positionActu)
            .arg(m_controlvalue.val_gain)
            .arg(m_controlvalue.val_exposure)
            .arg(m_controlvalue.val_gamma)
            .arg(m_controlvalue.val_offset)
            .arg(m_controlvalue.val_bandWidth)
            .arg(m_controlvalue.val_temperature)
            .arg(m_controlvalue.val_flip)
            .arg(m_controlvalue.val_autoExpMaxGain)
            .arg(m_controlvalue.val_autoExpTargetBrightness)
            .arg(m_controlvalue.val_harwareBin)
            .arg(m_controlvalue.auto_highSpeedMode);


    nameImage.prepend(m_imageDir);
    nameFile.prepend(m_imageDir);

    QFile fileImage(nameFile);

    if (!fileImage.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        m_error = true;
        //return;
    }

    if(!m_image->save(nameImage,"PNG"))
    {
        m_error = true;
    }


    QTextStream out(&fileImage);
    out << setProprityImage;

    fileImage.close();

    // Save in other dir only first image

    if (m_firstImage)
    {
        m_firstImage = false;

        QString nameImageOld = QString("/capture_#%1_%2us_old.png").arg(stringCounter).arg(m_controlvalue.val_exposure);
        QString nameImageCalibrate = QString("/capture_#%1_%2us.png").arg(stringCounter).arg(m_controlvalue.val_exposure);

        nameImageOld.prepend(m_imageCalibrateDir);
        nameImageCalibrate.prepend(m_imageCalibrateDir);

        // Old file

        if(!m_image->save(nameImageOld,"PNG"))
        {
            m_error = true;
        }

        //Calibrate file


        if(!m_image->save(nameImageCalibrate,"PNG"))
        {
            m_error = true;
        }

    }

    delete m_image;
}

void Measure::errorCam(){

}
