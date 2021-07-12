#include "calibration.h"

Calibration::Calibration()
{

}

void Calibration::calibration(QImage frame){

    if(m_calibration == true)
    {
        //m_mut.lock();

        QColor color;
        int r = 0, g = 0, b = 0;
        int pixelValue_act = 0;
        int saturation = 0;
        //int max = ui->spinBox_max_calibration->value();
        //int min = ui->spinBox_min_calibration->value();

        for (int i = 0; i < frame.width(); i++)
        {
               for (int j = 0; j < frame.height(); j++)
               {
                   color = QColor(frame.pixel(i,j));
                   color.getRgb(&r, &g, &b);
                   pixelValue_act = qGray(r,g,b);

                   if(pixelValue_act > saturation)
                   {
                       saturation = pixelValue_act;
                   //    ui->lcdNumber_pixel->display(saturation);
                   }
               }
        }


        /*if(saturation >= min && saturation <= max){
            m_calibration = false;
        }
        else if(saturation > max)
        {
            m_calibrationExposure = m_controlValue.val_exposure/2;
        }
        else if (saturation < min)
        {
            m_calibrationExposure = m_controlValue.val_exposure * 1.33;
        }

        if(m_calibration)
        {
            allSettings();
        }
        else
            ui->label_CalibrationState->setText("Calibration States : calibration terminate");

        //m_mut.unlock();*/
    }

}

void Calibration::imageReception(ASI_IMG_TYPE format, const int width, const int height, const QImage frame){

}

void Calibration::startCalibration(int max, int min, float step, QString dir){

}

void Calibration::endCalibration(){

}

void Calibration::oppenedCameraCalibration(){

}

void Calibration::closedCameraCalibration(){

}

void sigOpenCameraMeasure(){

}
void sigTakePhotoMeasure(){

}
