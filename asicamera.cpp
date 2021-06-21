#include "asicamera.h"

Q_DECLARE_METATYPE(ASI_CAMERA_INFO);
Q_DECLARE_METATYPE(ASI_IMG_TYPE);
Q_DECLARE_METATYPE(AsiCamera::AllControlCAP);
Q_DECLARE_METATYPE(AsiCamera::ControlIndex);
Q_DECLARE_METATYPE(AsiCamera::ControlValue);

AsiCamera::AsiCamera()
{
    pASICameraInfo = new ASI_CAMERA_INFO;
    m_sem = new QSemaphore(0);
    m_frame = new QImage;

    m_piNumberOfControls = 0;
    m_NBConnectedCamera = 0;
    m_statut = ASI_EXP_IDLE;
    m_modeCamera = ASI_MODE_NORMAL;

    m_askClose = false;
    m_getPhoto = false;
    m_videoStart = false;
    m_askVideo = false;

    m_error = ASI_SUCCESS;
}

AsiCamera::~AsiCamera()
{
    if (m_connected == true)
        closeCamera();

    delete pASICameraInfo;
    delete m_frame;
}

void AsiCamera::run()
{
    //bool m_videoStart = false;
     m_connected = false;

     qRegisterMetaType<ASI_CAMERA_INFO>();
     qRegisterMetaType<ASI_IMG_TYPE>();
     qRegisterMetaType<AsiCamera::AllControlCAP>();
     qRegisterMetaType<AsiCamera::ControlIndex>();
     qRegisterMetaType<AsiCamera::ControlValue>();

    qDebug() << "[" << QDateTime::currentDateTime().toString("dd-MM-yyyy_HH.mm.ss") << "][Camera] " << QThread::currentThread();

    while(1)
    {
        if(m_connected == true)
        {
            if (connectCamera() != ASI_SUCCESS)
                m_connected = false;
            if (initCamera() != ASI_SUCCESS)
                m_connected = false;
            if (cameraMode() != ASI_SUCCESS)
                m_connected = false;


            while(m_connected){
                if(m_sem->tryAcquire(1)) {
                    mut.lock();
                        if(m_getPhoto == true) {
                           getPhoto();
                           closeCamera();
                        }
                        else if(m_askClose == true){
                            closeCamera();
                        }
                        else if(m_askVideo == true){
                            controlVideo();
                        }
                    mut.unlock();
                }
            }               
        }
        else
            QThread::usleep(100);
    }
}

ASI_ERROR_CODE AsiCamera::connectCamera()
{
    m_error = ASI_SUCCESS;

    // Get count of connected cameras

    m_NBConnectedCamera = ASIGetNumOfConnectedCameras();

    if (m_NBConnectedCamera <= 0)
    {
        m_error = ASI_ERROR_INVALID_INDEX;
        emit sigErrorCamera(m_error);
        return m_error;
    }

    // Get cameras' ID and other information like name, resolution, etc. Refreshing devices won’t change
    // this ID

    m_error = ASIGetCameraProperty(pASICameraInfo,0);

    if (m_error != ASI_SUCCESS)
    {
        emit sigErrorCamera(m_error);
        return m_error;
    }

    emit sigInfoCamera(*pASICameraInfo);

    // Open camera

    m_error = ASIOpenCamera(pASICameraInfo->CameraID);

    if (m_error != ASI_SUCCESS)
    {
        emit sigErrorCamera(m_error);
        return m_error;
    }

    qDebug() << "[" << QDateTime::currentDateTime().toString("dd-MM-yyyy_HH.mm.ss") << "][Camera] Connected";

    return m_error;
}

ASI_ERROR_CODE AsiCamera::closeCamera()
{
    // Stop la video de la camera

    if (m_askVideo == true)
    {

        m_videoStart = false;

        m_error = ASIStopVideoCapture(pASICameraInfo->CameraID);

        if (m_error != ASI_SUCCESS)
        {
            emit sigErrorCamera(m_error);
            return m_error;
        }

        m_askVideo = false;
    }

    // Coupe la liaison

    m_error = ASICloseCamera(pASICameraInfo->CameraID);

    if (m_error != ASI_SUCCESS)
    {
        emit sigErrorCamera(m_error);
    }

    m_connected = false;
    m_askClose = false;

    qDebug() << "[" << QDateTime::currentDateTime().toString("dd-MM-yyyy_HH.mm.ss") << "][Camera] Clossed";

    emit sigClosedCamera(true);

    return m_error;
}

ASI_ERROR_CODE AsiCamera::initCamera(){

    m_error = ASI_SUCCESS;
    ASI_CONTROL_CAPS ccActu;

    // Initialize

    m_error = ASIInitCamera(pASICameraInfo->CameraID);

    if (m_error != ASI_SUCCESS)
    {
        emit sigErrorCamera(m_error);
        return m_error;
    }

    // Get count of control type

    m_error = ASIGetNumOfControls(pASICameraInfo->CameraID, &m_piNumberOfControls);

    if (m_error != ASI_SUCCESS)
    {
        emit sigErrorCamera(m_error);
        return m_error;
    }

    // Get capacity of every control type

    for (int i = 0 ; i < m_piNumberOfControls ; i++)
    {
        m_error = ASIGetControlCaps(pASICameraInfo->CameraID, i, &ccActu);

        if (m_error != ASI_SUCCESS)
        {
            emit sigErrorCamera(m_error);
            break;
            return m_error;
        }

        switch (ccActu.ControlType) {
        case ASI_GAIN:
            m_controlCap.cc_gain = ccActu;
            m_controlCap.ac_gain = true;
            m_controlIndex.ind_gain = i;
            break;
        case ASI_EXPOSURE:
            m_controlCap.cc_exposure = ccActu;
            m_controlCap.ac_exposure = true;
            m_controlIndex.ind_exposure = i;
            break;
        case ASI_GAMMA:
            m_controlCap.cc_gamma = ccActu;
            m_controlCap.ac_gamma = true;
            m_controlIndex.ind_gamma = i;
            break;
        case ASI_WB_R:
            m_controlCap.cc_WB_R = ccActu;
            m_controlCap.ac_WB_R = true;
            m_controlIndex.ind_WB_R = i;
            break;
        case ASI_WB_B:
            m_controlCap.cc_WB_B = ccActu;
            m_controlCap.ac_WB_B = true;
            m_controlIndex.ind_WB_B = i;
            break;
        case ASI_OFFSET:
            m_controlCap.cc_offset = ccActu;
            m_controlCap.ac_offset = true;
            m_controlIndex.ind_offset = i;
            break;
        case ASI_BANDWIDTHOVERLOAD:
            m_controlCap.cc_bandWidh = ccActu;
            m_controlCap.ac_bandWidh = true;
            m_controlIndex.ind_bandWidth = i;
            break;
        case ASI_OVERCLOCK:
            m_controlCap.cc_overclock = ccActu;
            m_controlCap.ac_overClock = true;
            m_controlIndex.ind_overClock = i;
            break;
        case ASI_TEMPERATURE:
            m_controlCap.cc_temperature = ccActu;
            m_controlCap.ac_temperature = true;
            m_controlIndex.ind_temperature = i;
            break;
        case ASI_FLIP:
            m_controlCap.cc_flip = ccActu;
            m_controlCap.ac_flip = true;
            m_controlIndex.ind_flip = i;
            break;
        case ASI_AUTO_MAX_GAIN:
            m_controlCap.cc_autoExpMaxGain = ccActu;
            m_controlCap.ac_autoExpMaxGain = true;
            m_controlIndex.ind_autoExpMaxGain = i;
            break;
        case ASI_AUTO_MAX_EXP:
            m_controlCap.cc_autoExpMaxExpMS = ccActu;
            m_controlCap.ac_autoExpMaxExpMS = true;
            m_controlIndex.ind_autoExpMaxExpMS = i;
            break;
        case ASI_AUTO_TARGET_BRIGHTNESS:
            m_controlCap.cc_autoExpTargetBrightness = ccActu;
            m_controlCap.ac_autoExpTargetBrightness = true;
            m_controlIndex.ind_autoExpTargetBrightness = i;
            break;
        case ASI_HARDWARE_BIN:
            m_controlCap.cc_harwareBin = ccActu;
            m_controlCap.ac_harwareBin = true;
            m_controlIndex.ind_harwareBin = i;
            break;
        case ASI_HIGH_SPEED_MODE:
            m_controlCap.cc_highSpeedMode = ccActu;
            m_controlCap.ac_highSpeedMode = true;
            m_controlIndex.ind_highSpeedMode = i;
            break;
        case ASI_COOLER_POWER_PERC:
            m_controlCap.cc_coolerPowerPerc = ccActu;
            m_controlCap.ac_coolerPowerPerc = true;
            m_controlIndex.ind_coolerPowerPerc = i;
            break;
        case ASI_TARGET_TEMP:
            m_controlCap.cc_targetTemp = ccActu;
            m_controlCap.ac_targetTem = true;
            m_controlIndex.ind_targetTemp = i;
            break;
        case ASI_COOLER_ON:
            m_controlCap.cc_coolerOn = ccActu;
            m_controlCap.ac_coolerOn = true;
            m_controlIndex.ind_coolerOn = i;
            break;
        case ASI_MONO_BIN:
            m_controlCap.cc_monoBin = ccActu;
            m_controlCap.ac_monoBin = true;
            m_controlIndex.ind_monoBin = i;
            break;
        case ASI_FAN_ON:
            m_controlCap.cc_fanOn = ccActu;
            m_controlCap.ac_fanOn = true;
            m_controlIndex.ind_fanOn = i;
            break;
        case ASI_PATTERN_ADJUST:
            m_controlCap.cc_paternAjust = ccActu;
            m_controlCap.ac_paternAjust = true;
            m_controlIndex.ind_paternAjust = i;
            break;
        case ASI_ANTI_DEW_HEATER:
            m_controlCap.cc_AntiDewHeater = ccActu;
            m_controlCap.ac_AntiDewHeater = true;
            m_controlIndex.ind_AntiDewHeater = i;
        default:
            break;
        }
    }

    emit sigControlCap(m_controlCap, m_controlIndex, m_piNumberOfControls);

    // Get actual image size and format

    m_error = ASIGetROIFormat(pASICameraInfo->CameraID, &m_piWidth, &m_piHeight, &m_piBin, &m_piformat);

    if (m_error != ASI_SUCCESS)
    {
        emit sigErrorCamera(m_error);
        return m_error;
    }

    // Get actual start position when ROI

    m_error = ASIGetStartPos(pASICameraInfo->CameraID, &m_piStartX, &m_piStartY);

    if (m_error != ASI_SUCCESS)
    {
        emit sigErrorCamera(m_error);
        return m_error;
    }

    qDebug() << "[" << QDateTime::currentDateTime().toString("dd-MM-yyyy_HH.mm.ss") << "][Camera] Initialize";


    emit sigControlFomat(m_piWidth, m_piHeight, m_piBin, m_piStartX, m_piStartY, m_piformat);

    emit sigOpennedCamera(true);

    return m_error;

}

ASI_ERROR_CODE AsiCamera::formatImage()
{
    // Set image size and format

    m_error = ASI_SUCCESS;

    m_error = ASISetROIFormat(pASICameraInfo->CameraID, m_iWidth, m_iHeight, m_iBin, m_iformat);

    if (m_error != ASI_SUCCESS)
    {
        emit sigErrorCamera(m_error);
        return m_error;
    }

    // Set start position when ROI

    m_error = ASISetStartPos(pASICameraInfo->CameraID, m_iStartX, m_iStartY);

    if (m_error != ASI_SUCCESS)
    {
        emit sigErrorCamera(m_error);
        return m_error;
    }

    // Get actual image size and format

    m_error = ASIGetROIFormat(pASICameraInfo->CameraID, &m_piWidth, &m_piHeight, &m_piBin, &m_piformat);

    if (m_error != ASI_SUCCESS)
    {
        emit sigErrorCamera(m_error);
        return m_error;
    }

    // Get actual start position when ROI

    m_error = ASIGetStartPos(pASICameraInfo->CameraID, &m_piStartX, &m_piStartY);

    if (m_error != ASI_SUCCESS)
    {
        emit sigErrorCamera(m_error);
        return m_error;
    }

    qDebug() << "[" << QDateTime::currentDateTime().toString("dd-MM-yyyy_HH.mm.ss") << "][Camera] Format Change";

    emit sigControlFomat(m_piWidth, m_piHeight, m_piBin, m_piStartX, m_piStartY, m_piformat);

    return m_error;
}

ASI_ERROR_CODE AsiCamera::controlValueCamera()
{
    m_error = ASI_SUCCESS;

    // Set a specific control type's value

    for (int i = 0 ; i < m_piNumberOfControls ; i++)
    {

        if (i == m_controlIndex.ind_gain)
        {
            m_error = ASISetControlValue(pASICameraInfo->CameraID, ASI_GAIN, m_icontrolValue.val_gain, m_icontrolValue.auto_gain);

            if (m_error != ASI_SUCCESS)
            {
                emit sigErrorCamera(m_error);
                return m_error;
            }
        }
        else if (i == m_controlIndex.ind_exposure)
        {
            m_error = ASISetControlValue(pASICameraInfo->CameraID, ASI_EXPOSURE, m_icontrolValue.val_exposure, m_icontrolValue.auto_exposure);

            if (m_error != ASI_SUCCESS)
            {
                emit sigErrorCamera(m_error);
                return m_error;
            }
        }
        else if (i == m_controlIndex.ind_gamma)
        {
            m_error = ASISetControlValue(pASICameraInfo->CameraID, ASI_GAMMA, m_icontrolValue.val_gamma, m_icontrolValue.auto_gamma);

            if (m_error != ASI_SUCCESS)
            {
                emit sigErrorCamera(m_error);
                return m_error;
            }
        }
        else if (i == m_controlIndex.ind_WB_R)
        {
            m_error = ASISetControlValue(pASICameraInfo->CameraID, ASI_WB_R, m_icontrolValue.val_WB_R, m_icontrolValue.auto_WB_R);

            if (m_error != ASI_SUCCESS)
            {
                emit sigErrorCamera(m_error);
                return m_error;
            }
        }
        else if (i == m_controlIndex.ind_WB_B)
        {
            m_error = ASISetControlValue(pASICameraInfo->CameraID, ASI_WB_B, m_icontrolValue.val_WB_B, m_icontrolValue.auto_WB_B);

            if (m_error != ASI_SUCCESS)
            {
                emit sigErrorCamera(m_error);
                return m_error;
            }
        }
        else if (i == m_controlIndex.ind_offset)
        {
            m_error = ASISetControlValue(pASICameraInfo->CameraID, ASI_OFFSET, m_icontrolValue.val_offset, m_icontrolValue.auto_offset);

            if (m_error != ASI_SUCCESS)
            {
                emit sigErrorCamera(m_error);
                return m_error;
            }
        }
        else if (i == m_controlIndex.ind_bandWidth)
        {
            m_error = ASISetControlValue(pASICameraInfo->CameraID, ASI_BANDWIDTHOVERLOAD, m_icontrolValue.val_bandWidth, m_icontrolValue.auto_bandWidth);

            if (m_error != ASI_SUCCESS)
            {
                emit sigErrorCamera(m_error);
                return m_error;
            }
        }
        else if (i == m_controlIndex.ind_overClock)
        {
            m_error = ASISetControlValue(pASICameraInfo->CameraID, ASI_OVERCLOCK, m_icontrolValue.val_overClock, m_icontrolValue.auto_overClock);

            if (m_error != ASI_SUCCESS)
            {
                emit sigErrorCamera(m_error);
                return m_error;
            }
        }
        else if (i == m_controlIndex.ind_temperature)
        {
            m_error = ASISetControlValue(pASICameraInfo->CameraID, ASI_TEMPERATURE, m_icontrolValue.val_temperature, m_icontrolValue.auto_temperature);

            if (m_error != ASI_SUCCESS)
            {
                emit sigErrorCamera(m_error);
                return m_error;
            }
        }
        else if (i == m_controlIndex.ind_flip)
        {
            m_error = ASISetControlValue(pASICameraInfo->CameraID, ASI_FLIP, m_icontrolValue.val_flip, m_icontrolValue.auto_flip);

            if (m_error != ASI_SUCCESS)
            {
                emit sigErrorCamera(m_error);
                return m_error;
            }
        }
        else if (i == m_controlIndex.ind_autoExpMaxGain)
        {
            m_error = ASISetControlValue(pASICameraInfo->CameraID, ASI_AUTO_MAX_GAIN, m_icontrolValue.val_autoExpMaxGain, m_icontrolValue.auto_autoExpMaxGain);

            if (m_error != ASI_SUCCESS)
            {
                emit sigErrorCamera(m_error);
                return m_error;
            }
        }
        else if (i == m_controlIndex.ind_autoExpMaxExpMS)
        {
            m_error = ASISetControlValue(pASICameraInfo->CameraID, ASI_AUTO_MAX_EXP, m_icontrolValue.val_autoExpMaxExpMS, m_icontrolValue.auto_autoExpMaxExpMS);

            if (m_error != ASI_SUCCESS)
            {
                emit sigErrorCamera(m_error);
                return m_error;
            }
        }
        else if (i == m_controlIndex.ind_autoExpTargetBrightness)
        {
            m_error = ASISetControlValue(pASICameraInfo->CameraID, ASI_AUTO_TARGET_BRIGHTNESS, m_icontrolValue.val_autoExpTargetBrightness, m_icontrolValue.auto_autoExpTargetBrightness);

            if (m_error != ASI_SUCCESS)
            {
                emit sigErrorCamera(m_error);
                return m_error;
            }
        }
        else if (i == m_controlIndex.ind_harwareBin)
        {
            m_error = ASISetControlValue(pASICameraInfo->CameraID, ASI_HARDWARE_BIN, m_icontrolValue.val_harwareBin, m_icontrolValue.auto_harwareBin);

            if (m_error != ASI_SUCCESS)
            {
                emit sigErrorCamera(m_error);
                return m_error;
            }
        }
        else if (i == m_controlIndex.ind_highSpeedMode)
        {
            m_error = ASISetControlValue(pASICameraInfo->CameraID, ASI_HIGH_SPEED_MODE, m_icontrolValue.val_highSpeedMode, m_icontrolValue.auto_highSpeedMode);

            if (m_error != ASI_SUCCESS)
            {
                emit sigErrorCamera(m_error);
                return m_error;
            }
        }
        else if (i == m_controlIndex.ind_coolerPowerPerc)
        {
            m_error = ASISetControlValue(pASICameraInfo->CameraID, ASI_COOLER_POWER_PERC, m_icontrolValue.val_coolerPowerPerc, m_icontrolValue.auto_coolerPowerPerc);

            if (m_error != ASI_SUCCESS)
            {
                emit sigErrorCamera(m_error);
                return m_error;
            }
        }
        else if (i == m_controlIndex.ind_targetTemp)
        {
            m_error = ASISetControlValue(pASICameraInfo->CameraID, ASI_TARGET_TEMP, m_icontrolValue.val_targetTemp, m_icontrolValue.auto_targetTemp);

            if (m_error != ASI_SUCCESS)
            {
                emit sigErrorCamera(m_error);
                return m_error;
            }
        }
        else if (i == m_controlIndex.ind_coolerOn)
        {
            m_error = ASISetControlValue(pASICameraInfo->CameraID, ASI_COOLER_ON, m_icontrolValue.val_coolerOn, m_icontrolValue.auto_coolerOn);

            if (m_error != ASI_SUCCESS)
            {
                emit sigErrorCamera(m_error);
                return m_error;
            }
        }
        else if (i == m_controlIndex.ind_monoBin)
        {
            m_error = ASISetControlValue(pASICameraInfo->CameraID, ASI_MONO_BIN, m_icontrolValue.val_monoBin, m_icontrolValue.auto_monoBin);

            if (m_error != ASI_SUCCESS)
            {
                emit sigErrorCamera(m_error);
                return m_error;
            }
        }
        else if (i == m_controlIndex.ind_fanOn)
        {
            m_error = ASISetControlValue(pASICameraInfo->CameraID, ASI_FAN_ON, m_icontrolValue.val_fanOn, m_icontrolValue.auto_fanOn);

            if (m_error != ASI_SUCCESS)
            {
                emit sigErrorCamera(m_error);
                return m_error;
            }
        }
        else if (i == m_controlIndex.ind_paternAjust)
        {
            m_error = ASISetControlValue(pASICameraInfo->CameraID, ASI_PATTERN_ADJUST, m_icontrolValue.val_paternAjust, m_icontrolValue.auto_paternAjust);

            if (m_error != ASI_SUCCESS)
            {
                emit sigErrorCamera(m_error);
                return m_error;
            }
        }
        else if (i == m_controlIndex.ind_AntiDewHeater)
        {
            m_error = ASISetControlValue(pASICameraInfo->CameraID, ASI_ANTI_DEW_HEATER, m_icontrolValue.val_AntiDewHeater, m_icontrolValue.auto_AntiDewHeater);

            if (m_error != ASI_SUCCESS)
            {
                emit sigErrorCamera(m_error);
                return m_error;
            }
        }

    }

    // get a specific control type's value as currently set

    for (int i = 0 ; i < m_piNumberOfControls ; i++)
    {

        if (i == m_controlIndex.ind_gain)
        {
            m_error = ASIGetControlValue(pASICameraInfo->CameraID, ASI_GAIN, &m_picontrolValue.val_gain, &m_picontrolValue.auto_gain);

            if (m_error != ASI_SUCCESS)
            {
                emit sigErrorCamera(m_error);
                return m_error;
            }
        }
        else if (i == m_controlIndex.ind_exposure)
        {
            m_error = ASIGetControlValue(pASICameraInfo->CameraID, ASI_EXPOSURE, &m_picontrolValue.val_exposure, &m_picontrolValue.auto_exposure);

            if (m_error != ASI_SUCCESS)
            {
                emit sigErrorCamera(m_error);
                return m_error;
            }
        }
        else if (i == m_controlIndex.ind_gamma)
        {
            m_error = ASIGetControlValue(pASICameraInfo->CameraID, ASI_GAMMA, &m_picontrolValue.val_gamma, &m_picontrolValue.auto_gamma);

            if (m_error != ASI_SUCCESS)
            {
                emit sigErrorCamera(m_error);
                return m_error;
            }
        }
        else if (i == m_controlIndex.ind_WB_R)
        {
            m_error = ASIGetControlValue(pASICameraInfo->CameraID, ASI_WB_R, &m_picontrolValue.val_WB_R, &m_picontrolValue.auto_WB_R);

            if (m_error != ASI_SUCCESS)
            {
                emit sigErrorCamera(m_error);
                return m_error;
            }
        }
        else if (i == m_controlIndex.ind_WB_B)
        {
            m_error = ASIGetControlValue(pASICameraInfo->CameraID, ASI_WB_B, &m_picontrolValue.val_WB_B, &m_picontrolValue.auto_WB_B);

            if (m_error != ASI_SUCCESS)
            {
                emit sigErrorCamera(m_error);
                return m_error;
            }
        }
        else if (i == m_controlIndex.ind_offset)
        {
            m_error = ASIGetControlValue(pASICameraInfo->CameraID, ASI_OFFSET, &m_picontrolValue.val_offset, &m_picontrolValue.auto_offset);

            if (m_error != ASI_SUCCESS)
            {
                emit sigErrorCamera(m_error);
                return m_error;
            }
        }
        else if (i == m_controlIndex.ind_bandWidth)
        {
            m_error = ASIGetControlValue(pASICameraInfo->CameraID, ASI_BANDWIDTHOVERLOAD, &m_picontrolValue.val_bandWidth, &m_picontrolValue.auto_bandWidth);

            if (m_error != ASI_SUCCESS)
            {
                emit sigErrorCamera(m_error);
                return m_error;
            }
        }
        else if (i == m_controlIndex.ind_overClock)
        {
            m_error = ASIGetControlValue(pASICameraInfo->CameraID, ASI_OVERCLOCK, &m_picontrolValue.val_overClock, &m_picontrolValue.auto_overClock);

            if (m_error != ASI_SUCCESS)
            {
                emit sigErrorCamera(m_error);
                return m_error;
            }
        }
        else if (i == m_controlIndex.ind_temperature)
        {
            m_error = ASIGetControlValue(pASICameraInfo->CameraID, ASI_TEMPERATURE, &m_picontrolValue.val_temperature, &m_picontrolValue.auto_temperature);

            if (m_error != ASI_SUCCESS)
            {
                emit sigErrorCamera(m_error);
                return m_error;
            }
        }
        else if (i == m_controlIndex.ind_flip)
        {
            m_error = ASIGetControlValue(pASICameraInfo->CameraID, ASI_FLIP, &m_picontrolValue.val_flip, &m_picontrolValue.auto_flip);

            if (m_error != ASI_SUCCESS)
            {
                emit sigErrorCamera(m_error);
                return m_error;
            }
        }
        else if (i == m_controlIndex.ind_autoExpMaxGain)
        {
            m_error = ASIGetControlValue(pASICameraInfo->CameraID, ASI_AUTO_MAX_GAIN, &m_picontrolValue.val_autoExpMaxGain, &m_picontrolValue.auto_autoExpMaxGain);

            if (m_error != ASI_SUCCESS)
            {
                emit sigErrorCamera(m_error);
                return m_error;
            }
        }
        else if (i == m_controlIndex.ind_autoExpMaxExpMS)
        {
            m_error = ASIGetControlValue(pASICameraInfo->CameraID, ASI_AUTO_MAX_EXP, &m_picontrolValue.val_autoExpMaxExpMS, &m_picontrolValue.auto_autoExpMaxExpMS);

            if (m_error != ASI_SUCCESS)
            {
                emit sigErrorCamera(m_error);
                return m_error;
            }
        }
        else if (i == m_controlIndex.ind_autoExpTargetBrightness)
        {
            m_error = ASIGetControlValue(pASICameraInfo->CameraID, ASI_AUTO_TARGET_BRIGHTNESS, &m_picontrolValue.val_autoExpTargetBrightness, &m_picontrolValue.auto_autoExpTargetBrightness);

            if (m_error != ASI_SUCCESS)
            {
                emit sigErrorCamera(m_error);
                return m_error;
            }
        }
        else if (i == m_controlIndex.ind_harwareBin)
        {
            m_error = ASIGetControlValue(pASICameraInfo->CameraID, ASI_HARDWARE_BIN, &m_picontrolValue.val_harwareBin, &m_picontrolValue.auto_harwareBin);

            if (m_error != ASI_SUCCESS)
            {
                emit sigErrorCamera(m_error);
                return m_error;
            }
        }
        else if (i == m_controlIndex.ind_highSpeedMode)
        {
            m_error = ASIGetControlValue(pASICameraInfo->CameraID, ASI_HIGH_SPEED_MODE, &m_picontrolValue.val_highSpeedMode, &m_picontrolValue.auto_highSpeedMode);

            if (m_error != ASI_SUCCESS)
            {
                emit sigErrorCamera(m_error);
                return m_error;
            }
        }
        else if (i == m_controlIndex.ind_coolerPowerPerc)
        {
            m_error = ASIGetControlValue(pASICameraInfo->CameraID, ASI_COOLER_POWER_PERC, &m_picontrolValue.val_coolerPowerPerc, &m_picontrolValue.auto_coolerPowerPerc);

            if (m_error != ASI_SUCCESS)
            {
                emit sigErrorCamera(m_error);
                return m_error;
            }
        }
        else if (i == m_controlIndex.ind_targetTemp)
        {
            m_error = ASIGetControlValue(pASICameraInfo->CameraID, ASI_TARGET_TEMP, &m_picontrolValue.val_targetTemp, &m_picontrolValue.auto_targetTemp);

            if (m_error != ASI_SUCCESS)
            {
                emit sigErrorCamera(m_error);
                return m_error;
            }
        }
        else if (i == m_controlIndex.ind_coolerOn)
        {
            m_error = ASIGetControlValue(pASICameraInfo->CameraID, ASI_COOLER_ON, &m_picontrolValue.val_coolerOn, &m_picontrolValue.auto_coolerOn);

            if (m_error != ASI_SUCCESS)
            {
                emit sigErrorCamera(m_error);
                return m_error;
            }
        }
        else if (i == m_controlIndex.ind_monoBin)
        {
            m_error = ASIGetControlValue(pASICameraInfo->CameraID, ASI_MONO_BIN, &m_picontrolValue.val_monoBin, &m_picontrolValue.auto_monoBin);

            if (m_error != ASI_SUCCESS)
            {
                emit sigErrorCamera(m_error);
                return m_error;
            }
        }
        else if (i == m_controlIndex.ind_fanOn)
        {
            m_error = ASIGetControlValue(pASICameraInfo->CameraID, ASI_FAN_ON, &m_picontrolValue.val_fanOn, &m_picontrolValue.auto_fanOn);

            if (m_error != ASI_SUCCESS)
            {
                emit sigErrorCamera(m_error);
                return m_error;
            }
        }
        else if (i == m_controlIndex.ind_paternAjust)
        {
            m_error = ASIGetControlValue(pASICameraInfo->CameraID, ASI_PATTERN_ADJUST, &m_picontrolValue.val_paternAjust, &m_picontrolValue.auto_paternAjust);

            if (m_error != ASI_SUCCESS)
            {
                emit sigErrorCamera(m_error);
                return m_error;
            }
        }
        else if (i == m_controlIndex.ind_AntiDewHeater)
        {
            m_error = ASIGetControlValue(pASICameraInfo->CameraID, ASI_ANTI_DEW_HEATER, &m_picontrolValue.val_AntiDewHeater, &m_picontrolValue.auto_AntiDewHeater);

            if (m_error != ASI_SUCCESS)
            {
                emit sigErrorCamera(m_error);
                return m_error;
            }
        }

    }

    emit sigcontrolValue(m_picontrolValue, m_controlIndex, m_piNumberOfControls);

    qDebug() << "[" << QDateTime::currentDateTime().toString("dd-MM-yyyy_HH.mm.ss") << "][Camera] Control Value ok";

    return m_error;
}

ASI_ERROR_CODE AsiCamera::getPhoto(){


    if (m_piformat == ASI_IMG_RAW8)
    {
        *m_frame = {m_piWidth, m_piHeight, QImage::Format_Grayscale8};
    }
    else if (m_piformat == ASI_IMG_RAW16)
    {
        *m_frame = {m_piWidth, m_piHeight, QImage::Format_Grayscale16};
    }
    else
    {
        emit sigErrorCamera(m_error);
        return ASI_ERROR_INVALID_IMGTYPE;
    }

    m_error = ASIStartExposure(pASICameraInfo->CameraID,ASI_FALSE);

    if (m_error != ASI_SUCCESS)
    {
        emit sigErrorCamera(m_error);
        return m_error;
    }


    while(m_statut != ASI_EXP_SUCCESS && m_statut != ASI_EXP_FAILED)
    {
        m_error = ASIGetExpStatus(pASICameraInfo->CameraID, &m_statut);

        if (m_error != ASI_SUCCESS)
        {
            emit sigErrorCamera(m_error);
            return m_error;
        }
    }

    m_error = ASIGetDataAfterExp(pASICameraInfo->CameraID, m_frame->bits(), m_frame->sizeInBytes());

    if (m_error != ASI_SUCCESS)
    {
        emit sigErrorCamera(m_error);
        return m_error;
    }

    sendControlValue();
    emit sigImageReception(m_piformat, m_piWidth, m_piHeight, *m_frame);

    m_getPhoto = false;


    qDebug() << "[" << QDateTime::currentDateTime().toString("dd-MM-yyyy_HH.mm.ss") << "][Camera] Take Image";

    m_error = ASIStopExposure(pASICameraInfo->CameraID);

    if (m_error != ASI_SUCCESS)
    {
        emit sigErrorCamera(m_error);
        return m_error;
    }

    m_statut = ASI_EXP_IDLE;

    return m_error;
}

ASI_ERROR_CODE AsiCamera::sendControlValue()
{
    // get a specific control type's value as currently set

    for (int i = 0 ; i < m_piNumberOfControls ; i++)
    {

        if (i == m_controlIndex.ind_gain)
        {
            m_error = ASIGetControlValue(pASICameraInfo->CameraID, ASI_GAIN, &m_picontrolValue.val_gain, &m_picontrolValue.auto_gain);

            if (m_error != ASI_SUCCESS)
            {
                emit sigErrorCamera(m_error);
                return m_error;
            }
        }
        else if (i == m_controlIndex.ind_exposure)
        {
            m_error = ASIGetControlValue(pASICameraInfo->CameraID, ASI_EXPOSURE, &m_picontrolValue.val_exposure, &m_picontrolValue.auto_exposure);

            if (m_error != ASI_SUCCESS)
            {
                emit sigErrorCamera(m_error);
                return m_error;
            }
        }
        else if (i == m_controlIndex.ind_gamma)
        {
            m_error = ASIGetControlValue(pASICameraInfo->CameraID, ASI_GAMMA, &m_picontrolValue.val_gamma, &m_picontrolValue.auto_gamma);

            if (m_error != ASI_SUCCESS)
            {
                emit sigErrorCamera(m_error);
                return m_error;
            }
        }
        else if (i == m_controlIndex.ind_WB_R)
        {
            m_error = ASIGetControlValue(pASICameraInfo->CameraID, ASI_WB_R, &m_picontrolValue.val_WB_R, &m_picontrolValue.auto_WB_R);

            if (m_error != ASI_SUCCESS)
            {
                emit sigErrorCamera(m_error);
                return m_error;
            }
        }
        else if (i == m_controlIndex.ind_WB_B)
        {
            m_error = ASIGetControlValue(pASICameraInfo->CameraID, ASI_WB_B, &m_picontrolValue.val_WB_B, &m_picontrolValue.auto_WB_B);

            if (m_error != ASI_SUCCESS)
            {
                emit sigErrorCamera(m_error);
                return m_error;
            }
        }
        else if (i == m_controlIndex.ind_offset)
        {
            m_error = ASIGetControlValue(pASICameraInfo->CameraID, ASI_OFFSET, &m_picontrolValue.val_offset, &m_picontrolValue.auto_offset);

            if (m_error != ASI_SUCCESS)
            {
                emit sigErrorCamera(m_error);
                return m_error;
            }
        }
        else if (i == m_controlIndex.ind_bandWidth)
        {
            m_error = ASIGetControlValue(pASICameraInfo->CameraID, ASI_BANDWIDTHOVERLOAD, &m_picontrolValue.val_bandWidth, &m_picontrolValue.auto_bandWidth);

            if (m_error != ASI_SUCCESS)
            {
                emit sigErrorCamera(m_error);
                return m_error;
            }
        }
        else if (i == m_controlIndex.ind_overClock)
        {
            m_error = ASIGetControlValue(pASICameraInfo->CameraID, ASI_OVERCLOCK, &m_picontrolValue.val_overClock, &m_picontrolValue.auto_overClock);

            if (m_error != ASI_SUCCESS)
            {
                emit sigErrorCamera(m_error);
                return m_error;
            }
        }
        else if (i == m_controlIndex.ind_temperature)
        {
            m_error = ASIGetControlValue(pASICameraInfo->CameraID, ASI_TEMPERATURE, &m_picontrolValue.val_temperature, &m_picontrolValue.auto_temperature);

            if (m_error != ASI_SUCCESS)
            {
                emit sigErrorCamera(m_error);
                return m_error;
            }
        }
        else if (i == m_controlIndex.ind_flip)
        {
            m_error = ASIGetControlValue(pASICameraInfo->CameraID, ASI_FLIP, &m_picontrolValue.val_flip, &m_picontrolValue.auto_flip);

            if (m_error != ASI_SUCCESS)
            {
                emit sigErrorCamera(m_error);
                return m_error;
            }
        }
        else if (i == m_controlIndex.ind_autoExpMaxGain)
        {
            m_error = ASIGetControlValue(pASICameraInfo->CameraID, ASI_AUTO_MAX_GAIN, &m_picontrolValue.val_autoExpMaxGain, &m_picontrolValue.auto_autoExpMaxGain);

            if (m_error != ASI_SUCCESS)
            {
                emit sigErrorCamera(m_error);
                return m_error;
            }
        }
        else if (i == m_controlIndex.ind_autoExpMaxExpMS)
        {
            m_error = ASIGetControlValue(pASICameraInfo->CameraID, ASI_AUTO_MAX_EXP, &m_picontrolValue.val_autoExpMaxExpMS, &m_picontrolValue.auto_autoExpMaxExpMS);

            if (m_error != ASI_SUCCESS)
            {
                emit sigErrorCamera(m_error);
                return m_error;
            }
        }
        else if (i == m_controlIndex.ind_autoExpTargetBrightness)
        {
            m_error = ASIGetControlValue(pASICameraInfo->CameraID, ASI_AUTO_TARGET_BRIGHTNESS, &m_picontrolValue.val_autoExpTargetBrightness, &m_picontrolValue.auto_autoExpTargetBrightness);

            if (m_error != ASI_SUCCESS)
            {
                emit sigErrorCamera(m_error);
                return m_error;
            }
        }
        else if (i == m_controlIndex.ind_harwareBin)
        {
            m_error = ASIGetControlValue(pASICameraInfo->CameraID, ASI_HARDWARE_BIN, &m_picontrolValue.val_harwareBin, &m_picontrolValue.auto_harwareBin);

            if (m_error != ASI_SUCCESS)
            {
                emit sigErrorCamera(m_error);
                return m_error;
            }
        }
        else if (i == m_controlIndex.ind_highSpeedMode)
        {
            m_error = ASIGetControlValue(pASICameraInfo->CameraID, ASI_HIGH_SPEED_MODE, &m_picontrolValue.val_highSpeedMode, &m_picontrolValue.auto_highSpeedMode);

            if (m_error != ASI_SUCCESS)
            {
                emit sigErrorCamera(m_error);
                return m_error;
            }
        }
        else if (i == m_controlIndex.ind_coolerPowerPerc)
        {
            m_error = ASIGetControlValue(pASICameraInfo->CameraID, ASI_COOLER_POWER_PERC, &m_picontrolValue.val_coolerPowerPerc, &m_picontrolValue.auto_coolerPowerPerc);

            if (m_error != ASI_SUCCESS)
            {
                emit sigErrorCamera(m_error);
                return m_error;
            }
        }
        else if (i == m_controlIndex.ind_targetTemp)
        {
            m_error = ASIGetControlValue(pASICameraInfo->CameraID, ASI_TARGET_TEMP, &m_picontrolValue.val_targetTemp, &m_picontrolValue.auto_targetTemp);

            if (m_error != ASI_SUCCESS)
            {
                emit sigErrorCamera(m_error);
                return m_error;
            }
        }
        else if (i == m_controlIndex.ind_coolerOn)
        {
            m_error = ASIGetControlValue(pASICameraInfo->CameraID, ASI_COOLER_ON, &m_picontrolValue.val_coolerOn, &m_picontrolValue.auto_coolerOn);

            if (m_error != ASI_SUCCESS)
            {
                emit sigErrorCamera(m_error);
                return m_error;
            }
        }
        else if (i == m_controlIndex.ind_monoBin)
        {
            m_error = ASIGetControlValue(pASICameraInfo->CameraID, ASI_MONO_BIN, &m_picontrolValue.val_monoBin, &m_picontrolValue.auto_monoBin);

            if (m_error != ASI_SUCCESS)
            {
                emit sigErrorCamera(m_error);
                return m_error;
            }
        }
        else if (i == m_controlIndex.ind_fanOn)
        {
            m_error = ASIGetControlValue(pASICameraInfo->CameraID, ASI_FAN_ON, &m_picontrolValue.val_fanOn, &m_picontrolValue.auto_fanOn);

            if (m_error != ASI_SUCCESS)
            {
                emit sigErrorCamera(m_error);
                return m_error;
            }
        }
        else if (i == m_controlIndex.ind_paternAjust)
        {
            m_error = ASIGetControlValue(pASICameraInfo->CameraID, ASI_PATTERN_ADJUST, &m_picontrolValue.val_paternAjust, &m_picontrolValue.auto_paternAjust);

            if (m_error != ASI_SUCCESS)
            {
                emit sigErrorCamera(m_error);
                return m_error;
            }
        }
        else if (i == m_controlIndex.ind_AntiDewHeater)
        {
            m_error = ASIGetControlValue(pASICameraInfo->CameraID, ASI_ANTI_DEW_HEATER, &m_picontrolValue.val_AntiDewHeater, &m_picontrolValue.auto_AntiDewHeater);

            if (m_error != ASI_SUCCESS)
            {
                emit sigErrorCamera(m_error);
                return m_error;
            }
        }

    }

    emit sigcontrolValue(m_picontrolValue, m_controlIndex, m_piNumberOfControls);

    emit sigControlValueAlone(m_picontrolValue);

    qDebug() << "[" << QDateTime::currentDateTime().toString("dd-MM-yyyy_HH.mm.ss") << "][Camera] Send Control Value ";

    // Get actual image size and format

    m_error = ASIGetROIFormat(pASICameraInfo->CameraID, &m_piWidth, &m_piHeight, &m_piBin, &m_piformat);

    if (m_error != ASI_SUCCESS)
    {
        emit sigErrorCamera(m_error);
        return m_error;
    }

    // Get actual start position when ROI

    m_error = ASIGetStartPos(pASICameraInfo->CameraID, &m_piStartX, &m_piStartY);

    if (m_error != ASI_SUCCESS)
    {
        emit sigErrorCamera(m_error);
        return m_error;
    }

    qDebug() << "[" << QDateTime::currentDateTime().toString("dd-MM-yyyy_HH.mm.ss") << "][Camera] Send Format";

    emit sigControlFomat(m_piWidth, m_piHeight, m_piBin, m_piStartX, m_piStartY, m_piformat);

    return m_error;
}

ASI_ERROR_CODE AsiCamera::cameraMode()
{
    m_error = ASISetCameraMode(pASICameraInfo->CameraID, ASI_MODE_NORMAL);

    if (m_error != ASI_SUCCESS)
    {
        emit sigErrorCamera(m_error);
        return m_error;
    }

    m_error = ASIDisableDarkSubtract(pASICameraInfo->CameraID);

    if (m_error != ASI_SUCCESS)
    {
        emit sigErrorCamera(m_error);
        return m_error;
    }

    qDebug() << "[" << QDateTime::currentDateTime().toString("dd-MM-yyyy_HH.mm.ss") << "][Camera] Camera Mod";

    return m_error;
}

void AsiCamera::askConnect()
{
    m_connected = true;
    qDebug() << "[" << QDateTime::currentDateTime().toString("dd-MM-yyyy_HH.mm.ss") << "][Camera] Ask Connect";
}

void AsiCamera::askClose()
{
    m_askClose = true;

    if (m_askVideo == true)
         m_videoStart = false;

    m_sem->release(1);
    qDebug() << "[" << QDateTime::currentDateTime().toString("dd-MM-yyyy_HH.mm.ss") << "][Camera] Ask Close";

}

void AsiCamera::askPhoto()
{
    m_getPhoto = true;
    m_sem->release(1);
    qDebug() << "[" << QDateTime::currentDateTime().toString("dd-MM-yyyy_HH.mm.ss") << "][Camera] Ask photo";

}

void AsiCamera::allSettings(const int width, const int Height, const int Bin, const int startX, const int startY, const ASI_IMG_TYPE format, const ControlValue controlValue)
{
    m_iWidth = width;
    m_iHeight = Height;
    m_iBin = Bin;
    m_iStartX = startX;
    m_iStartY = startY;
    m_iformat = format;
    m_icontrolValue = controlValue;

    formatImage();
    controlValueCamera();

}

//Mode Video

ASI_ERROR_CODE AsiCamera::askVideo(){

    m_error = ASIStartVideoCapture(pASICameraInfo->CameraID);

    if (m_error != ASI_SUCCESS)
    {
        emit sigErrorCamera(m_error);
        return m_error;
    }

    m_videoStart = true;
    m_askVideo = true;
    m_sem->release(1);
    qDebug() << "[" << QDateTime::currentDateTime().toString("dd-MM-yyyy_HH.mm.ss") << "][Camera] Ask video";

    return m_error;
}

ASI_ERROR_CODE AsiCamera::controlVideo()
{
    if (m_piformat == ASI_IMG_RAW8)
    {
        *m_frame = {m_piWidth, m_piHeight, QImage::Format_Grayscale8};
    }
    else if (m_piformat == ASI_IMG_RAW16)
    {
        *m_frame = {m_piWidth, m_piHeight, QImage::Format_Grayscale16};
    }
    else
    {
        emit sigErrorCamera(m_error);
        return ASI_ERROR_INVALID_IMGTYPE;
    }

    while(m_videoStart == true){

        m_error = ASIGetVideoData(pASICameraInfo->CameraID,m_frame->bits(), m_frame->sizeInBytes(), -1);

        if(m_error == ASI_SUCCESS) {
            sendControlValue();
            emit sigImageReception(m_piformat, m_piWidth, m_piHeight, *m_frame);

            qDebug() << "[" << QDateTime::currentDateTime().toString("dd-MM-yyyy_HH.mm.ss") << "][Camera] Video Image";

        }
        else
        {
           emit sigErrorCamera(m_error);
           return m_error;
        }

        QThread::usleep(10);
    }


    return m_error;
}
