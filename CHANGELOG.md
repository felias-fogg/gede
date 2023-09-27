# CHANGELOG

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

   

   
