/*
 * Copyright (C) 2014-2017 Johan Henriksson.
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms
 * of the BSD license.  See the LICENSE file for details.
 */


#include "memorywidget.h"

#include <QApplication>
#include <QClipboard>
#include <stdint.h>
#include <QPainter>
#include <QPaintEvent>
#include <QColor>
#include "log.h"
#include "util.h"

static const int PAD_ADDR_LEFT = 10; //!< Pad length left to the address field
static const int PAD_ADDR_RIGHT = 10; //!< Pad length right to the address field.
static const int PAD_HEX_MIDDLE = 10;  //!< Space between the first 8 and the last 8 bytes in a row
static const int PAD_HEX_RIGHT = 10;   //!< Pad length right to the hex field.
static const int PAD_DATA = 5;
static const int BYTES_PER_ROW = 16;


MemoryWidget::MemoryWidget(QWidget *parent)
 : QWidget(parent)
 ,m_selectionStart(0)
 ,m_selectionEnd(0)
 ,m_inf(0)
{
    m_font = QFont("Monospace", 10);
    m_fontInfo = new QFontMetrics(m_font);



    setFocusPolicy(Qt::StrongFocus);

    setMinimumSize(400,getRowHeight()*10);
    update();

    m_startAddress = 0;
}

MemoryWidget::~MemoryWidget()
{
    delete m_fontInfo;
}


void MemoryWidget::setConfig(Settings *cfg)
{
    delete m_fontInfo;
    m_font = QFont(cfg->m_memoryFontFamily, cfg->m_memoryFontSize);
    m_fontInfo = new QFontMetrics(m_font);

    update();
}

void MemoryWidget::keyPressEvent(QKeyEvent *e)
{
    if(e->type() == QKeyEvent::KeyPress)
    {
        if(e->matches(QKeySequence::Copy))
        {
            onCopy();
        }
    }
    QWidget::keyPressEvent(e);
}

void MemoryWidget::setInterface(IMemoryWidget *inf)
{
    m_inf = inf;
}


void MemoryWidget::setStartAddress(unsigned int addr)
{

    m_startAddress = addr;
    update();
}


/**
 * @brief Returns the height of a text row in pixels.
 */
int MemoryWidget::getRowHeight()
{
    int rowHeight = m_fontInfo->lineSpacing()+2;
    return rowHeight;
}



int MemoryWidget::getHeaderHeight()
{
    return getRowHeight()+5;
}


char MemoryWidget::byteToChar(uint8_t d)
{
    char c;
    if(0x21 <= d && d <= 0x7e)
        c = (char)d;
    else
        c = '.';
    return c;
}



void MemoryWidget::paintEvent ( QPaintEvent * event )
{
    QPainter painter(this);
    const int rowHeight = getRowHeight();
    const int charWidth = m_fontInfo->width("H");
    QString text;
    int HEADER_HEIGHT = getHeaderHeight();
    int x;
    int rowCount = ((size().height()-HEADER_HEIGHT)/rowHeight)+1;
    unsigned int startAddress = m_startAddress;

    unsigned int selectionFirst;
    unsigned int selectionLast;
    if(m_selectionEnd < m_selectionStart)
    {
        selectionFirst = m_selectionEnd;
        selectionLast = m_selectionStart;
    }
    else
    {
        selectionFirst = m_selectionStart;
        selectionLast = m_selectionEnd;
    }
     
    
    painter.setFont(m_font);

    QByteArray content;
    if(m_inf)
        content = m_inf->getMemory(startAddress, rowCount*16);
    

    //if((0xffffffffU-startAddress) < rowCount*16)
    //    startAddress = 0xffffffffU-((rowCount-2)*16);
    
    // Draw 'address' field background
    QRect rect2(0,0,PAD_ADDR_LEFT+charWidth*9+PAD_ADDR_RIGHT/2, event->rect().bottom()+1);
    painter.fillRect(rect2, Qt::lightGray);

    // Draw 'header' background
    QRect rect3(0,0, event->rect().right()+1, HEADER_HEIGHT);
    painter.fillRect(rect3, Qt::cyan);

    // Draw 'header' frame
    rect3 = QRect(0,HEADER_HEIGHT, event->rect().right(), HEADER_HEIGHT);
    painter.setPen(Qt::black);
    painter.drawLine(0, HEADER_HEIGHT, event->rect().right(), HEADER_HEIGHT);
    
    // Draw header
    text.sprintf("Address");
    x = PAD_ADDR_LEFT;
    painter.drawText(PAD_ADDR_LEFT, rowHeight, text);
    x += (charWidth*9)+PAD_ADDR_RIGHT;
    for(int off = 0;off < 16;off++)
    {
        text.sprintf("%x", off);
        painter.drawText(x, rowHeight, text);
        x += (charWidth*2)+PAD_DATA;
        if(off==8)
            x += PAD_HEX_MIDDLE;
    }
    x += PAD_HEX_RIGHT;

    rect2 = QRect(x,HEADER_HEIGHT+1,x+charWidth*16, event->rect().bottom());
    painter.fillRect(rect2, Qt::lightGray);

    for(int off = 0;off < 16;off++)
    {
        text.sprintf("%x", off);
        painter.drawText(x, rowHeight, text);
        x += charWidth;
    }

    // Draw data
    for(int rowIdx= 0;rowIdx < rowCount;rowIdx++)
    {
        int y = HEADER_HEIGHT+rowHeight*rowIdx+rowHeight;
        x = PAD_ADDR_LEFT;
        
        unsigned int memoryAddr = startAddress + rowIdx*16;
        if(memoryAddr < startAddress)
            break;
            
        painter.setPen(Qt::black);
        text.sprintf("%04x_%04x", (unsigned int)(memoryAddr>>16),(unsigned int)(memoryAddr&0xffffUL));
        painter.drawText(x, y, text);
        x += charWidth*text.length();
        x += PAD_ADDR_RIGHT;
        
        for(int off = 0;off < 16;off++)
        {
            int dataIdx = rowIdx*16+off;
            if(dataIdx < content.size())
            {
            uint8_t d = content[dataIdx];

            if(selectionFirst != 0 || selectionLast != 0)
            {
                if(selectionFirst <= off+memoryAddr && off+memoryAddr <=  selectionLast)
                    painter.setPen(Qt::red);
                else
                    painter.setPen(Qt::black);
            }
            
            text.sprintf("%02x", d);
            painter.drawText(x, y, text);
            }
        
            x += charWidth*text.length()+5;

            if(off == 8)
                x += PAD_HEX_MIDDLE;
        }
        x += PAD_HEX_RIGHT;

        painter.setPen(Qt::black);
            
        for(int off = 0;off < 16;off++)
        {
            int dataIdx = rowIdx*16+off;
            if(dataIdx < content.size())
            {
                char c2 = byteToChar(content[dataIdx]);

            if(selectionFirst != 0 || selectionLast != 0)
            {
                if(selectionFirst <= off+memoryAddr && off+memoryAddr <=  selectionLast)
                    painter.setPen(Qt::red);
                else
                    painter.setPen(Qt::black);
            }
            
            painter.drawText(x, y, QString(c2));
            }
            x += charWidth*1;
        }


    }

    // Draw border
    painter.setPen(Qt::black);
    painter.drawRect(0,0, frameSize().width()-2,frameSize().height()-1);

}



unsigned int MemoryWidget::getAddrAtPos(QPoint pos)
{
    const int rowHeight = getRowHeight();
    const int charWidth = m_fontInfo->width("H");
    unsigned int addr;
    const int field_hex_width = PAD_HEX_MIDDLE + 16*(PAD_DATA+charWidth*2) + PAD_HEX_RIGHT;
    const int field_address_width = PAD_ADDR_LEFT+(PAD_DATA+charWidth*9)+PAD_ADDR_RIGHT;
    int idx = 0;
    
    addr = m_startAddress+(pos.y()-getHeaderHeight())/rowHeight*BYTES_PER_ROW;

    // Adjust for the address column
    int x = pos.x();
    x -= field_address_width;
    if(x > 0)
    {
        // In the ascii field?
        if(x >= field_hex_width)
        {
            idx = (x-field_hex_width)/charWidth;
        }
        else
        {
            // Adjust for the middle space
            if(x > (PAD_HEX_MIDDLE+8*((charWidth*2)+5)))
                x -= PAD_HEX_MIDDLE;

            // Get the character index
            idx = x / (((charWidth*2)+5));
        }
    }
    if(idx < 0)
        idx = -1;
    else if(BYTES_PER_ROW-1 < idx)
        idx = BYTES_PER_ROW-1;

    addr += idx;
    return addr;
 } 

void MemoryWidget::mouseMoveEvent ( QMouseEvent * event )
{
    m_selectionEnd = getAddrAtPos(event->pos());

    debugMsg("addr:%x", getAddrAtPos(event->pos()));

    update();
}
    
void MemoryWidget::mouseReleaseEvent(QMouseEvent * event)
{
    Q_UNUSED(event);
    
    update();
    
}

void MemoryWidget::mousePressEvent(QMouseEvent * event)
{
    if(event->button() == Qt::RightButton)
    {
        QPoint pos = event->globalPos();

        //
        m_popupMenu.clear();
        // Add 'copy'
        QAction *action = m_popupMenu.addAction("Copy");
        connect(action, SIGNAL(triggered()), this, SLOT(onCopy()));

        m_popupMenu.popup(pos);

    }
    else
    {
        m_selectionStart = getAddrAtPos(event->pos());
        m_selectionEnd = m_selectionStart;
    }


    update();    
}



void MemoryWidget::onCopy()
{
    unsigned int selectionFirst,selectionLast;
    
    if(m_selectionEnd < m_selectionStart)
    {
        selectionFirst = m_selectionEnd;
        selectionLast = m_selectionStart;
    }
    else
    {
        selectionFirst = m_selectionStart;
        selectionLast = m_selectionEnd;
    }

    if(m_inf)
    {
        QByteArray content;
        content = m_inf->getMemory(selectionFirst, selectionLast-selectionFirst+1);

        QString contentStr;
        QString subText;
        for(uint64_t addr = (selectionFirst & ~0xfULL);addr <= selectionLast;addr+=16)
        {
            unsigned int j;
            
            // Display address
            subText.sprintf("0x%08llx | ", (long long)addr);
            contentStr += subText;

            // Display data as hex
            for(j = 0;j < 16;j++)
            {
                if(selectionFirst <= addr+j && addr+j <= selectionLast) 
                {
                    uint8_t b = (unsigned char)content[(int)(addr+j-selectionFirst)];
                    subText.sprintf("%02x ", b);
                }
                else
                    subText = "   ";
                if(j == 7)
                    subText += " ";
                contentStr += subText;
            }
            contentStr += "| ";

            // Display data as ascii
            for(j = 0;j < 16;j++)
            {
                if(selectionFirst <= addr+j && addr+j <= selectionLast) 
                {
                    uint8_t b = content[(int)(addr+j-selectionFirst)];
                    subText.sprintf("%c", byteToChar(b));
                }
                else
                    subText = " ";
                if(j == 7)
                    subText += "  ";
                contentStr += subText;
            }
            contentStr += "\n";
        }
        QClipboard * clipboard = QApplication::clipboard();
        clipboard->setText(contentStr);
    }
}



