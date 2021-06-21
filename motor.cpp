#include "motor.h"

Motor::Motor()
{

}

Motor::~Motor()
{
    delete m_semStack;
    delete m_semWait;
}

void Motor::run(){

    m_motorRun = false;
    m_homeApply = false;
    m_positionApply = false;
    m_errorSerial = false;

    m_semWait = new QSemaphore(0);
    m_semStack = new QSemaphore(0);

    qDebug() << "[" << QDateTime::currentDateTime().toString("dd-MM-yyyy_HH.mm.ss") << "][MOTOR] " << QThread::currentThread();

    while(1){
        if(m_motorRun)
        {
            setHome();

            while(m_motorRun){

               if(m_semStack->tryAcquire(1)){

                   mut.lock();

                   if (m_homeApply){
                       movingHome();
                   }
                   else if (m_positionApply){
                       movingPosition();
                   }

                   mut.unlock();
               }
            }
        }
        else
            QThread::usleep(100);
   }
}

void Motor::initMotor(){

    m_positionActu = 0.0f;
    m_positionApply = 0.0f;

    qDebug() << "[" << QDateTime::currentDateTime().toString("dd-MM-yyyy_HH.mm.ss") << "][MOTOR] Motor Initialisation";

    m_motorRun = true;
    m_errorSerial = false;

}

void Motor::setHome()
{
    m_homeApply = true;
    m_positionActu = 0.0f;
    m_semStack->release(1);
}

void Motor::setPosition(const double position)
{
    m_positionApply = true;
    m_positionAsk = position;
    m_semStack->release(1);
}

void Motor::errorSerial()
{
    m_errorSerial = true;
    m_motorRun = false;

    qDebug() << "[" << QDateTime::currentDateTime().toString("dd-MM-yyyy_HH.mm.ss") << "][MOTOR] Serial error confirm";

    m_semStack->release(1);
}

void Motor::closeSerial()
{
    m_motorRun = false;

    qDebug() << "[" << QDateTime::currentDateTime().toString("dd-MM-yyyy_HH.mm.ss") << "][MOTOR] Serial closed confirm";

    m_semStack->release(1);
}

void Motor::movingPosition()
{


    QByteArray cmd;
    int counterMax(0), counter(0);

    m_semWait->release(1);

    //int positionActue = qRound(m_positionActu*10);
    //int positionAsk = qRound(m_positionAsk*10);

    if (m_positionActu > m_positionAsk){

        cmd.resize(12);
        cmd[0] = Cmd_gcode;
        cmd[1] = Cmd_echelle_nb2;
        cmd[2] = Cmd_echelle_nb1;
        cmd[3] = Cmd_gcode;
        cmd[4] = Cmd_position_nb9;
        cmd[5] = Cmd_position_nb1;
        cmd[6] = Cmd_position_X;
        cmd[7] = Cmd_neg;
        cmd[8] = Cmd_position_nb0;
        cmd[9] = Cmd_point;
        cmd[10] = Cmd_position_nb1;
        cmd[11] = Cmd_end;

        counterMax = qRound(m_positionActu*10)-qRound(m_positionAsk*10);

        for (counter = 1 ; counter <= counterMax ; counter++)
        {
            if (m_semWait->tryAcquire(1))
            {
                qDebug() << "[" << QDateTime::currentDateTime().toString("dd-MM-yyyy_HH.mm.ss") << "][MOTOR] Motor send position :";

                emit sendToCmd(cmd);
                QThread::msleep(300); // Time to travel
                m_positionActu -= 0.1f;
                qRound(m_positionActu);
                emit motorState(true, m_positionActu);
            }
        }

        emit motorState(false, m_positionActu);

    }
    else if (m_positionActu < m_positionAsk){
        cmd.resize(11);
        cmd[0] = Cmd_gcode;
        cmd[1] = Cmd_echelle_nb2;
        cmd[2] = Cmd_echelle_nb1;
        cmd[3] = Cmd_gcode;
        cmd[4] = Cmd_position_nb9;
        cmd[5] = Cmd_position_nb1;
        cmd[6] = Cmd_position_X;
        cmd[7] = Cmd_position_nb0;
        cmd[8] = Cmd_point;
        cmd[9] = Cmd_position_nb1;
        cmd[10] = Cmd_end;

        counterMax = qRound(m_positionAsk*10)-qRound(m_positionActu*10);

        for (counter = 1 ; counter <= counterMax ; counter++)
        {
            if (m_semWait->tryAcquire(1))
            {
                qDebug() << "[" << QDateTime::currentDateTime().toString("dd-MM-yyyy_HH.mm.ss") << "][MOTOR] Motor send position :";

                emit sendToCmd(cmd);
                QThread::msleep(300); // Time to travel
                m_positionActu += 0.1f;
                qRound(m_positionActu);
                emit motorState(true, m_positionActu);
            }
        }

        emit motorState(false, m_positionActu);
    }
    else{

        emit motorState(false, m_positionActu);
    }

    emit endMove(m_positionActu);
    m_positionApply = false;


}

void Motor::movingHome()
{


    emit doHome();

    QByteArray cmd;
        cmd.resize(3);
        cmd[0] = Cmd_start;
        cmd[1] = Cmd_home_H;
        cmd[2] = Cmd_end;
    emit sendToCmd(cmd);

    qDebug() << "[" << QDateTime::currentDateTime().toString("dd-MM-yyyy_HH.mm.ss") << "][MOTOR] Motor send Home position";

    emit motorState(false, m_positionActu);

    m_homeApply = false;


}

void Motor::serialsendMessage()
{
    if (m_positionApply)
    {
    m_semWait->release(1);
    }
    qDebug() << "[" << QDateTime::currentDateTime().toString("dd-MM-yyyy_HH.mm.ss") << "][MOTOR] Motor confirm send message";

}
