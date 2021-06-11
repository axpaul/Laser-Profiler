#ifndef MOTOR_H
#define MOTOR_H

#define Cmd_end 0x0d // \r in text
#define Cmd_start 0x24 // & in text
#define Cmd_home_H 0x48 // H in text
#define Cmd_gcode 0x47 // G in text
#define Cmd_echelle_nb2 0x32 // 2 in text
#define Cmd_echelle_nb1 0x31 // 1 in text
#define Cmd_position_X 0x58 // X in text
#define Cmd_position_nb9 0x39 // 9 in text
#define Cmd_position_nb0 0x30 // 0 in text
#define Cmd_position_nb1 0x31 // 1 in text
#define Cmd_neg 0x2d // - in text
#define Cmd_point 0x2e // . in text

#include <QThread>
#include <QObject>
#include <QWidget>
#include <QtDebug>
#include <QDateTime>
#include <QSemaphore>
#include <QMutex>


class Motor : public QThread
{
    Q_OBJECT
public:
    Motor();
    ~Motor();

    void run();
    void movingPosition();
    void movingHome();

public slots:

    void initMotor();
    void closeSerial();

    void setHome();
    void serialsendMessage();

    void setPosition(const double position);
    void errorSerial();

signals:

    void sendToCmd(QByteArray cmd);
    void motorState(const bool state, const double position);
    void endMove(const double position);
    void doHome();

private:

    float m_positionActu;
    float m_positionAsk;

    bool m_motorRun;
    bool m_errorSerial;
    bool m_homeApply;
    bool m_positionApply;

    QSemaphore *m_semStack;
    QSemaphore *m_semWait;
    QMutex mut;

};

#endif // MOTOR_H
