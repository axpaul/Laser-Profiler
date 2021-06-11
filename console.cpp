#include "console.h"

#include <QScrollBar>

#include <QtCore/QDebug>

Console::Console(QWidget *parent) : QPlainTextEdit(parent)//, localEchoEnabled(true)
{
    document()->setMaximumBlockCount(100);
    QPalette p = palette();
    p.setColor(QPalette::Base, Qt::white);
    p.setColor(QPalette::Text, Qt::black);
    setPalette(p);
    isReadOnly();

}

void Console::putData(const QByteArray &data)
{
    insertPlainText(QString(data));

    QScrollBar *bar = verticalScrollBar();
    bar->setValue(bar->maximum());
}

/*void Console::setLocalEchoEnabled(bool set)
{
    localEchoEnabled = set;
}*/

void Console::keyPressEvent(QKeyEvent *e)
{
    Q_UNUSED(e)
}

/*void Console::mousePressEvent(QMouseEvent *e)
{
    Q_UNUSED(e)
    setFocus();
}*/

void Console::mouseDoubleClickEvent(QMouseEvent *e)
{
    Q_UNUSED(e)
}

/*void Console::contextMenuEvent(QContextMenuEvent *e)
{
    Q_UNUSED(e)
}*/

