#ifndef FILE__MEMORYWIDGET_H
#define FILE__MEMORYWIDGET_H

#include <QWidget>
#include <QFont>
#include <QScrollBar>

#include <stdint.h>

class IMemoryWidget
{
public:
    virtual QByteArray getMemory(unsigned int startAddress, int count) = 0;

};

class MemoryWidget : public QWidget
{
    Q_OBJECT

public:

    MemoryWidget(QWidget *parent = NULL);
    virtual ~MemoryWidget();

 void paintEvent ( QPaintEvent * event );
    void setInterface(IMemoryWidget *inf);
    
private:
    int getRowHeight();
    unsigned int getAddrAtPos(QPoint pos);
    int getHeaderHeight();
    char byteToChar(uint8_t d);
    
public slots:
    void setStartAddress(unsigned int addr);

private:
    void mousePressEvent(QMouseEvent * event);
    void mouseMoveEvent ( QMouseEvent * event );
    void mouseReleaseEvent(QMouseEvent * event);
    
private:
    QFont m_font;
    QFontMetrics *m_fontInfo;

    bool m_selectionStartValid;
    unsigned int m_startAddress;
    unsigned int m_selectionStart, m_selectionEnd;
    IMemoryWidget *m_inf;
    
};

#endif // FILE__MEMORYWIDGET_H

