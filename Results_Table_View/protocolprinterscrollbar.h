#ifndef PROTOCOLPRINTERSCROLLBAR_H
#define PROTOCOLPRINTERSCROLLBAR_H

#include <QScrollBar>

class ProtocolPrinterScrollBar : public QScrollBar
{
public:
    ProtocolPrinterScrollBar(Qt::Orientation orientation, QWidget *parent = nullptr);

    // QObject interface
public:
    virtual bool event(QEvent *event) override;

};

#endif // PROTOCOLPRINTERSCROLLBAR_H
