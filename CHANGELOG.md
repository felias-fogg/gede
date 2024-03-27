# CHANGELOG

## V2.20.1 (27-Mar-2024)

1. Added `down.png` to `src/res` (where it already should have been)

   

## V2.20.0 (29-Sep-2023)

1. Added a "reload" operation. Interestingly, it leads sometimes to a GDB error. This seems to happen when the changes are too big. No idea why. I have not seen it when I used avr-gdb directly.

   In addition to the changes below, you need to add the icon down.png (which is the next icon turned by 90 degrees).

   ```diff
   diff --git a/src/core.cpp b/src/core.cpp
   index 555e13d..b222261 100644
   --- a/src/core.cpp
   +++ b/src/core.cpp
   @@ -847,6 +847,67 @@ void Core::gdbRun()
        }
    }
    
   +/**
   + * @brief Asks gdb to reload (a probably modified binary) and run it.
   + * This command is only active when we are in remote mode, download has been activated and an
   + * executable has been specified.
   + */
   +void Core::gdbReload(QString progpath)
   +{
   +    GdbCom& com = GdbCom::getInstance();
   +    Tree resultData;
   +    ICore::TargetState oldState;
   +    GdbResult rc;
   +
   +    if(m_targetState == ICore::TARGET_STARTING || m_targetState == ICore::TARGET_RUNNING)
   +    {
   +        if(m_inf)
   +            m_inf->ICore_onMessage("Program is currently running");
   +        return;
   +    }
   +
   +    //
   +
   +    rc = com.commandF(&resultData, "-file-symbol-file %s", stringToCStr(progpath));
   +    if (rc != GDB_ERROR)
   +      rc = com.commandF(&resultData, "-file-exec-file %s", stringToCStr(progpath));
   +    if (rc != GDB_ERROR)
   +      rc = com.command(&resultData, "-environment-directory -r");
   +    if (rc != GDB_ERROR)
   +      rc = com.command(&resultData, "-target-download");
   +    if (rc == GDB_ERROR) return;
   +
   +    if(m_ptsListener)
   +        delete m_ptsListener;
   +    m_ptsListener = new QSocketNotifier(m_ptsFd, QSocketNotifier::Read);
   +    connect(m_ptsListener, SIGNAL(activated(int)), this, SLOT(onGdbOutput(int)));
   +
   +    m_pid = 0;
   +    oldState = m_targetState;
   +    m_targetState = ICore::TARGET_STARTING;
   +    rc = com.commandF(&resultData, "-exec-run");
   +    if(rc == GDB_ERROR)
   +        m_targetState = oldState;
   +
   +    // Loop through all source files
   +    for(int i = 0;i < m_sourceFiles.size();i++)
   +      {
   +	SourceFile *sourceFile = m_sourceFiles[i];
   +	
   +	if(QFileInfo(sourceFile->m_fullName).exists())
   +	  {
   +	    // Has the file being modified?
   +	    QDateTime modTime = QFileInfo(sourceFile->m_fullName).lastModified();
   +	    if(sourceFile->m_modTime <  modTime)
   +	      {
   +		m_inf->ICore_onSourceFileChanged(sourceFile->m_fullName);
   +	      }
   +	  }
   +      }
   +
   +    // Get all source files
   +    gdbGetFiles();
   +}
    
    /**
     * @brief  Resumes the execution until a breakpoint is encountered, or until the program exits.
   diff --git a/src/core.h b/src/core.h
   index f8d13f7..f8a1543 100644
   --- a/src/core.h
   +++ b/src/core.h
   @@ -254,6 +254,7 @@ public:
        void gdbStepOut();
        void gdbContinue();
        void gdbRun();
   +    void gdbReload(QString progpath);
        bool gdbGetFiles();
    
        int getMemoryDepth();
   diff --git a/src/mainwindow.cpp b/src/mainwindow.cpp
   index 0ca64c8..fbbf836 100644
   --- a/src/mainwindow.cpp
   +++ b/src/mainwindow.cpp
   @@ -142,6 +142,7 @@ MainWindow::MainWindow(QWidget *parent)
        connect(m_ui.actionStep_In, SIGNAL(triggered()), SLOT(onStepIn()));
        connect(m_ui.actionStep_Out, SIGNAL(triggered()), SLOT(onStepOut()));
        connect(m_ui.actionRestart, SIGNAL(triggered()), SLOT(onRestart()));
   +    connect(m_ui.actionReload, SIGNAL(triggered()), SLOT(onReload()));
        connect(m_ui.actionContinue, SIGNAL(triggered()), SLOT(onContinue()));
    
        connect(m_ui.actionViewStack, SIGNAL(triggered()), SLOT(onViewStack()));
   @@ -1376,6 +1377,15 @@ void MainWindow::onRestart()
        core.gdbRun();
    }
    
   +void MainWindow::onReload()
   +{
   +    Core &core = Core::getInstance();
   +
   +    m_ui.targetOutputView->clearAll();
   +
   +    core.gdbReload(m_cfg.getProgramPath());
   +}
   +
    
    void MainWindow::onContinue()
    {
   @@ -2013,6 +2023,7 @@ void MainWindow::ICore_onStateChanged(TargetState state)
        m_ui.actionStop->setEnabled(isRunning);
        m_ui.actionContinue->setEnabled(isStopped);
        m_ui.actionRestart->setEnabled(!isRunning);
   +    m_ui.actionReload->setEnabled(!isRunning && m_cfg.m_download && !(m_cfg.getProgramPath().isEmpty()));
    
        m_ui.varWidget->setEnabled(isStopped);
    
   diff --git a/src/mainwindow.h b/src/mainwindow.h
   index 1456469..f2d9d4f 100644
   --- a/src/mainwindow.h
   +++ b/src/mainwindow.h
   @@ -152,6 +152,7 @@ public slots:
        void onStop();
        void onBreakpointsWidgetItemDoubleClicked(QTreeWidgetItem * item,int column);
        void onRestart();
   +    void onReload();
        void onContinue();
        void onCodeViewContextMenuAddWatch();
        void onCodeViewContextMenuOpenFile();
   diff --git a/src/mainwindow.ui b/src/mainwindow.ui
   index 92e04d5..3f24122 100644
   --- a/src/mainwindow.ui
   +++ b/src/mainwindow.ui
   @@ -589,6 +589,7 @@
        <property name="title">
         <string>Execution</string>
        </property>
   +    <addaction name="actionReload"/>
        <addaction name="actionRestart"/>
        <addaction name="actionStop"/>
        <addaction name="separator"/>
   @@ -651,6 +652,7 @@
       </attribute>
       <addaction name="actionQuit"/>
       <addaction name="separator"/>
   +   <addaction name="actionReload"/>
       <addaction name="actionRestart"/>
       <addaction name="actionStop"/>
       <addaction name="separator"/>
   @@ -735,6 +737,18 @@
       <property name="shortcut">
        <string>F11</string>
       </property>
   +  </action>
   +    <action name="actionReload">
   +   <property name="icon">
   +    <iconset resource="resource.qrc">
   +     <normaloff>:/images/res/down.png</normaloff>:/images/res/down.png</iconset>
   +   </property>
   +   <property name="text">
   +    <string>Reload</string>
   +   </property>
   +   <property name="shortcut">
   +    <string>F12</string>
   +   </property>
      </action>
      <action name="actionSettings">
       <property name="text">
   diff --git a/src/resource.qrc b/src/resource.qrc
   index 078058d..adbf418 100644
   --- a/src/resource.qrc
   +++ b/src/resource.qrc
   @@ -10,5 +10,6 @@
        <file>res/file.png</file>
        <file>res/folder.png</file>
        <file>res/step_out.png</file>
   +    <file>res/down.png</file>
     </qresource>
    </RCC>
   diff --git a/src/version.h b/src/version.h
   index 05ccae6..0eb637e 100644
   --- a/src/version.h
   +++ b/src/version.h
   @@ -10,8 +10,8 @@
    #define FILE__VERSION_H
    
    #define GD_MAJOR 2
   -#define GD_MINOR 19
   -#define GD_PATCH 5
   +#define GD_MINOR 20
   +#define GD_PATCH 0
    
    
    #endif // FILE__VERSION_H
   
   ```

   

## V2.19.5 (29-Sep-2023)

1. Added "/private" to the list of excluded directories since this sometimes pops up on my Mac.

   ```diff
   diff --git a/src/settings.cpp b/src/settings.cpp
   index db73441..ca36914 100644
   --- a/src/settings.cpp
   +++ b/src/settings.cpp
   @@ -129,7 +129,7 @@ void Settings::loadDefaultsAdvanced()
        m_sourceIgnoreDirs.clear();
        m_sourceIgnoreDirs.append("/build");
        m_sourceIgnoreDirs.append("/usr");
   -    
   +    m_sourceIgnoreDirs.append("/private");
    }
    
   ```

## V2.19.4 (27-Sep-2023)

1. The Arduino INO files were not included in ctag scanning so that the functions did not show up in the functions window. The .ino file extension has been added:

   ```diff
   diff --git a/src/config.h b/src/config.h
   index 06a8537..c8a9365 100644
   --- a/src/config.h
   +++ b/src/config.h
   @@ -22,7 +22,7 @@
    // etags command and argument to use to get list of tags
    #define ETAGS_CMD1     "ctags"    // Used on Linux
    #define ETAGS_CMD2     "exctags"  // Used on freebsd
   -#define ETAGS_ARGS    " -f - --excmd=number --fields=+nmsSk"
   +#define ETAGS_ARGS    " -f - --excmd=number --fields=+nmsSk --langmap=c++:+.ino"
   
   ```

2. On macOS: In the serial port combo box, also the tty.* serial ports were listed, which should not be used. I filtered them out.

   ```diff
   diff --git a/src/opendialog.cpp b/src/opendialog.cpp
   index 97d0492..05742fa 100644
   --- a/src/opendialog.cpp
   +++ b/src/opendialog.cpp
   @@ -56,6 +56,9 @@ OpenDialog::OpenDialog(QWidget *parent)
        for(QSerialPortInfo &port : portList)
        {
            QString text = port.portName() + ": " + port.description();
   +#if __APPLE__
   +       if (text.startsWith("tty.")) continue;
   +#endif
            m_ui.comboBox_serialPort->addItem(text, QVariant(port.systemLocation()));
            if(text.startsWith("ttyUSB") || text.startsWith("ttyACM"))
                selectIdx = m_ui.comboBox_serialPort->count()-1;
   
   ```

3. On macOS: The directory gede.app needs to be deleted when cleaning up

   ```diff
   diff --git a/build.py b/build.py
   index 6b81716..0da2a67 100755
   --- a/build.py
   +++ b/build.py
   @@ -106,6 +106,8 @@ def doClean():
            removeFile(g_exeName)
            removeFile("Makefile")
            removeFile(".qmake.stash")
   +        if platform == "darwin" and os.path.exists(g_exeName + ".app"):
   +            shutil.rmtree(g_exeName + ".app")
            os.chdir(oldP)
    
   ```

   



## V2.19.3 (25-Sep-2023)

1. You could not set the file name in the opening dialog when you debug remotely. Now the field is enabled in case you debug remotely:

```diff
diff --git a/src/opendialog.cpp b/src/opendialog.cpp
index c20ab6a..97d0492 100644
--- a/src/opendialog.cpp
+++ b/src/opendialog.cpp
@@ -276,6 +276,7 @@ void OpenDialog::onConnectionTypeLocal(bool checked)
 
 void OpenDialog::onConnectionTypeTcp(bool checked)
 {
+    m_ui.pushButton_selectFile->setEnabled(checked);
     m_ui.lineEdit_tcpHost->setEnabled(checked);
     m_ui.lineEdit_tcpPort->setEnabled(checked);
     m_ui.checkBox_download->setEnabled(checked);
@@ -284,6 +285,7 @@ void OpenDialog::onConnectionTypeTcp(bool checked)
 
 void OpenDialog::onConnectionTypeSerial(bool checked)
 {
+    m_ui.pushButton_selectFile->setEnabled(checked);
     m_ui.comboBox_baudRate->setEnabled(checked);
     m_ui.comboBox_serialPort->setEnabled(checked);
     m_ui.checkBox_download->setEnabled(checked);
```



2. Gede communicates with the remote target for 20 seconds when starting up. The command GDB sends to the target looks very strange. Apparently, this is caused by executing the target-select command without having specified a binary file before (my best guess is that this is caused by an uninitialized variable in GDB). Executing file-symbols-file and file-exec-file before target-select avoids this problem.

   ```diff
   diff --git a/src/core.cpp b/src/core.cpp
   index d7c28d1..555e13d 100644
   --- a/src/core.cpp
   +++ b/src/core.cpp
   @@ -517,22 +517,26 @@ int Core::initRemote(Settings *cfg, QString gdbPath, QString programPath, QStrin
            return -1;
        }
    
   -    com.commandF(&resultData, "-target-select extended-remote %s:%d", stringToCStr(tcpHost), tcpPort); 
   -
        if(!programPath.isEmpty())
        {
            com.commandF(&resultData, "-file-symbol-file %s", stringToCStr(programPath));
    
        }
    
   -    runInitCommands(cfg);
   -
        if(!programPath.isEmpty())
        {
          com.commandF(&resultData, "-file-exec-file %s", stringToCStr(programPath));
    
   -        if(cfg->m_download)
   -            com.commandF(&resultData, "-target-download");
   +    }
   +
   +    com.commandF(&resultData, "-target-select extended-remote %s:%d", stringToCStr(tcpHost), tcpPort); 
   +
   +    runInitCommands(cfg);
   +
   +    if(!programPath.isEmpty())
   +    {
   +      if(cfg->m_download)
   +	com.commandF(&resultData, "-target-download");
        }
        
        // Get memory depth (32 or 64)
   @@ -567,23 +571,25 @@ int Core::initSerial(Settings *cfg, QString gdbPath, QString programPath, QStrin
    
        com.commandF(&resultData, "set serial baud %d", baudRate); 
    
   -    com.commandF(&resultData, "-target-select extended-remote %s", stringToCStr(serialPort)); 
   -
   -
        if(!programPath.isEmpty())
        {
            com.commandF(&resultData, "-file-symbol-file %s", stringToCStr(programPath));
    
        }
    
   -    runInitCommands(cfg);
   -
        if(!programPath.isEmpty())
        {
          com.commandF(&resultData, "-file-exec-file %s", stringToCStr(programPath));
   +    }
   +    
   +    com.commandF(&resultData, "-target-select extended-remote %s", stringToCStr(serialPort)); 
   +
   +    runInitCommands(cfg);
    
   -        if(cfg->m_download)
   -            com.commandF(&resultData, "-target-download");
   +    if(!programPath.isEmpty())
   +    {
   +      if(cfg->m_download)
   +	com.commandF(&resultData, "-target-download");
        }
        
        // Get memory depth (32 or 64)
   
   ```

   

3. On macOS: One should ask for the presence of clang instead of gcc:

   ```diff
   diff --git a/build.py b/build.py
   index 28b4a97..6b81716 100755
   --- a/build.py
   +++ b/build.py
   @@ -28,7 +28,10 @@ g_otherSrcDirs = [
                "./tests/ini"
                ]
    g_mainSrcDir = ["./src" ]
   -g_requiredPrograms = ["make", "gcc", "ctags" ]
   +if platform == "darwin":
   +    g_requiredPrograms = ["make", "clang", "ctags" ]
   +else:
   +    g_requiredPrograms = ["make", "gcc", "ctags" ]
    MIN_QT_VER = "4.0.0"
    #-----------------------------#
    
   ```

   

4. The baud rate is (almost) always set to 1200 baud. This seems to be a Qt problem, though.

   Only when you have selected the baud rate field explicitly, then the set baud rate is passed on. Otherwise, 1200 baud is selected (although 115200 was shown). A quick fix is to remove the "editable" property from the baud rate.  I guess, if one leaves it at that, one should delete all baud rates below 19200 and add a few higher ones (for the black magic probe). I am not sure whether a better fix is possible.

   ```diff
   diff --git a/src/opendialog.ui b/src/opendialog.ui
   index ceb7048..d309a09 100644
   --- a/src/opendialog.ui
   +++ b/src/opendialog.ui
   @@ -230,7 +230,7 @@
                </sizepolicy>
               </property>
               <property name="editable">
   -            <bool>true</bool>
   +            <bool>false</bool>
               </property>
              </widget>
             </item>
   
   ```

   

5. Gede accumulated the initial breakpoint many times over when loading breakpoints from last time. Now we check in the loadBreakpoints method whether a breakpoint is already there before inserting it.

   ```diff
   diff --git a/src/gd.cpp b/src/gd.cpp
   index 04288df..2221893 100644
   --- a/src/gd.cpp
   +++ b/src/gd.cpp
   @@ -70,7 +70,9 @@ void loadBreakpoints(Settings &cfg, Core &core)
        {
            SettingsBreakpoint bkptCfg = cfg.m_breakpoints[i];
            debugMsg("Setting breakpoint at %s:L%d", qPrintable(bkptCfg.m_filename), bkptCfg.m_lineNo);
   -        core.gdbSetBreakpoint(bkptCfg.m_filename, bkptCfg.m_lineNo);
   +	if (core.findBreakPoint(bkptCfg.m_filename, bkptCfg.m_lineNo) == NULL)
   +	  // insert only if not already there
   +	  core.gdbSetBreakpoint(bkptCfg.m_filename, bkptCfg.m_lineNo);
        }
    }
    
   ```

   

   
