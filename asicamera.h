#ifndef ASICAMERA_H
#define ASICAMERA_H

#include <QObject>
#include <QImage>
#include <QTimer>
#include <QSemaphore>
#include <QStack>
#include <QThread>
#include <QMutex>
#include <QDebug>
#include <QThread>
#include <QDateTime>
#include "lib/ASICamera2.h"

class AsiCamera : public QThread
{
    Q_OBJECT
public:
    typedef struct AllControlCAP{
        ASI_CONTROL_CAPS cc_gain; //gain
        bool ac_gain = false;
        ASI_CONTROL_CAPS cc_exposure; //exposure time (microsecond)
        bool ac_exposure = false;
        ASI_CONTROL_CAPS cc_offset; // brightness
        bool ac_offset = false;
        ASI_CONTROL_CAPS cc_bandWidh; //The total data transfer rate percentage
        bool ac_bandWidh = false;
        ASI_CONTROL_CAPS cc_flip; //image flip
        bool ac_flip = false;
        ASI_CONTROL_CAPS cc_autoExpMaxGain; //maximum gain when auto adjust
        bool ac_autoExpMaxGain = false;
        ASI_CONTROL_CAPS cc_autoExpMaxExpMS; //maximum exposure time when auto adjust，unit is micro seconds
        bool ac_autoExpMaxExpMS = false;
        ASI_CONTROL_CAPS cc_autoExpTargetBrightness; //target brightness when auto adjust
        bool ac_autoExpTargetBrightness = false;
        ASI_CONTROL_CAPS cc_harwareBin; //hardware binning of pixels
        bool ac_harwareBin = false;
        ASI_CONTROL_CAPS cc_highSpeedMode; //high speed mode
        bool ac_highSpeedMode = false;
        ASI_CONTROL_CAPS cc_temperature; // sensor temperature，10 times the actual temperature
        bool ac_temperature = false;

        ASI_CONTROL_CAPS cc_gamma; //gamma with range 1 to 100 (nominally 50)
        bool ac_gamma = false;
        ASI_CONTROL_CAPS cc_WB_R; //red component of white balance
        bool ac_WB_R = false;
        ASI_CONTROL_CAPS cc_WB_B; // blue component of white balance
        bool ac_WB_B = false;
        ASI_CONTROL_CAPS cc_overclock; //over clock
        bool ac_overClock = false;
        ASI_CONTROL_CAPS cc_coolerPowerPerc; //cooler power percent(only cool camera)
        bool ac_coolerPowerPerc = false;
        ASI_CONTROL_CAPS cc_targetTemp; //sensor's target temperature(only cool camera)，don't multiply by 10
        bool ac_targetTem = false;
        ASI_CONTROL_CAPS cc_coolerOn; //open cooler (only cool camera)
        bool ac_coolerOn = false;
        ASI_CONTROL_CAPS cc_monoBin; //lead to a smaller grid at software bin mode for color camera
        bool ac_monoBin = false;
        ASI_CONTROL_CAPS cc_fanOn; //only cooled camera has fan
        bool ac_fanOn = false;
        ASI_CONTROL_CAPS cc_paternAjust; //currently only supported by 1600 mono camera
        bool ac_paternAjust = false;
        ASI_CONTROL_CAPS cc_AntiDewHeater;
        bool ac_AntiDewHeater = false;
    }AllControlCAP;

    typedef struct ControlIndex{
        int ind_gain = -1;
        int ind_exposure = -1;
        int ind_gamma = -1;
        int ind_WB_R = -1;
        int ind_WB_B = -1;
        int ind_offset = -1;
        int ind_bandWidth = -1;
        int ind_overClock = -1;
        int ind_temperature = -1;
        int ind_flip = -1;
        int ind_autoExpMaxGain = -1;
        int ind_autoExpMaxExpMS = -1;
        int ind_autoExpTargetBrightness = -1;
        int ind_harwareBin = -1;
        int ind_highSpeedMode = -1;
        int ind_coolerPowerPerc = -1;
        int ind_targetTemp = -1;
        int ind_coolerOn = -1;
        int ind_monoBin = -1;
        int ind_fanOn = -1;
        int ind_paternAjust = -1;
        int ind_AntiDewHeater =-1;
    } ControlIndex;

    typedef struct ControlValue{
        long val_gain = 0;
        ASI_BOOL auto_gain = ASI_FALSE;

        long val_exposure = 100;
        ASI_BOOL auto_exposure = ASI_FALSE;

        long val_gamma = 0;
        ASI_BOOL auto_gamma = ASI_FALSE;

        long val_WB_R = 0;
        ASI_BOOL auto_WB_R = ASI_FALSE;

        long val_WB_B = 0;
        ASI_BOOL auto_WB_B = ASI_FALSE;

        long val_offset = 0;
        ASI_BOOL auto_offset = ASI_FALSE;

        long val_bandWidth = 0;
        ASI_BOOL auto_bandWidth = ASI_FALSE;

        long val_overClock = 0;
        ASI_BOOL auto_overClock = ASI_FALSE;

        long val_temperature = 0;
        ASI_BOOL auto_temperature = ASI_FALSE;

        long val_flip = 0;
        ASI_BOOL auto_flip = ASI_FALSE;

        long val_autoExpMaxGain = 255;
        ASI_BOOL auto_autoExpMaxGain = ASI_FALSE;

        long val_autoExpMaxExpMS = 27976;
        ASI_BOOL auto_autoExpMaxExpMS = ASI_FALSE;

        long val_autoExpTargetBrightness = 100;
        ASI_BOOL auto_autoExpTargetBrightness = ASI_FALSE;

        long val_harwareBin = 0;
        ASI_BOOL auto_harwareBin = ASI_FALSE;

        long val_highSpeedMode = 0;
        ASI_BOOL auto_highSpeedMode = ASI_FALSE;

        long val_coolerPowerPerc = 0;
        ASI_BOOL auto_coolerPowerPerc = ASI_FALSE;

        long val_targetTemp = 0;
        ASI_BOOL auto_targetTemp = ASI_FALSE;

        long val_coolerOn = 0;
        ASI_BOOL auto_coolerOn = ASI_FALSE;

        long val_monoBin = 0;
        ASI_BOOL auto_monoBin = ASI_FALSE;

        long val_fanOn = 0;
        ASI_BOOL auto_fanOn = ASI_FALSE;

        long val_paternAjust = 0;
        ASI_BOOL auto_paternAjust = ASI_FALSE;

        long val_AntiDewHeater =0;
        ASI_BOOL auto_AntiDewHeater = ASI_FALSE;
    } ControlValue;

    AsiCamera();
    ~AsiCamera();

    void run() override;
    ASI_ERROR_CODE sendControlValue();

public slots:

    void askConnect();
    void askClose();
    void askPhoto();
    //void askVideo();

    void allSettings(const int iWidth, const int Height, const int Bin, const int startX, const int startY, const ASI_IMG_TYPE format, const ControlValue controlValue);

signals :

    void sigErrorCamera(ASI_ERROR_CODE error);
    void sigImageReception(ASI_IMG_TYPE format, const int width, const int height, QImage frame);
    void sigInfoCamera(const ASI_CAMERA_INFO cameraInfo);
    void sigControlCap(const AsiCamera::AllControlCAP controlCap, const AsiCamera::ControlIndex index, const int piNumberOfControls);
    void sigControlFomat(const int iWidth, const int Height, const int Bin, const int startX, const int startY, const ASI_IMG_TYPE format);
    void sigcontrolValue(const AsiCamera::ControlValue controlvalue, const AsiCamera::ControlIndex index, const int piNumberOfControls);
    void sigControlValueAlone(const AsiCamera::ControlValue controlvalue);


    void sigClosedCamera(bool state);
    void sigOpennedCamera(bool state);

private:

    ASI_ERROR_CODE formatImage();
    ASI_ERROR_CODE controlValueCamera();

    ASI_ERROR_CODE initCamera();
    ASI_ERROR_CODE cameraMode();
    ASI_ERROR_CODE getPhoto();
    ASI_ERROR_CODE connectCamera();
    ASI_ERROR_CODE closeCamera();

    ASI_ERROR_CODE m_error;
    int m_NBConnectedCamera;
    int m_piNumberOfControls;

    bool m_videoStart;
    bool m_connected;
    bool m_getPhoto;
    bool m_askClose;

    ControlIndex m_controlIndex;
    AllControlCAP m_controlCap;
    ASI_CAMERA_MODE m_modeCamera;

    // Actual value in camera
    int m_piWidth;
    int m_piHeight;
    int m_piBin;
    int m_piStartX;
    int m_piStartY;
    ASI_IMG_TYPE m_piformat;
    ControlValue m_picontrolValue;

    // value ask for camera
    int m_iWidth = 3096;
    int m_iHeight = 2080;
    int m_iBin = 1;
    int m_iStartX = 0;
    int m_iStartY = 0;
    ASI_IMG_TYPE m_iformat = ASI_IMG_RAW8;
    ControlValue m_icontrolValue;

    QImage *m_frame;
    ASI_EXPOSURE_STATUS m_statut;

    ASI_CAMERA_INFO *pASICameraInfo;
    QSemaphore *m_sem;
    QMutex mut;

};

#endif // ASICAMERA_H
