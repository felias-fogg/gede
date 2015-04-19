#include "memorydialog.h"

#include "core.h"

QByteArray MemoryDialog::getMemory(unsigned int startAddress, int count)
{
     Core &core = Core::getInstance();
   
    QByteArray b;
    core.gdbGetMemory(startAddress, count, &b);

    return b;
}

MemoryDialog::MemoryDialog(QWidget *parent)
    : QDialog(parent)
{
    
    m_ui.setupUi(this);

    m_ui.verticalScrollBar->setRange(0,0xffffffffUL/16);
    connect(m_ui.verticalScrollBar, SIGNAL(valueChanged(int)), this, SLOT(onVertScroll(int)));

    m_ui.memorywidget->setInterface(this);

}

void MemoryDialog::setStartAddress(unsigned int addr)
{
    m_ui.memorywidget->setStartAddress(addr);
    m_ui.verticalScrollBar->setValue(addr/16);

    QString addrText;
    addrText.sprintf("0x%x", addr);
    m_ui.lineEdit_address->setText(addrText);
}


void MemoryDialog::onVertScroll(int pos)
{
    m_ui.memorywidget->setStartAddress(((unsigned int)pos)*16UL);
}



