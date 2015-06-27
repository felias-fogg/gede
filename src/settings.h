/*
 * Copyright (C) 2014-2015 Johan Henriksson.
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms
 * of the BSD license.  See the LICENSE file for details.
 */

#ifndef FILE_SETTINGS_H
#define FILE_SETTINGS_H

#include <QString>
#include <QStringList>

enum ConnectionMode
{
    MODE_LOCAL = 0,
    MODE_TCP
    
};

class SettingsBreakpoint
{
public:
    
    QString filename;
    int lineNo;
};


class Settings
{
    public:
        Settings();

        void load(QString filepath);
        void save(QString filepath);

        static QStringList getDefaultKeywordList();
        static QStringList getDefaultCppKeywordList();
        
        
    public:
        QStringList m_argumentList;
        ConnectionMode m_connectionMode;
        int m_tcpPort;
        QString m_tcpHost;
        QString m_tcpProgram;
        QStringList m_initCommands;
        QString m_gdbPath;
        QString m_lastProgram;
        QString m_fontFamily;
        int m_fontSize;
        QString m_memoryFontFamily;
        int m_memoryFontSize;
        bool m_reloadBreakpoints;

        QList<SettingsBreakpoint> m_breakpoints;
};

#endif // FILE_SETTINGS_H

