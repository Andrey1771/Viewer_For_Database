#ifndef PROTOCOLPRINTERHEADERVIEWEXTENDED_H
#define PROTOCOLPRINTERHEADERVIEWEXTENDED_H

#include "protocolprinterheaderview.h"

class NewModel;

class ProtocolPrinterHeaderViewExtended : public ProtocolPrinterHeaderView
{
public:
    ProtocolPrinterHeaderViewExtended(QTableView *parent);

    // ProtocolPrinterHeaderView interface
public:
    virtual void startSetSettings(QTableView *parent) override;

private:
    NewModel *newModel{nullptr};

};

#endif // PROTOCOLPRINTERHEADERVIEWEXTENDED_H
