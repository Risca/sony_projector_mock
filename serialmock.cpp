#include "serialmock.h"

#include <QDebug>

namespace {

#define START_CODE ((char)0xA9)
#define ON_CODE    ((char)0x2E)
#define OFF_CODE   ((char)0x2F)
#define END_CODE   ((char)0x9A)

const QByteArray BuildStatusCmd()
{
    const char cmd[] = {START_CODE, 0x01, 0x02, 0x01, 0x00, 0x00, 0x03, END_CODE};
    return QByteArray(cmd, 8);
}

const QByteArray BuildPowerCmd(bool on)
{
    const char cmd[] = {START_CODE, 0x17, (on ? ON_CODE : OFF_CODE), 0x00, 0x00, 0x00, 0x3F, END_CODE };
    return QByteArray(cmd, 8);
}

const QByteArray BuildStatusRespons(int state)
{
    char response[] = { START_CODE, 0x01, 0x02, 0x02, 0x00, (char)state, (char)(0x03 | state), END_CODE };
    return QByteArray(response, 8);
}

}

SerialMock::SerialMock(QObject *parent) :
    QObject(parent),
    m_SerialPort("ttyUSB1"),
    m_State(STATE_STANDBY)
{
    m_StatusCmd = BuildStatusCmd();
    m_PowerOnCmd = BuildPowerCmd(true);
    m_PowerOffCmd = BuildPowerCmd(false);

    m_ResetTimer.setInterval(3000);
    m_ResetTimer.setSingleShot(true);
    connect(&m_ResetTimer, SIGNAL(timeout()), this, SLOT(clearCommandBuffer()));

    m_StateTransitionTimer.setInterval(1000);
    m_StateTransitionTimer.setSingleShot(false);
    connect(&m_StateTransitionTimer, SIGNAL(timeout()), this, SLOT(transitionState()));

    m_SerialPort.setBaudRate(QSerialPort::Baud38400);
    m_SerialPort.setFlowControl(QSerialPort::NoFlowControl);
    m_SerialPort.setParity(QSerialPort::EvenParity);
    connect(&m_SerialPort, SIGNAL(readyRead()), this, SLOT(readBytes()));
    if (m_SerialPort.open(QSerialPort::ReadWrite)) {
        qDebug() << "Listening on" << m_SerialPort.portName();
    }
}

void SerialMock::clearCommandBuffer()
{
    qDebug() << "Resetting input buffer:" << m_Cmd;
    m_Cmd.clear();
}

void SerialMock::transitionState()
{
    switch (m_State) {
    case STATE_STANDBY:
        m_State = STATE_START_UP;
        break;
    case STATE_START_UP:
        m_State = STATE_STARTUP_LAMP;
        break;
    case STATE_STARTUP_LAMP:
        m_State = STATE_POWER_ON;
        m_StateTransitionTimer.stop();
        break;
    case STATE_POWER_ON:
        m_State = STATE_COOLING_1;
        break;
    case STATE_COOLING_1:
        m_State = STATE_COOLING_2;
        break;
    case STATE_COOLING_2:
    default:
        m_State = STATE_STANDBY;
        m_StateTransitionTimer.stop();
        break;
    }
    qDebug() << "changed state to " << StateStr(m_State);
}

void SerialMock::readBytes()
{
    m_Cmd += m_SerialPort.readAll();
    int idx = m_Cmd.indexOf(0xA9);
    if (idx < 0) {
        qDebug() << "clearing garbage:" << m_Cmd;
        m_Cmd.clear();
    }
    else {
        m_Cmd = m_Cmd.mid(idx);
        while (m_Cmd.size() >= 8) {
            QByteArray cmd = m_Cmd.mid(0, 8);
            if (cmd == m_StatusCmd) {
                qDebug() << "STATUS, sending state" << StateStr(m_State);
                QByteArray response = BuildStatusRespons(m_State);
                m_SerialPort.write(response);
            }
            else if (cmd == m_PowerOnCmd) {
                qDebug() << "POWER ON";
                m_StateTransitionTimer.start();
            }
            else if (cmd == m_PowerOffCmd) {
                qDebug() << "POWER OFF";
                m_StateTransitionTimer.start();
            }
            else {
                qDebug() << "other cmd:" << cmd;
            }
            m_Cmd = m_Cmd.mid(8);
        }
    }
    if (m_Cmd.isEmpty()) {
        m_ResetTimer.stop();
    }
    else {
        m_ResetTimer.start();
    }
}

const char *SerialMock::StateStr(SerialMock::ProjectorState state)
{
    switch (state) {
    case STATE_STANDBY:
        return "STANDBY";
    case STATE_START_UP:
        return "START UP";
    case STATE_STARTUP_LAMP:
        return "STARTUP_LAMP";
    case STATE_POWER_ON:
        return "POWER ON";
    case STATE_COOLING_1:
        return "COOLING 1";
    case STATE_COOLING_2:
        return "COOLING 2";
    case STATE_SAVING_COOLING_1:
        return "SAVING COOLING 1";
    case STATE_SAVING_COOLING_2:
        return "SAVING COOLING 2";
    case STATE_SAVING_STANDBY:
        return "SAVING STANDBY";
    case STATE_UNKNOWN:
    default:
        return "UNKNOWN";
    }
}
