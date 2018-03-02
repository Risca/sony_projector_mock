#ifndef SERIALMOCK_H
#define SERIALMOCK_H

#include <QObject>
#include <QTimer>
#include <QByteArray>

#include <QtSerialPort/QSerialPort>

class SerialMock : public QObject
{
    Q_OBJECT
public:
    explicit SerialMock(QObject *parent = 0);

signals:

public slots:
    void clearCommandBuffer();
    void transitionState();
    void readBytes();

private:
    QSerialPort m_SerialPort;
    QTimer m_ResetTimer;
    QTimer m_StateTransitionTimer;
    QByteArray m_Cmd;
    QByteArray m_StatusCmd;
    QByteArray m_PowerOnCmd;
    QByteArray m_PowerOffCmd;
    enum ProjectorState {
      STATE_STANDBY = 0,
      STATE_START_UP = 1,
      STATE_STARTUP_LAMP = 2,
      STATE_POWER_ON = 3,
      STATE_COOLING_1 = 4,
      STATE_COOLING_2 = 5,
      STATE_SAVING_COOLING_1 = 6,
      STATE_SAVING_COOLING_2 = 7,
      STATE_SAVING_STANDBY = 8,
      STATE_UNKNOWN,
    };
    ProjectorState m_State;

    const char* StateStr(ProjectorState state);
};

#endif // SERIALMOCK_H
