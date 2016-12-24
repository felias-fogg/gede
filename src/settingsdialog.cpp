/*
 * Copyright (C) 2014-2015 Johan Henriksson.
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms
 * of the BSD license.  See the LICENSE file for details.
 */

#include "settingsdialog.h"
#include "version.h"
#include "log.h"
#include "util.h"


#include <QFontDialog>


SettingsDialog::SettingsDialog(QWidget *parent, Settings *cfg)
    : QDialog(parent)
    ,m_cfg(cfg)
{
    
    m_ui.setupUi(this);

    connect(m_ui.pushButton_selectFont, SIGNAL(clicked()), SLOT(onSelectFont()));
    connect(m_ui.pushButton_selectMemoryFont, SIGNAL(clicked()), SLOT(onSelectMemoryFont()));
    connect(m_ui.pushButton_selectOutputFont, SIGNAL(clicked()), SLOT(onSelectOutputFont()));
    connect(m_ui.pushButton_selectGdbOutputFont, SIGNAL(clicked()), SLOT(onSelectGdbOutputFont()));

    QObject::connect(m_ui.buttonBox, SIGNAL(clicked(QAbstractButton*)), this, SLOT(onButtonBoxClicked(QAbstractButton*)));



    loadConfig();
    
    updateGui();
    
}

void SettingsDialog::onButtonBoxClicked(QAbstractButton* button)
{
    // Clicked on the "Restore Defaults" button?
    if(m_ui.buttonBox->standardButton(button) == QDialogButtonBox::RestoreDefaults)
    {
        m_cfg->loadDefaultsGui();
        m_cfg->loadDefaultsAdvanced();
        loadConfig();

        updateGui();
    }
}


void SettingsDialog::updateGui()
{
    QString labelText;
    labelText.sprintf("%s  %d", stringToCStr(m_settingsFontFamily), m_settingsFontSize);
    m_ui.pushButton_selectFont->setText(labelText);

    labelText.sprintf("%s  %d", stringToCStr(m_settingsMemoryFontFamily), m_settingsMemoryFontSize);
    m_ui.pushButton_selectMemoryFont->setText(labelText);

    labelText.sprintf("%s  %d", stringToCStr(m_settingsOutputFontFamily), m_settingsOutputFontSize);
    m_ui.pushButton_selectOutputFont->setText(labelText);

    labelText.sprintf("%s  %d", stringToCStr(m_settingsGdbOutputFontFamily), m_settingsGdbOutputFontSize);
    m_ui.pushButton_selectGdbOutputFont->setText(labelText);
}    

void SettingsDialog::loadConfig()
{
    m_settingsFontFamily = m_cfg->m_fontFamily;
    m_settingsFontSize = m_cfg->m_fontSize;
    m_settingsMemoryFontFamily = m_cfg->m_memoryFontFamily;
    m_settingsMemoryFontSize = m_cfg->m_memoryFontSize;
    m_settingsOutputFontFamily = m_cfg->m_outputFontFamily;
    m_settingsOutputFontSize = m_cfg->m_outputFontSize;
    m_settingsGdbOutputFontFamily = m_cfg->m_gdbOutputFontFamily;
    m_settingsGdbOutputFontSize = m_cfg->m_gdbOutputFontSize;

    m_ui.lineEdit_sourceIgnoreDirs->setText(m_cfg->m_sourceIgnoreDirs.join(";"));

    m_ui.pushButton_clr_background->setColor(m_cfg->m_clrBackground);
    m_ui.pushButton_clr_comment->setColor(m_cfg->m_clrComment);
    m_ui.pushButton_clr_string->setColor(m_cfg->m_clrString);
    m_ui.pushButton_clr_incString->setColor(m_cfg->m_clrIncString);
    m_ui.pushButton_clr_keyword->setColor(m_cfg->m_clrKeyword);
    m_ui.pushButton_clr_cppKeyword->setColor(m_cfg->m_clrCppKeyword);
    m_ui.pushButton_clr_number->setColor(m_cfg->m_clrNumber);
    m_ui.pushButton_clr_foreground->setColor(m_cfg->m_clrForeground);

    m_ui.checkBox_showLineNumbers->setCheckState(m_cfg->m_tagShowLineNumbers ? Qt::Checked : Qt::Unchecked);


    int comboIdx = 0;
    if(m_cfg->m_tagSortByName)
        comboIdx = 1;
    else
        comboIdx = 0;
    m_ui.comboBox_sortTags->setCurrentIndex(comboIdx);
}

void SettingsDialog::saveConfig()
{
    getConfig(m_cfg);
}

void SettingsDialog::getConfig(Settings *cfg)
{
    cfg->m_fontFamily = m_settingsFontFamily;
    cfg->m_fontSize = m_settingsFontSize;

    cfg->m_memoryFontFamily = m_settingsMemoryFontFamily;
    cfg->m_memoryFontSize = m_settingsMemoryFontSize;

    cfg->m_outputFontFamily = m_settingsOutputFontFamily;
    cfg->m_outputFontSize = m_settingsOutputFontSize;

    cfg->m_gdbOutputFontFamily = m_settingsGdbOutputFontFamily;
    cfg->m_gdbOutputFontSize = m_settingsGdbOutputFontSize;

    cfg->m_sourceIgnoreDirs = m_ui.lineEdit_sourceIgnoreDirs->text().split(';');

    cfg->m_tagShowLineNumbers = (m_ui.checkBox_showLineNumbers->checkState() == Qt::Unchecked) ? false : true;

    cfg->m_clrBackground = m_ui.pushButton_clr_background->getColor();
    cfg->m_clrComment = m_ui.pushButton_clr_comment->getColor();
    cfg->m_clrString = m_ui.pushButton_clr_string->getColor();
    cfg->m_clrIncString = m_ui.pushButton_clr_incString->getColor();
    cfg->m_clrKeyword = m_ui.pushButton_clr_keyword->getColor();
    cfg->m_clrCppKeyword = m_ui.pushButton_clr_cppKeyword->getColor();
    cfg->m_clrNumber = m_ui.pushButton_clr_number->getColor();
    cfg->m_clrForeground = m_ui.pushButton_clr_foreground->getColor();


    int comboIdx = m_ui.comboBox_sortTags->currentIndex();
    if(comboIdx == 0)
        m_cfg->m_tagSortByName = false;
    if(comboIdx == 1)
        m_cfg->m_tagSortByName = true;
    
    
}


    
void SettingsDialog::onSelectFont()
{
    showFontSelection(&m_settingsFontFamily, &m_settingsFontSize);
}

void SettingsDialog::showFontSelection(QString *fontFamily, int *fontSize)
{
    bool ok;
    QFont font = QFontDialog::getFont(
                 &ok, QFont(*fontFamily, *fontSize), this);
    if (ok)
    {
        *fontFamily = font.family();
        *fontSize = font.pointSize();

        updateGui();
    
    } else
    {
    }

}


void SettingsDialog::onSelectMemoryFont()
{
    showFontSelection(&m_settingsMemoryFontFamily, &m_settingsMemoryFontSize);
}


void SettingsDialog::onSelectOutputFont()
{
    showFontSelection(&m_settingsOutputFontFamily, &m_settingsOutputFontSize);
}

void SettingsDialog::onSelectGdbOutputFont()
{
    showFontSelection(&m_settingsGdbOutputFontFamily, &m_settingsGdbOutputFontSize);
}

