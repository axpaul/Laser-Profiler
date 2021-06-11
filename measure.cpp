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

    qDebug() << "[" << QDateTime::currentDateTime().toString("dd-MM-yyyy_HH.mm.ss") << "][MEASURE] " << QThread::currentThread();
    qRegisterMetaType<AsiCamera::ControlValue>();

    m_startMeasure = false;

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

               //int pourcentage = (m_counter/m_counterMax)*100;
               //emit sigSendStateMeasure(pourcentage);
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
}

void Measure::askOpenCamera()
{
    emit sigOpenCameraMeasure();

     qDebug() << "[" << QDateTime::currentDateTime().toString("dd-MM-yyyy_HH.mm.ss") << "][[MEASURE] Send a new move";
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
    //QString ImagesCalibrates = "/Images_to_calibrate";

    m_mainFolder = QDir(dir);
    m_mainFolder.mkdir("Images");
    //m_mainFolder.mkdir("Images_to_calibrate");
    m_imageFolder = QDir(Images.prepend(dir));
    //m_dossierImagesCalibrate = new QDir(ImagesCalibrates.prepend(dir));

    m_startMeasure = true;

    qDebug() << "[" << QDateTime::currentDateTime().toString("dd-MM-yyyy_HH.mm.ss") << "][[MEASURE] Start Measure";

}

void Measure::imageReception(ASI_IMG_TYPE format, const int width, const int height, const QImage frame)
{
    if(m_startMeasure){

        m_mut.lock();
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

        m_mut.unlock();
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
    m_controlvalue = controlvalue;
    qDebug() << "[" << QDateTime::currentDateTime().toString("dd-MM-yyyy_HH.mm.ss") << "][[MEASURE] Control value receive";
}

/*void Measure::propertyImage(QString name)
{
    m_proprieteImage = new QFile(name);

    QString stringPosition;

     //stringPosition = QString("Position=%1\n"
     //                "PositionMax=%2\n"
     //                "PositionMin=%3\n").arg((double)*m_position,0,'g',4).arg(*m_positionMax,0,'g',4).arg(*m_positionMin,0,'g',4);

    if (!m_proprieteImage->open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QMessageBox::warning(this, "Enregistrement des propriétés images ","Enregistrement des propriétés images impossible");
        return;
    }

    //m_proprity->prepend(stringPosition);

    QTextStream out(m_proprieteImage);
    out << *m_proprity;

    m_proprieteImage->close();

    delete m_proprieteImage;

}
*/
