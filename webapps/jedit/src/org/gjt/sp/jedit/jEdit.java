/*
 * jEdit.java - Main class of the jEdit editor
 * :tabSize=8:indentSize=8:noTabs=false:
 * :folding=explicit:collapseFolds=1:
 *
 * Copyright (C) 1998, 2005 Slava Pestov
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or any later version.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

package org.gjt.sp.jedit;

//{{{ Imports

import bsh.UtilEvalError;
import com.microstar.xml.*;

import javax.swing.*;
import javax.swing.Timer;
import java.awt.event.KeyEvent;
import java.awt.*;
import java.io.*;
import java.net.*;
import java.text.MessageFormat;
import java.util.*;
import java.util.List;
import java.applet.Applet;
import java.applet.AppletContext;
import java.lang.reflect.Method;

import org.gjt.sp.jedit.buffer.BufferIORequest;
import org.gjt.sp.jedit.buffer.KillRing;
import org.gjt.sp.jedit.msg.*;
import org.gjt.sp.jedit.gui.*;
import org.gjt.sp.jedit.help.HelpViewer;
import org.gjt.sp.jedit.io.*;
import org.gjt.sp.jedit.pluginmgr.PluginManager;
import org.gjt.sp.jedit.search.SearchAndReplace;
import org.gjt.sp.jedit.syntax.*;
import org.gjt.sp.jedit.textarea.*;
import org.gjt.sp.util.Log;

//}}}

/**
 * The main class of the jEdit text editor.
 *
 * @author Slava Pestov
 * @version $Id$
 */
public class jEdit extends Applet
{
  //{{{ getVersion() method
  /**
   * Returns the jEdit version as a human-readable string.
   */
  public static String getVersion()
  {
    return MiscUtilities.buildToVersion(getBuild());
  } //}}}

  //{{{ getBuild() method
  /**
   * Returns the internal version. MiscUtilities.compareStrings() can be used
   * to compare different internal versions.
   */
  public static String getBuild()
  {
    // (major).(minor).(<99 = preX, 99 = final).(bug fix)
    return "04.03.02.00";
  } //}}}

  public static URL baseUrl = null;
  public static URL servletUrl = null;
  public static int ping_port;
  public static int ping_timeout;
  public static String username = null;
  public static String password = null;
  public static char separatorChar ;
  private boolean initialized = false;
  private static String windowClosed = null;
  private static boolean stopped = true;
  private void initialize() {
    String[] args = new String[5];
    String userFile = getParameter("file"); //"applet";//+username;
    args[0]=userFile;
    this.main(args);
  }
  public void init() {
    System.out.println("Init rule manager.");
    //super.init();
    setFont(new Font("dialog", Font.BOLD, 12));
    setLayout(new GridBagLayout());
    setBackground(SystemColor.control);
    GridBagConstraints gbc = new GridBagConstraints();
    gbc.fill = GridBagConstraints.BOTH;
    //String[] args = new String[5];

    baseUrl = getCodeBase();

//    try{
//    baseUrl = new URL("http://localhost:20801/msag/rules/rules/"); //todo only for degub - comment
//    }catch (MalformedURLException e){
//
//    }
      
    username = getParameter("username");
    password = getParameter("password");
    jEditHome =getParameter("homedir");
    //String userFile = getParameter("file"); //"applet";//+username;
    //args[0]=userFile;
    //System.out.println("userfile= "+userfile);
    String protocol = baseUrl.getProtocol();
    String host = baseUrl.getHost();
    int port = baseUrl.getPort();
    String file = baseUrl.getFile();
    String path = baseUrl.getPath();
    try {
      servletUrl = new URL(baseUrl, getParameter("servleturl"));
      System.out.println("jEdit 131 servletUrl: "+servletUrl);
    } catch (MalformedURLException e) { e.printStackTrace();
    }
    ping_port = Integer.parseInt(getParameter("ping_port"));
    ping_timeout = Integer.parseInt(getParameter("ping_timeout"));
    System.out.println("baseUrl= " + baseUrl.toString());
    System.out.println("servletUrl= " + servletUrl.toString());
    System.out.println("ping_port= " + ping_port);
    System.out.println("ping_timeout= " + ping_timeout);
    initSystemProperties();
    VFSManager.init();
    if (!getBooleanProperty("debug"))  {
      System.setOut(new PrintStream(new OutputStream(){
        public void write(int b) throws IOException {
            //doing nothing!!!;
        }
      }));
      System.setErr(System.out);
    }       
    //this.main(args);
    isNotReload=true;
  }

  static boolean  isNotReload = false;

  public void start()
  {
    System.out.println("Starting rule editor...");
    super.start();
    String[] args = new String[5];
    args[0]=getParameter("file");
    System.out.println("Parameter file="+args[0]);

    // Debug todo debug -comment
    //openRule("1////SMPP");
  }
    
  public String openRule(final String userFile)
   {
     stopped = false;
     String validationResult = validate(userFile, false);
     if (validationResult!=null) {
       setWindowClosed(editRule);
       return validationResult;
     }       
     if(!initialized) {
       initialize();
       initialized = true;
     }
     System.out.println("openRule... "+userFile);
     super.start();
     String[] args = new String[5];
     args[0]=userFile;
     setBooleanProperty("newRule",false);
      this.main2(args);
     isNotReload=true;
     return null;
     //}
   }

  public String newRule(final String userFile)
    {
      stopped = false;
      String validationResult = validate(userFile, true);
      if (validationResult!=null) {
        setWindowClosed(addRule);
        return validationResult;
      }
      if(!initialized) {
        initialize();
        initialized = true;
      }
      System.out.println("newRule... ");
      super.start();
      String[] args = new String[5];
      args[0]=userFile;
      setBooleanProperty("newRule",true);
      this.main2(args);
      isNotReload=true;
      //}
      return null;
    }

  public void stop()
  {
    System.out.println("Stopping...");
   //  isNotReload=true;
    //initialized = false;
    super.stop();
  }

  private String validate(String userFile, boolean newRule) {
    System.out.println("Begin validation");
    long begin = System.currentTimeMillis();
    boolean isLocked = false;
    boolean isExist = false;
    Object ruleState = getObject(userFile, getRuleStateAndLock);
    try {
       Method locked = ruleState.getClass().getMethod("getLocked",new Class[]{});
       isLocked = ((Boolean)locked.invoke(ruleState,new Object[]{})).booleanValue();
       Method exist = ruleState.getClass().getMethod("getExists",new Class[]{});
       isExist = ((Boolean)exist.invoke(ruleState,new Object[]{})).booleanValue();
      System.out.println("!!!!Rule isLocked = " + isLocked);
      if (isLocked) {
        return (String)ruleState.getClass().getField("lockedError").get(ruleState) ;
      }
      if (newRule) {
        //somebody has created it before you press "Add" button
        if (isExist) {
          //rule was locked by command, so we need to unlock it first!
          unlockRule(userFile);
          return (String)ruleState.getClass().getField("existError").get(ruleState) ;
        }
      } else {
        //somebody has deleted it before you press "Edit" button
        if (!isExist) {
          //rule was locked by command, so we need to unlock it first!
          unlockRule(userFile);
          return (String)ruleState.getClass().getField("notExistError").get(ruleState) ;
        }
      }
    } catch (Exception e) {
      e.printStackTrace();
    }
    long end = System.currentTimeMillis();
    long interval = (end - begin)/1000;
    System.out.println("Validation time: "+interval);
    return null;
  }

  public static boolean isDestroyed()
  {
    return destroyed;
  }

  public boolean isStopped() {
    windowClosed=null;
    return stopped;
  }

  public void destroy()
  {
    System.out.println("Destroying...");
    //unlock rules of this instance of jedit!!!
    unlockAllRules();
    stopped = true;
    destroyed=true;
    startupDone=false;
    initialized = false;
    DockableWindowFactory.getInstance().clear();
    KillRing.getInstance().clear();
     //{{{ Clear static variables in plugins that must be cleared at destroy
    if (jars!=null)
     for (int i = 0; i < jars.size(); i++) {
       PluginJAR jar=(PluginJAR) jars.elementAt(i);
       jar.clear();
     } //}}}
    //VFSManager.clear();
    if (bufferHash!=null) bufferHash.clear();
    bufferCount=0;
    buffersFirst=null;
    buffersLast=null;
    viewCount=0;
    viewsFirst=null;
    viewsLast=null;
    activeView=null;
    System.out.println("exit zakomentirovan!!!...");
   //  jEdit.exit(activeView,true);
  }
  //{{{ main() method
  /**
   * The main method of the jEdit application.
   * This should never be invoked directly.
   *
   * @param args The command line arguments
   */
  public void main(String[] args)
  {
    //{{{ Check for Java 1.4 or later
    /* String javaVersion = System.getProperty("java.version");
      if(javaVersion.compareTo("1.4") < 0)
      {
        System.err.println("You are running Java version "
          + javaVersion + ".");
        System.err.println("XMLEditApplet requires Java 1.4 or later.");
        System.exit(1);
      } //}}}
  */

    // later on we need to know if certain code is called from
    // the main thread
    mainThread = Thread.currentThread();

    //lc = new LiveConnect(baseUrl.getHost(),ping_port);
    //lcTimer = new Timer(ping_timeout,lc);

    lctt = new LiveConnectTimerTask(baseUrl.getHost(),ping_port);
    lcTimer = new java.util.Timer(true);
    lcTimer.schedule(lctt, 0, ping_timeout);  

    settingsDirectory = null; // ".jedit";

    // MacOS users expect the app to keep running after all windows
    // are closed
    //denied in applet: background = OperatingSystem.isMacOS();

    //{{{ Parse command line
    boolean endOpts = false;
    int level = Log.WARNING;
    boolean restore = true;
    boolean newView = true;
    boolean newPlainView = false;
    boolean gui = true; // open initial view?
    boolean loadPlugins = true;
    boolean runStartupScripts = true;
    boolean quit = false;
    boolean wait = false;
    //denied in applet: String userDir = System.getProperty("user.dir");

    // script to run
    String scriptFile = null;
    String userFile="";
    for (int i = 0; i < args.length; i++) {
      String arg = args[i];
      userFile=args[0];
      if (arg == null)
        continue;
      else if (arg.length() == 0)
        args[i] = null;
      else if (arg.startsWith("-") && !endOpts) {
        if (arg.equals("--"))
          endOpts = true;
        else if (arg.equals("-usage")) {
          version();
          System.err.println();
          usage();
          super.stop();//System.exit(1);
        }
        else if (arg.equals("-version")) {
          version();
          super.stop(); //System.exit(1);
        }
        else if (arg.startsWith("-log=")) {
          try {
            level = Integer.parseInt(arg.substring("-log=".length()));
          } catch (NumberFormatException nf) {
            System.err.println("Malformed option: " + arg);
          }
        }
        else if (arg.equals("-nosettings"))
          settingsDirectory = null;
        else if (arg.startsWith("-settings="))
          settingsDirectory = arg.substring(10);
        else if (arg.equals("-gui"))
          gui = true;
        else if (arg.equals("-nogui"))
          gui = false;
        else if (arg.equals("-newview"))
          newView = true;
        else if (arg.equals("-newplainview"))
          newPlainView = true;
        else if (arg.equals("-reuseview"))
          newPlainView = newView = false;
        else if (arg.equals("-restore"))
          restore = true;
        else if (arg.equals("-norestore"))
          restore = false;
        else if (arg.equals("-plugins"))
          loadPlugins = true;
        else if (arg.equals("-noplugins"))
          loadPlugins = false;
        else if (arg.equals("-startupscripts"))
          runStartupScripts = true;
        else if (arg.equals("-nostartupscripts"))
          runStartupScripts = false;
        else if (arg.startsWith("-run="))
          scriptFile = arg.substring(5);
        else if (arg.equals("-wait"))
          wait = true;
        else {
          System.err.println("Unknown option: "
                  + arg);
          usage();
          stop();//System.exit(1);
        }
        args[i] = null;
      }
    } //}}}
    separatorChar=StringGet("file",SeparatorChar).charAt(0);  // "\\ or /"
    //{{{ We need these initializations very early on
    if (settingsDirectory != null) {
     settingsDirectory = MiscUtilities.constructPath(jEditHome, settingsDirectory);
     settingsDirectory = MiscUtilities.resolveSymlinks(settingsDirectory);
    }

    // don't show splash screen if there is a file named
    // 'nosplash' in the settings directory
    // if(!new File(settingsDirectory,"nosplash").exists())
    //  GUIUtilities.showSplashScreen();

    //{{{ Initialize settings directory
    //{{{ Get things rolling

    initMisc();
    //initSystemProperties();

    //GUIUtilities.advanceSplashProgress();
    GUIUtilities.init();
    BeanShell.init();
    if (jEditHome != null)
      initSiteProperties();
   // initUserProperties();
    //}}}
    //{{{ Do more stuff
    initPLAF();
    //VFSManager.init();

    initResources();
    SearchAndReplace.load();
    GUIUtilities.advanceSplashProgress();
    if (loadPlugins)
      initPlugins();

   // HistoryModel.loadHistory();
   // BufferHistory.load();
   // KillRing.getInstance().load();
    propertiesChanged();

    GUIUtilities.advanceSplashProgress();

    // Buffer sort
    sortBuffers = getBooleanProperty("sortBuffers");
    sortByName = getBooleanProperty("sortByName");

    reloadModes();

    GUIUtilities.advanceSplashProgress();
    //}}}

    //{{{ Activate plugins that must be activated at startup
    for (int i = 0; i < jars.size(); i++) {
      ((PluginJAR) jars.elementAt(i)).activatePluginIfNecessary();
    } //}}}

    //{{{ Load macros and run startup scripts, after plugins and settings are loaded
    Macros.loadMacros();
    Macros.getMacroActionSet().initKeyBindings();

    if (runStartupScripts && jEditHome != null) {
      String path = MiscUtilities.constructPath(jEditHome, "startup");
      System.out.println("jEdit line 531 path= " + path);
      if (BoolGet(path, Exists))// if(file.exists())
        runStartupScripts(path);
    }
    if (runStartupScripts && settingsDirectory != null) {
      String path = MiscUtilities.constructPath(settingsDirectory, "startup");
      if (!BoolGet(path, Exists))//if(!file.exists())
        BoolGet(path, MkDirs); //file.mkdirs();
      else
        runStartupScripts(path);
    } //}}}
    if (jEdit.getBooleanProperty("xml.disableRootElement")) {
         String Root=StringGet("",RootElement);
        System.out.println("RootElement= "+Root);
        setProperty("RootElement",Root);
    }

    //{{{ Run script specified with -run= parameter
    //todo userDir changed
    //String transport=getParameter("transport");

    // userfile = MiscUtilities.constructPath(transport,userfile);
/*    String userDir = MiscUtilities.constructPath(jEditHome, jEdit.username);//jEditHome+"\\"+jEdit.username;//null;
    if (!BoolGet(userDir, Exists)) BoolGet(userDir, MkDir);
    if (scriptFile != null) {
      scriptFile = MiscUtilities.constructPath(userDir, scriptFile);
      try {  BeanShell.getNameSpace().setVariable("args", args);
      } catch (UtilEvalError e) {
        Log.log(Log.ERROR, jEdit.class, e);
      }
      BeanShell.runScript(null, scriptFile, null, false);
    } //}}}
    GUIUtilities.advanceSplashProgress();

    String transport=StringGet(userFile,Transport);
    userDir = MiscUtilities.constructPath(userDir, transport);//jEditHome+"\\"+jEdit.username;//null;
    if (!BoolGet(userDir, Exists)) BoolGet(userDir, MkDir);
    System.out.println("userDir+transport= "+userDir);
    userFile="rule_"+userFile+".xml";
    // Open files, create the view and hide the splash screen.
    finishStartup(gui, restore,userDir, args,userFile);
 */
  } //}}}

   public void main2(String[] args)
  {
    //{{{ Check for Java 1.4 or later
    /* String javaVersion = System.getProperty("java.version");
      if(javaVersion.compareTo("1.4") < 0)
      {
        System.err.println("You are running Java version "
          + javaVersion + ".");
        System.err.println("XMLEditApplet requires Java 1.4 or later.");
        System.exit(1);
      } //}}}
  */

    // later on we need to know if certain code is called from
    // the main thread
   // mainThread = Thread.currentThread();

    settingsDirectory = null; // ".jedit";

    // MacOS users expect the app to keep running after all windows
    // are closed
    //denied in applet: background = OperatingSystem.isMacOS();

    //{{{ Parse command line
    boolean endOpts = false;
    int level = Log.WARNING;
    boolean restore = true;
    boolean newView = true;
    boolean newPlainView = false;
    boolean gui = true; // open initial view?
    boolean loadPlugins = true;
    boolean runStartupScripts = true;
    boolean quit = false;
    boolean wait = false;
    //denied in applet: String userDir = System.getProperty("user.dir");

    // script to run
    String scriptFile = null;
    String userFile="";
    userFile=args[0];
    //lc.addRuleId(userFile);
    lctt.addRuleId(userFile);  
    //{{{ Run script specified with -run= parameter
    //todo userDir changed
    //String transport=getParameter("transport");

    // userfile = MiscUtilities.constructPath(transport,userfile);
    String userDir = MiscUtilities.constructPath(jEditHome, jEdit.username);//jEditHome+"\\"+jEdit.username;//null;
    if (!BoolGet(userDir, Exists)) BoolGet(userDir, MkDir);
    if (scriptFile != null) {
      scriptFile = MiscUtilities.constructPath(userDir, scriptFile);
      try {  BeanShell.getNameSpace().setVariable("args", args);
      } catch (UtilEvalError e) {
        Log.log(Log.ERROR, jEdit.class, e);
      }
      BeanShell.runScript(null, scriptFile, null, false);
    } //}}}
    GUIUtilities.advanceSplashProgress();

    String transport = StringGet(userFile,Transport);
    userDir = MiscUtilities.constructPath(userDir, transport);//jEditHome+"\\"+jEdit.username;//null;
    if (!BoolGet(userDir, Exists)) BoolGet(userDir, MkDir);
    System.out.println("userDir+transport= "+userDir);
    if (!jEdit.getBooleanProperty("bufferWorkWithId")) userFile="rule_"+userFile+".xml";
    // Open files, create the view and hide the splash screen.
    finishStartupNext(gui, restore,userDir, args,userFile);

  } //}}}
  public static final String IdToFileName(final String Id)
    {
      String IdPrefix=getProperty("Id.prefix");
      String IdSuffix=getProperty("Id.suffix");
      return IdPrefix+Id+IdSuffix;
    } //}}}

  //{{{ Property methods

  //{{{ getProperties() method
  /**
   * Returns the properties object which contains all known
   * jEdit properties. Note that as of jEdit 4.2pre10, this returns a
   * new collection, not the existing properties instance.
   *
   * @since jEdit 3.1pre4
   */
  public static final Properties getProperties()
  {
    return propMgr.getProperties();
  } //}}}

  //{{{ getProperty() method
  /**
   * Fetches a property, returning null if it's not defined.
   *
   * @param name The property
   */
  public static final String getProperty(String name)
  {
    return propMgr.getProperty(name);
  } //}}}

  //{{{ getProperty() method
  /**
   * Fetches a property, returning the default value if it's not
   * defined.
   *
   * @param name The property
   * @param def  The default value
   */
  public static final String getProperty(String name, String def)
  {
    String value = propMgr.getProperty(name);
    if (value == null)
      return def;
    else
      return value;
  } //}}}

  //{{{ getProperty() method
  /**
   * Returns the property with the specified name.<p>
   * <p/>
   * The elements of the <code>args</code> array are substituted
   * into the value of the property in place of strings of the
   * form <code>{<i>n</i>}</code>, where <code><i>n</i></code> is an index
   * in the array.<p>
   * <p/>
   * You can find out more about this feature by reading the
   * documentation for the <code>format</code> method of the
   * <code>java.text.MessageFormat</code> class.
   *
   * @param name The property
   * @param args The positional parameters
   */
  public static final String getProperty(String name, Object[] args)
  {
    if (name == null)
      return null;
    if (args == null)
      return getProperty(name);
    else {
      String value = getProperty(name);
      if (value == null)
        return null;
      else
        return MessageFormat.format(value, args);
    }
  } //}}}

  //{{{ getBooleanProperty() method
  /**
   * Returns the value of a boolean property.
   *
   * @param name The property
   */
  public static final boolean getBooleanProperty(String name)
  {
    return getBooleanProperty(name, false);
  } //}}}

  //{{{ getBooleanProperty() method
  /**
   * Returns the value of a boolean property.
   *
   * @param name The property
   * @param def  The default value
   */
  public static final boolean getBooleanProperty(String name, boolean def)
  {
    String value = getProperty(name);
    if (value == null)
      return def;
    else if (value.equals("true") || value.equals("yes")
            || value.equals("on"))
      return true;
    else if (value.equals("false") || value.equals("no")
            || value.equals("off"))
      return false;
    else
      return def;
  } //}}}

  //{{{ getIntegerProperty() method
  /**
   * Returns the value of an integer property.
   *
   * @param name The property
   * @param def  The default value
   * @since jEdit 4.0pre1
   */
  public static final int getIntegerProperty(String name, int def)
  {
    String value = getProperty(name);
    if (value == null)
      return def;
    else {
      try {
        return Integer.parseInt(value.trim());
      } catch (NumberFormatException nf) {
        return def;
      }
    }
  } //}}}

  //{{{ getDoubleProperty() method
  public static double getDoubleProperty(String name, double def)
  {
    String value = getProperty(name);
    if (value == null)
      return def;
    else {
      try {
        return Double.parseDouble(value.trim());
      } catch (NumberFormatException nf) {
        return def;
      }
    }
  }
  //}}}

  //{{{ getFontProperty() method
  /**
   * Returns the value of a font property. The family is stored
   * in the <code><i>name</i></code> property, the font size is stored
   * in the <code><i>name</i>size</code> property, and the font style is
   * stored in <code><i>name</i>style</code>. For example, if
   * <code><i>name</i></code> is <code>view.gutter.font</code>, the
   * properties will be named <code>view.gutter.font</code>,
   * <code>view.gutter.fontsize</code>, and
   * <code>view.gutter.fontstyle</code>.
   *
   * @param name The property
   * @since jEdit 4.0pre1
   */
  public static final Font getFontProperty(String name)
  {
    return getFontProperty(name, null);
  } //}}}

  //{{{ getFontProperty() method
  /**
   * Returns the value of a font property. The family is stored
   * in the <code><i>name</i></code> property, the font size is stored
   * in the <code><i>name</i>size</code> property, and the font style is
   * stored in <code><i>name</i>style</code>. For example, if
   * <code><i>name</i></code> is <code>view.gutter.font</code>, the
   * properties will be named <code>view.gutter.font</code>,
   * <code>view.gutter.fontsize</code>, and
   * <code>view.gutter.fontstyle</code>.
   *
   * @param name The property
   * @param def  The default value
   * @since jEdit 4.0pre1
   */
  public static final Font getFontProperty(String name, Font def)
  {
    String family = getProperty(name);
    String sizeString = getProperty(name + "size");
    String styleString = getProperty(name + "style");

    if (family == null || sizeString == null || styleString == null)
      return def;
    else {
      int size, style;

      try {
        size = Integer.parseInt(sizeString);
      } catch (NumberFormatException nf) {
        return def;
      }

      try {
        style = Integer.parseInt(styleString);
      } catch (NumberFormatException nf) {
        return def;
      }

      return new Font(family, style, size);
    }
  } //}}}

  //{{{ getColorProperty() method
  /**
   * Returns the value of a color property.
   *
   * @param name The property name
   * @since jEdit 4.0pre1
   */
  public static Color getColorProperty(String name)
  {
    return getColorProperty(name, Color.black);
  } //}}}

  //{{{ getColorProperty() method
  /**
   * Returns the value of a color property.
   *
   * @param name The property name
   * @param def  The default value
   * @since jEdit 4.0pre1
   */
  public static Color getColorProperty(String name, Color def)
  {
    String value = getProperty(name);
    if (value == null)
      return def;
    else
      return GUIUtilities.parseColor(value, def);
  } //}}}

  //{{{ setColorProperty() method
  /**
   * Sets the value of a color property.
   *
   * @param name  The property name
   * @param value The value
   * @since jEdit 4.0pre1
   */
  public static void setColorProperty(String name, Color value)
  {
    setProperty(name, GUIUtilities.getColorHexString(value));
  } //}}}

  //{{{ setProperty() method
  /**
   * Sets a property to a new value.
   *
   * @param name  The property
   * @param value The new value
   */
  public static final void setProperty(String name, String value)
  {
      //System.out.println("jEdit:setProperty() name=\""+name+"\" value=\""+value+"\".");
      propMgr.setProperty(name, value);
  } //}}}

  //{{{ setTemporaryProperty() method
  /**
   * Sets a property to a new value. Properties set using this
   * method are not saved to the user properties list.
   *
   * @param name  The property
   * @param value The new value
   * @since jEdit 2.3final
   */
  public static final void setTemporaryProperty(String name, String value)
  {
    propMgr.setTemporaryProperty(name, value);
  } //}}}

  //{{{ setBooleanProperty() method
  /**
   * Sets a boolean property.
   *
   * @param name  The property
   * @param value The value
   */
  public static final void setBooleanProperty(String name, boolean value)
  {
    setProperty(name, value ? "true" : "false");
  } //}}}

  //{{{ setIntegerProperty() method
  /**
   * Sets the value of an integer property.
   *
   * @param name  The property
   * @param value The value
   * @since jEdit 4.0pre1
   */
  public static final void setIntegerProperty(String name, int value)
  {
     //System.out.println("JEdit:setIntegerProperty() name=\""+name+"\" value=\""+value+"\".");
     setProperty(name, String.valueOf(value));
  } //}}}

  //{{{ setDoubleProperty() method
  public static final void setDoubleProperty(String name, double value)
  {
    setProperty(name, String.valueOf(value));
  }
  //}}}

  //{{{ setFontProperty() method
  /**
   * Sets the value of a font property. The family is stored
   * in the <code><i>name</i></code> property, the font size is stored
   * in the <code><i>name</i>size</code> property, and the font style is
   * stored in <code><i>name</i>style</code>. For example, if
   * <code><i>name</i></code> is <code>view.gutter.font</code>, the
   * properties will be named <code>view.gutter.font</code>,
   * <code>view.gutter.fontsize</code>, and
   * <code>view.gutter.fontstyle</code>.
   *
   * @param name  The property
   * @param value The value
   * @since jEdit 4.0pre1
   */
  public static final void setFontProperty(String name, Font value)
  {
    setProperty(name, value.getFamily());
    setIntegerProperty(name + "size", value.getSize());
    setIntegerProperty(name + "style", value.getStyle());
  } //}}}

  //{{{ unsetProperty() method
  /**
   * Unsets (clears) a property.
   *
   * @param name The property
   */
  public static final void unsetProperty(String name)
  {
    propMgr.unsetProperty(name);
  } //}}}

  //{{{ resetProperty() method
  /**
   * Resets a property to its default value.
   *
   * @param name The property
   * @since jEdit 2.5pre3
   */
  public static final void resetProperty(String name)
  {
    propMgr.resetProperty(name);
  } //}}}

  //{{{ propertiesChanged() method
  /**
   * Reloads various settings from the properties.
   */
  public static void propertiesChanged()
  {
      System.out.println("jEdit:propertiesChanged()");
      initKeyBindings();



    saveCaret = getBooleanProperty("saveCaret");

    UIDefaults defaults = UIManager.getDefaults();

    // give all text areas the same font
    Font font = getFontProperty("view.font");

    //defaults.put("TextField.font",font);
    defaults.put("TextArea.font", font);
    defaults.put("TextPane.font", font);

    // Enable/Disable tooltips
    ToolTipManager.sharedInstance().setEnabled(jEdit.getBooleanProperty("showTooltips"));

    // initProxy();

    // we do this here instead of adding buffers to the bus.
    Buffer buffer = buffersFirst;
    while (buffer != null) {
      buffer.resetCachedProperties();
      buffer.propertiesChanged();
      buffer = buffer.next;
    }

    HistoryModel.propertiesChanged();
    KillRing.getInstance().propertiesChanged();

    EditBus.send(new PropertiesChanged(null));
  } //}}}

  //}}}

  //{{{ Plugin management methods

  //{{{ getNotLoadedPluginJARs() method
  /**
   * Returns a list of plugin JARs that are not currently loaded
   * by examining the user and system plugin directories.
   * @since jEdit 3.2pre1
   */
/* public static String[] getNotLoadedPluginJARs()
 {
  Vector returnValue = new Vector();

  if(jEditHome != null)
  {
   String systemPluginDir = MiscUtilities
    .constructPath(jEditHome,"jars");

   String[] list = new File(systemPluginDir).list();
   if(list != null)
    getNotLoadedPluginJARs(returnValue,systemPluginDir,list);
  }

  if(settingsDirectory != null)
  {
   String userPluginDir = MiscUtilities
    .constructPath(settingsDirectory,"jars");
   String[] list = new File(userPluginDir).list();
   if(list != null)
   {
    getNotLoadedPluginJARs(returnValue,
     userPluginDir,list);
   }
  }

  String[] _returnValue = new String[returnValue.size()];
  returnValue.copyInto(_returnValue);
  return _returnValue;
 } //}}}
  */
  //{{{ getPlugin() method
  /**
   * Returns the plugin with the specified class name.
   */
  public static EditPlugin getPlugin(String name)
  {
    return getPlugin(name, false);
  } //}}}

  //{{{ getPlugin(String, boolean) method
  /**
   * Returns the plugin with the specified class name. If
   * <code>loadIfNecessary</code> is true, the plugin will be activated in
   * case it has not yet been started.
   *
   * @since jEdit 4.2pre4
   */
  public static EditPlugin getPlugin(String name, boolean loadIfNecessary)
  {
    EditPlugin[] plugins = getPlugins();
    EditPlugin plugin = null;
    for (int i = 0; i < plugins.length; i++) {
      if (plugins[i].getClassName().equals(name))
        plugin = plugins[i];
      if (loadIfNecessary) {
        if (plugin instanceof EditPlugin.Deferred) {
          plugin.getPluginJAR().activatePlugin();
          plugin = plugin.getPluginJAR().getPlugin();
          break;
        }
      }
    }

    return plugin;
  } //}}}

  //{{{ getPlugins() method
  /**
   * Returns an array of installed plugins.
   */
  public static EditPlugin[] getPlugins()
  {
    Vector vector = new Vector();
    for (int i = 0; i < jars.size(); i++) {
      EditPlugin plugin = ((PluginJAR) jars.elementAt(i))
              .getPlugin();
      if (plugin != null)
        vector.add(plugin);
    }

    EditPlugin[] array = new EditPlugin[vector.size()];
    vector.copyInto(array);
    return array;
  } //}}}

  //{{{ getPluginJARs() method
  /**
   * Returns an array of installed plugins.
   *
   * @since jEdit 4.2pre1
   */
  public static PluginJAR[] getPluginJARs()
  {
    PluginJAR[] array = new PluginJAR[jars.size()];
    jars.copyInto(array);
    return array;
  } //}}}

  //{{{ getPluginJAR() method
  /**
   * Returns the JAR with the specified path name.
   *
   * @param className The Class name
   * @since jEdit 4.2pre1
   */
  public static PluginJAR getPluginJAR(String className)
  {
    for (int i = 0; i < jars.size(); i++) {
      PluginJAR jar = (PluginJAR) jars.elementAt(i);
      if (jar.getClassName().equals(className))
        return jar;
    }

    return null;
  } //}}}

  //{{{ addPluginJAR() method
  /**
   * Loads the plugin JAR with the specified path. Some notes about this
   * method:
   * <p/>
   * <ul>
   * <li>Calling this at a time other than jEdit startup can have
   * unpredictable results if the plugin has not been updated for the
   * jEdit 4.2 plugin API.
   * <li>You must make sure yourself the plugin is not already loaded.
   * <li>After loading, you just make sure all the plugin's dependencies
   * are satisified before activating the plugin, using the
   * {@link PluginJAR#checkDependencies()} method.
   * </ul>
   *
   * @param clas The JAR class name
   * @since jEdit 4.2pre1
   */
  public static void addPluginJAR(Class clas)
  {
    PluginJAR jar = new PluginJAR(clas);
    jars.addElement(jar);
    jar.init(clas);
    EditBus.send(new PluginUpdate(jar, PluginUpdate.LOADED, false));
    if (!isMainThread()) {
      EditBus.send(new DynamicMenuChanged("plugins"));
      initKeyBindings();
    }
  } //}}}

  //{{{ addPluginJARsFromDirectory() method
  /**
   * Loads all plugins in a jar.
   *
   * @since jEdit 4.2pre1
   */
  private static void addPluginJARPackages()
  {
    String[] plugins = {"errorlist.ErrorListPlugin","sidekick.SideKickPlugin","gatchan.highlight.HighlightPlugin","xml.XmlPlugin"};//,"SideCick","HighLight","XML","TextTools"};//StringsGet(directory, ListFiles);//file.list();
    Class clas = null;
    for (int i = 0; i < plugins.length; i++) {
      String plugin = plugins[i];
      try {  clas=Class.forName(plugin);
      } catch (ClassNotFoundException e) {
        e.printStackTrace(); }
      addPluginJAR(clas);
    }
  } //}}}

  //{{{ removePluginJAR() method
  /**
   * Unloads the given plugin JAR with the specified path. Note that
   * calling this at a time other than jEdit shutdown can have
   * unpredictable results if the plugin has not been updated for the
   * jEdit 4.2 plugin API.
   *
   * @param jar  The <code>PluginJAR</code> instance
   * @param exit Set to true if jEdit is exiting; enables some
   *             shortcuts so the editor can close faster.
   * @since jEdit 4.2pre1
   */
  public static void removePluginJAR(PluginJAR jar, boolean exit)
  {
    if (exit) {
      jar.uninit(true);
    }
    else {
      jar.uninit(false);
      jars.removeElement(jar);
      initKeyBindings();
    }

    EditBus.send(new PluginUpdate(jar, PluginUpdate.UNLOADED, exit));
    if (!isMainThread() && !exit)
      EditBus.send(new DynamicMenuChanged("plugins"));
  } //}}}

  //}}}

  //{{{ Action methods

  //{{{ getActionContext() method
  /**
   * Returns the action context used to store editor actions.
   *
   * @since jEdit 4.2pre1
   */
  public static ActionContext getActionContext()
  {
    return actionContext;
  } //}}}

  //{{{ addActionSet() method
  /**
   * Adds a new action set to jEdit's list. Plugins probably won't
   * need to call this method.
   *
   * @since jEdit 4.0pre1
   */
  public static void addActionSet(ActionSet actionSet)
  {
    actionContext.addActionSet(actionSet);
  } //}}}

  //{{{ removeActionSet() method
  /**
   * Removes an action set from jEdit's list. Plugins probably won't
   * need to call this method.
   *
   * @since jEdit 4.2pre1
   */
  public static void removeActionSet(ActionSet actionSet)
  {
    actionContext.removeActionSet(actionSet);
  } //}}}

  //{{{ getBuiltInActionSet() method
  /**
   * Returns the set of commands built into jEdit.
   *
   * @since jEdit 4.2pre1
   */
  public static ActionSet getBuiltInActionSet()
  {
    return builtInActionSet;
  } //}}}

  //{{{ getActionSets() method
  /**
   * Returns all registered action sets.
   *
   * @since jEdit 4.0pre1
   */
  public static ActionSet[] getActionSets()
  {
    return actionContext.getActionSets();
  } //}}}

  //{{{ getAction() method
  /**
   * Returns the specified action.
   *
   * @param name The action name
   */
  public static EditAction getAction(String name)
  {
    return actionContext.getAction(name);
  } //}}}

  //{{{ getActionSetForAction() method
  /**
   * Returns the action set that contains the specified action.
   *
   * @param action The action
   * @since jEdit 4.2pre1
   */
  public static ActionSet getActionSetForAction(String action)
  {
    return actionContext.getActionSetForAction(action);
  } //}}}

  //{{{ getActionSetForAction() method
  /**
   * @deprecated Use the form that takes a String instead
   */
  public static ActionSet getActionSetForAction(EditAction action)
  {
    return actionContext.getActionSetForAction(action.getName());
  } //}}}

  //{{{ getActions() method
  /**
   * @deprecated Call getActionNames() instead
   */
  public static EditAction[] getActions()
  {
    String[] names = actionContext.getActionNames();
    EditAction[] actions = new EditAction[names.length];
    for (int i = 0; i < actions.length; i++) {
      actions[i] = actionContext.getAction(names[i]);
      if (actions[i] == null)
        Log.log(Log.ERROR, jEdit.class, "wtf: " + names[i]);
    }
    return actions;
  } //}}}

  //{{{ getActionNames() method
  /**
   * Returns all registered action names.
   */
  public static String[] getActionNames()
  {
    return actionContext.getActionNames();
  } //}}}

  //}}}

  //{{{ Edit mode methods

  //{{{ reloadModes() method
  /**
   * Reloads all edit modes.
   *
   * @since jEdit 3.2pre2
   */
  public static void reloadModes()
  {
    /* Try to guess the eventual size to avoid unnecessary
     * copying */
    modes = new Vector(50);
    //{{{ Load the global catalog
    if (jEditHome == null)
      loadModeCatalog("/modes/catalog", true);
    else {
      String path=MiscUtilities.constructPath(jEditHome,
              "modes", "catalog");
      System.out.println("jEdit reloadModes before LoadModesCatalog path= "+path);
      loadModeCatalog(path, false);
    } //}}}

    //{{{ Load user catalog
    if (settingsDirectory != null) {
      String userModeName = MiscUtilities.constructPath(settingsDirectory, "modes");
      File userModeDir = new File(userModeName);
      if (!BoolGet(userModeName, Exists))    //userModeDir.exists()
        BoolGet(userModeName, MkDirs);//userModeDir.mkdirs();

      String userCatalogName = MiscUtilities.constructPath(settingsDirectory, "modes", "catalog");
      File userCatalog = new File(userCatalogName);
      if (!userCatalog.exists()) {
        // create dummy catalog
        FileWriter out = null;
        try {
          out = new FileWriter(userCatalog);
          out.write(jEdit.getProperty("defaultCatalog"));
          out.close();
        } catch (IOException io) {
          Log.log(Log.ERROR, jEdit.class, io);
        } finally {
          try {
            if (out != null)
              out.close();
          } catch (IOException e) {
          }
        }
      }

      loadModeCatalog(userCatalog.getPath(), false);
    } //}}}

    Buffer buffer = buffersFirst;
    System.out.println("jEdit.main.reloadModes buffersFirst= "+buffer);
    if (!isNotReload) {
    while (buffer != null) {
      // This reloads the token marker and sends a message
      // which causes edit panes to repaint their text areas
 //     System.out.println("before buffer.setMode() !!!!!!!!!!");
      buffer.setMode();

      buffer = buffer.next;
    }
     isNotReload=false;
    }
  } //}}}

  //{{{ getMode() method
  /**
   * Returns the edit mode with the specified name.
   *
   * @param name The edit mode
   */
  public static Mode getMode(String name)
  {
    for (int i = 0; i < modes.size(); i++) {
      Mode mode = (Mode) modes.elementAt(i);
      if (mode.getName().equals(name))
        return mode;
    }
    return null;
  } //}}}

  //{{{ getModes() method
  /**
   * Returns an array of installed edit modes.
   */
  public static Mode[] getModes()
  {
    Mode[] array = new Mode[modes.size()];
    modes.copyInto(array);
    return array;
  } //}}}

  //}}}

  //{{{ Buffer creation methods

  //{{{ openFiles() method
  /**
   * Opens the file names specified in the argument array. This
   * handles +line and +marker arguments just like the command
   * line parser.
   *
   * @param parent The parent directory
   * @param args   The file names to open
   * @since jEdit 3.2pre4
   */
  public static Buffer openFiles(View view, String parent, String[] args)
  {
    Buffer retVal = null;
    Buffer lastBuffer = null;
    for (int i = 0; i < args.length; i++) {
      String arg = args[i];
      if (arg == null)
        continue;
      else if (arg.startsWith("+line:") || arg.startsWith("+marker:")) {
        if (lastBuffer != null)
          gotoMarker(view, lastBuffer, arg);
        continue;
      }

      lastBuffer = openFile(null, parent, arg, false, null);

      if (retVal == null && lastBuffer != null)
        retVal = lastBuffer;
    }

    if (view != null && retVal != null)
      view.setBuffer(retVal);

    return retVal;
  } //}}}

  //{{{ openFile() method
  /**
   * Opens a file. Note that as of jEdit 2.5pre1, this may return
   * null if the buffer could not be opened.
   *
   * @param view The view to open the file in
   * @param path The file path
   * @since jEdit 2.4pre1
   */
  public static Buffer openFile(View view, String path)
  {
    return openFile(view, null, path, false, new Hashtable());
  } //}}}

  //{{{ openFile() method
  /**
   * @deprecated The openFile() forms with the readOnly parameter
   *             should not be used. The readOnly prameter is no longer supported.
   */
  public static Buffer openFile(View view, String parent,
                                String path, boolean readOnly, boolean newFile)
  {
    return openFile(view, parent, path, newFile, new Hashtable());
  } //}}}

  //{{{ openFile() method
  /**
   * @deprecated The openFile() forms with the readOnly parameter
   *             should not be used. The readOnly prameter is no longer supported.
   */
  public static Buffer openFile(View view, String parent,
                                String path, boolean readOnly, boolean newFile,
                                Hashtable props)
  {
    return openFile(view, parent, path, newFile, props);
  } //}}}

  //{{{ openFile() method
  /**
   * Opens a file. This may return null if the buffer could not be
   * opened for some reason.
   *
   * @param view    The view to open the file in
   * @param parent  The parent directory of the file
   * @param path    The path name of the file
   * @param newFile True if the file should not be loaded from disk
   *                be prompted if it should be reloaded
   * @param props   Buffer-local properties to set in the buffer
   * @since jEdit 3.2pre10
   */
  public static Buffer openFile(View view, String parent,
                                String path, boolean newFile, Hashtable props)
  {
    PerspectiveManager.setPerspectiveDirty(true);

    if (view != null && parent == null)
      parent = view.getBuffer().getDirectory();
    if (MiscUtilities.isURL(path)) {
      if (MiscUtilities.getProtocolOfURL(path).equals("file"))
        path = path.substring(5);
    }
    if (!jEdit.getBooleanProperty("bufferWorkWithId")) path = MiscUtilities.constructPath(parent, path);
    Buffer newBuffer;

    synchronized (bufferListLock) {
      Buffer buffer = getBuffer(path);
      if (buffer != null) {
        if (view != null)
          view.setBuffer(buffer);
        System.out.println("jEdit.openFile return Buffer line 1406");
        return buffer;
      }
      if (props == null)
        props = new Hashtable();

      BufferHistory.Entry entry = BufferHistory.getEntry(path);

      if (entry != null && saveCaret && props.get(Buffer.CARET) == null) {
        props.put(Buffer.CARET, new Integer(entry.caret));
        /* if(entry.selection != null)
        {
          // getSelection() converts from string to
          // Selection[]
          props.put(Buffer.SELECTION,entry.getSelection());
        } */
      }

      if (entry != null && props.get(Buffer.ENCODING) == null) {
        if (entry.encoding != null)
          props.put(Buffer.ENCODING, entry.encoding);
      }
      newBuffer = new Buffer(path, newFile, false, props);
      if (!newBuffer.load(view, false))
        return null;

      addBufferToList(newBuffer);
    }
    EditBus.send(new BufferUpdate(newBuffer, view, BufferUpdate.CREATED));

    if (view != null)
      view.setBuffer(newBuffer);
    return newBuffer;
  } //}}}

  //{{{ openTemporary() method
  /**
   * Opens a temporary buffer. A temporary buffer is like a normal
   * buffer, except that an event is not fired, the the buffer is
   * not added to the buffers list.
   *
   * @param view    The view to open the file in
   * @param parent  The parent directory of the file
   * @param path    The path name of the file
   * @param newFile True if the file should not be loaded from disk
   * @since jEdit 3.2pre10
   */
  public static Buffer openTemporary(View view, String parent,
                                     String path, boolean newFile)
  {
    if (view != null && parent == null)
      parent = view.getBuffer().getDirectory();
    if (MiscUtilities.isURL(path)) {
      if (MiscUtilities.getProtocolOfURL(path).equals("file"))
        path = path.substring(5);
    }
    if (!jEdit.getBooleanProperty("bufferWorkWithId")) path = MiscUtilities.constructPath(parent, path);
    synchronized (bufferListLock) {
      Buffer buffer = getBuffer(path);
      if (buffer != null)
        return buffer;

      buffer = new Buffer(path, newFile, true, new Hashtable());
      if (!buffer.load(view, false))
        return null;
      else
        return buffer;
    }
  } //}}}

  //{{{ commitTemporary() method
  /**
   * Adds a temporary buffer to the buffer list. This must be done
   * before allowing the user to interact with the buffer in any
   * way.
   *
   * @param buffer The buffer
   */
  public static void commitTemporary(Buffer buffer)
  {
    if (!buffer.isTemporary())
      return;

    PerspectiveManager.setPerspectiveDirty(true);

    addBufferToList(buffer);
    buffer.commitTemporary();

    // send full range of events to avoid breaking plugins
    EditBus.send(new BufferUpdate(buffer, null, BufferUpdate.CREATED));
    EditBus.send(new BufferUpdate(buffer, null, BufferUpdate.LOAD_STARTED));
    EditBus.send(new BufferUpdate(buffer, null, BufferUpdate.LOADED));
  } //}}}

  //{{{ newFile() method
  /**
   * Creates a new `untitled' file.
   *
   * @param view The view to create the file in
   */
  public static Buffer newFile(View view)
  {
    String path;

    if (view != null && view.getBuffer() != null) {
      path = view.getBuffer().getDirectory();
      VFS vfs = VFSManager.getVFSForPath(path);
      // don't want 'New File' to create a read only buffer
      // if current file is on SQL VFS or something
      if ((vfs.getCapabilities() & VFS.WRITE_CAP) == 0)
        path = System.getProperty("user.home");
    }
    else
      path = null;

    return newFile(view, path);
  } //}}}

  //{{{ newFile() method
  /**
   * Creates a new `untitled' file.
   *
   * @param view The view to create the file in
   * @param dir  The directory to create the file in
   * @since jEdit 3.1pre2
   */
  public static Buffer newFile(View view, String dir)
  {
    // If only one new file is open which is clean, just close
    // it, which will create an 'Untitled-1'
    if (dir != null
            && buffersFirst != null
            && buffersFirst == buffersLast
            && buffersFirst.isUntitled()
            && !buffersFirst.isDirty()) {
      closeBuffer(view, buffersFirst);
      // return the newly created 'untitled-1'
      return buffersFirst;
    }

    // Find the highest Untitled-n file
    int untitledCount = 0;
    Buffer buffer = buffersFirst;
    while (buffer != null) {
      if (buffer.getName().startsWith("Untitled-")) {
        try {
          untitledCount = Math.max(untitledCount,
                  Integer.parseInt(buffer.getName()
                  .substring(9)));
        } catch (NumberFormatException nf) {
        }
      }
      buffer = buffer.next;
    }

    return openFile(view, dir, "Untitled-" + (untitledCount + 1), true, null);
  } //}}}

  //}}}

  //{{{ Buffer management methods

  //{{{ closeBuffer() method
  /**
   * Closes a buffer. If there are unsaved changes, the user is
   * prompted if they should be saved first.
   *
   * @param view   The view
   * @param buffer The buffer
   * @return True if the buffer was really closed, false otherwise
   */
  public static boolean closeBuffer(View view, Buffer buffer)
  {
    // Wait for pending I/O requests
    if (buffer.isPerformingIO()) {
      VFSManager.waitForRequests();
      if (VFSManager.errorOccurred())
        return false;
    }

    if (buffer.isDirty()) {
      Object[] args = {buffer.getName()};
      int result = GUIUtilities.confirm(view, "closewindow", args,
              JOptionPane.YES_NO_OPTION,
              JOptionPane.WARNING_MESSAGE);
      if (result == JOptionPane.YES_OPTION) {
        //if (!buffer.save(view, null, true))
        _closeBuffer(view, buffer);
            return true;

        /*VFSManager.waitForRequests();
        if (buffer.getBooleanProperty(BufferIORequest
                .ERROR_OCCURRED)) {
          return false;
        }              */
      }
      else if (result == JOptionPane.NO_OPTION)
        return false;
    }

    _closeBuffer(view, buffer);

    return true;
  } //}}}

  //{{{ _closeBuffer() method
  /**
   * Closes the buffer, even if it has unsaved changes.
   *
   * @param view   The view, may be null
   * @param buffer The buffer
   * @throws NullPointerException if the buffer is null
   * @since jEdit 2.2pre1
   */
  public static void _closeBuffer(View view, Buffer buffer)
  {
    if (buffer.isClosed()) {
      // can happen if the user presses C+w twice real
      // quick and the buffer has unsaved changes
      return;
    }

    PerspectiveManager.setPerspectiveDirty(true);

    if (!buffer.isNewFile()) {
      if (view != null)
        view.getEditPane().saveCaretInfo();
      Integer _caret = (Integer) buffer.getProperty(Buffer.CARET);
      int caret = (_caret == null ? 0 : _caret.intValue());

      BufferHistory.setEntry(buffer.getPath(), caret,
              (Selection[]) buffer.getProperty(Buffer.SELECTION),
              buffer.getStringProperty(Buffer.ENCODING));
    }

    String path = buffer.getSymlinkPath();
    if ((VFSManager.getVFSForPath(path).getCapabilities()
            & VFS.CASE_INSENSITIVE_CAP) != 0) {
      path = path.toLowerCase();
    }
    bufferHash.remove(path);
    removeBufferFromList(buffer);
    buffer.close();
    DisplayManager.bufferClosed(buffer);
    //lc.removeRuleId(path);
    lctt.removeRuleId(path);
    unlockRule(path);
    System.out.println("jEdit._closeBuffer line 1654 EditBus.send(new BufferUpdate(buffer, view, BufferUpdate.CLOSED))");
    EditBus.send(new BufferUpdate(buffer, view, BufferUpdate.CLOSED));

    // Create a new file when the last is closed
 /*   if (buffersFirst == null && buffersLast == null)
      newFile(view);  */
  } //}}}

  //{{{ closeAllBuffers() method
  /**
   * Closes all open buffers.
   *
   * @param view The view
   */
  public static boolean closeAllBuffers(View view)
  {
    return closeAllBuffers(view, false);
  } //}}}

  //{{{ closeAllBuffers() method
  /**
   * Closes all open buffers.
   *
   * @param view      The view
   * @param isExiting This must be false unless this method is
   *                  being called by the exit() method
   */
  public static boolean closeAllBuffers(View view, boolean isExiting)
  {
    if (view != null)
      view.getEditPane().saveCaretInfo();

    boolean dirty = false;

    boolean saveRecent = !(isExiting && jEdit.getBooleanProperty("restore"));

    Buffer buffer = buffersFirst;
    while (buffer != null) {
      if (buffer.isDirty()) {
        dirty = true;
        break;
      }
      buffer = buffer.next;
    }

    if (dirty) {
      boolean ok = new CloseDialog(view).isOK();
      if (!ok)
        return false;
    }

    // Wait for pending I/O requests
    VFSManager.waitForRequests();
    if (VFSManager.errorOccurred())
      return false;

    // close remaining buffers (the close dialog only deals with
    // dirty ones)

    buffer = buffersFirst;
    System.out.println("before bufferHash.clear();");
    // zero it here so that BufferTabs doesn't have any problems
    buffersFirst = buffersLast = null;
    bufferHash.clear();
    bufferCount = 0;

    while (buffer != null) {
      if (!buffer.isNewFile() && saveRecent) {
        Integer _caret = (Integer) buffer.getProperty(Buffer.CARET);
        int caret = (_caret == null ? 0 : _caret.intValue());
        BufferHistory.setEntry(buffer.getPath(), caret,
                (Selection[]) buffer.getProperty(Buffer.SELECTION),
                buffer.getStringProperty(Buffer.ENCODING));
      }

      buffer.close();
      DisplayManager.bufferClosed(buffer);
      unlockRule(buffer.getSymlinkPath());
      if (!isExiting) {
        EditBus.send(new BufferUpdate(buffer, view,
                BufferUpdate.CLOSED));
      }
      buffer = buffer.next;
    }

    if (!isExiting)
      newFile(view);

    PerspectiveManager.setPerspectiveDirty(true);

    return true;
  } //}}}

  //{{{ saveAllBuffers() method
  /**
   * Saves all open buffers.
   *
   * @param view The view
   * @since jEdit 4.2pre1
   */
  public static void saveAllBuffers(View view)
  {
    saveAllBuffers(view, jEdit.getBooleanProperty("confirmSaveAll"));
  } //}}}

  //{{{ saveAllBuffers() method
  /**
   * Saves all open buffers.
   *
   * @param view    The view
   * @param confirm If true, a confirmation dialog will be shown first
   * @since jEdit 2.7pre2
   */
  public static void saveAllBuffers(View view, boolean confirm)
  {
    if (confirm) {
      int result = GUIUtilities.confirm(view, "saveall", null,
              JOptionPane.YES_NO_OPTION,
              JOptionPane.QUESTION_MESSAGE);
      if (result != JOptionPane.YES_OPTION)
        return;
    }

    Buffer current = view.getBuffer();

    Buffer buffer = buffersFirst;
    while (buffer != null) {
      if (buffer.isDirty()) {
        if (buffer.isNewFile())
          view.setBuffer(buffer);
        buffer.save(view, null, true);
      }

      buffer = buffer.next;
    }

    view.setBuffer(current);
  } //}}}

  //{{{ reloadAllBuffers() method
  /**
   * Reloads all open buffers.
   *
   * @param view    The view
   * @param confirm If true, a confirmation dialog will be shown first
   *                if any buffers are dirty
   * @since jEdit 2.7pre2
   */
  public static void reloadAllBuffers(final View view, boolean confirm)
  {
    boolean hasDirty = false;
    Buffer[] buffers = jEdit.getBuffers();

    for (int i = 0; i < buffers.length && hasDirty == false; i++)
      hasDirty = buffers[i].isDirty();

    if (confirm && hasDirty) {
      int result = GUIUtilities.confirm(view, "reload-all", null,
              JOptionPane.YES_NO_OPTION,
              JOptionPane.QUESTION_MESSAGE);
      if (result != JOptionPane.YES_OPTION)
        return;
    }

    // save caret info. Buffer.load() will load it.
    View _view = viewsFirst;
    while (_view != null) {
      EditPane[] panes = _view.getEditPanes();
      for (int i = 0; i < panes.length; i++) {
        panes[i].saveCaretInfo();
      }

      _view = _view.next;
    }

    for (int i = 0; i < buffers.length; i++) {
      Buffer buffer = buffers[i];
      buffer.load(view, true);
    }
  } //}}}

  //{{{ _getBuffer() method
  /**
   * Returns the buffer with the specified path name. The path name
   * must be an absolute, canonical, path.
   *
   * @param path The path name
   * @see MiscUtilities#constructPath(String,String)
   * @see MiscUtilities#resolveSymlinks(String)
   * @see #getBuffer(String)
   * @since jEdit 4.2pre7
   */
  public static Buffer _getBuffer(String path)
  {
    // paths on case-insensitive filesystems are stored as lower
    // case in the hash.
    if ((VFSManager.getVFSForPath(path).getCapabilities()
            & VFS.CASE_INSENSITIVE_CAP) != 0) {
      path = path.toLowerCase();
    }
    //System.out.println("jEdit.getBuffer newPath= "+path);
    synchronized (bufferListLock) {
      return (Buffer) bufferHash.get(path);
    }
  } //}}}

  //{{{ getBuffer() method
  /**
   * Returns the buffer with the specified path name. The path name
   * must be an absolute path. This method automatically resolves
   * symbolic links. If performance is critical, cache the canonical
   * path and call {@link #_getBuffer(String)} instead.
   *
   * @param path The path name
   * @see MiscUtilities#constructPath(String,String)
   * @see MiscUtilities#resolveSymlinks(String)
   */
  public static Buffer getBuffer(String path)
  {
    if (!jEdit.getBooleanProperty("bufferWorkWithId")) path=MiscUtilities.resolveSymlinks(path);
    return _getBuffer(path);
  } //}}}

  //{{{ getBuffers() method
  /**
   * Returns an array of open buffers.
   */
  public static Buffer[] getBuffers()
  {
    synchronized (bufferListLock) {
      Buffer[] buffers = new Buffer[bufferCount];
      Buffer buffer = buffersFirst;
      for (int i = 0; i < bufferCount; i++) {
        buffers[i] = buffer;
        buffer = buffer.next;
      }
      return buffers;
    }
  } //}}}

  //{{{ getBufferCount() method
  /**
   * Returns the number of open buffers.
   */
  public static int getBufferCount()
  {
    return bufferCount;
  } //}}}

  //{{{ getFirstBuffer() method
  /**
   * Returns the first buffer.
   */
  public static Buffer getFirstBuffer()
  {
    return buffersFirst;
  } //}}}

  //{{{ getLastBuffer() method
  /**
   * Returns the last buffer.
   */
  public static Buffer getLastBuffer()
  {
    return buffersLast;
  } //}}}

  //{{{ checkBufferStatus() method
  /**
   * Checks each buffer's status on disk and shows the dialog box
   * informing the user that buffers changed on disk, if necessary.
   *
   * @param view The view
   * @since jEdit 4.2pre1
   */
  public static void checkBufferStatus(View view)
  {
    // still need to call the status check even if the option is
    // off, so that the write protection is updated if it changes
    // on disk
    boolean showDialogSetting = getBooleanProperty("autoReloadDialog");

    // auto reload changed buffers?
    boolean autoReloadSetting = getBooleanProperty("autoReload");

    // the problem with this is that if we have two edit panes
    // looking at the same buffer and the file is reloaded both
    // will jump to the same location
    View _view = viewsFirst;
    while (_view != null) {
      EditPane[] editPanes = _view.getEditPanes();
      for (int i = 0; i < editPanes.length; i++) {
        editPanes[i].saveCaretInfo();
      }
      _view = _view.next;
    }

    Buffer buffer = buffersFirst;
    int[] states = new int[bufferCount];
    int i = 0;
    boolean show = false;
    while (buffer != null) {
      states[i] = buffer.checkFileStatus(view);

      switch (states[i]) {
        case Buffer.FILE_CHANGED:
          if (autoReloadSetting
                  && showDialogSetting
                  && !buffer.isDirty()) {
            buffer.load(view, true);
          }
          /* fall through */
        case Buffer.FILE_DELETED:
          show = true;
          break;
      }

      buffer = buffer.next;
      i++;
    }

    if (show && showDialogSetting)
      new FilesChangedDialog(view, states, autoReloadSetting);
  } //}}}

  //}}}

  //{{{ View methods

  //{{{ getInputHandler() method
  /**
   * Returns the current input handler (key binding to action mapping)
   *
   * @see org.gjt.sp.jedit.gui.InputHandler
   */
  public static InputHandler getInputHandler()
  {
    return inputHandler;
  } //}}}

  /* public static void newViewTest()
  {
    long time = System.currentTimeMillis();
    for(int i = 0; i < 30; i++)
    {
      Buffer b = newFile(null);
      b.insert(0,"x");
      new View(b,null,false);
    }
    System.err.println(System.currentTimeMillis() - time);
  } */

  //{{{ newView() method
  /**
   * Creates a new view.
   *
   * @param view An existing view
   * @since jEdit 3.2pre2
   */
  public static View newView(View view)
  {
    return newView(view, null, false);
  } //}}}

  //{{{ newView() method
  /**
   * Creates a new view of a buffer.
   *
   * @param view   An existing view
   * @param buffer The buffer
   */
  public static View newView(View view, Buffer buffer)
  {
    return newView(view, buffer, false);
  } //}}}

  //{{{ newView() method
  /**
   * Creates a new view of a buffer.
   *
   * @param view      An existing view
   * @param buffer    The buffer
   * @param plainView If true, the view will not have dockable windows or
   *                  tool bars.
   * @since 4.1pre2
   */
  public static View newView(View view, Buffer buffer, boolean plainView)
  {
    View.ViewConfig config;
    if (view != null && (plainView == view.isPlainView()))
      config = view.getViewConfig();
    else
      config = new View.ViewConfig(plainView);
    return newView(view, buffer, config);
  } //}}}

  //{{{ newView() method
  /**
   * Creates a new view.
   *
   * @param view   An existing view
   * @param buffer A buffer to display, or null
   * @param config Encapsulates the view geometry, split configuration
   *               and if the view is a plain view
   * @since jEdit 4.2pre1
   */
  public static View newView(View view, Buffer buffer, View.ViewConfig config)
  {
    PerspectiveManager.setPerspectiveDirty(true);

    try {
      if (view != null) {
        view.showWaitCursor();
        view.getEditPane().saveCaretInfo();
      }
      View newView = new View(buffer, config);
      destroyed=false;// mast stay after  new View#setSplitConfig ( in it use this parameter )
      addViewToList(newView);

      if (!config.plainView) {
        DockableWindowManager wm = newView.getDockableWindowManager();
        if (config.top != null
                && config.top.length() != 0)
          wm.showDockableWindow(config.top);

        if (config.left != null
                && config.left.length() != 0)
          wm.showDockableWindow(config.left);

        if (config.bottom != null
                && config.bottom.length() != 0)
          wm.showDockableWindow(config.bottom);

        if (config.right != null
                && config.right.length() != 0)
          wm.showDockableWindow(config.right);
      }

      newView.pack();

      if (config.width != 0 && config.height != 0) {
       if (config.x!=0 || config.y!=0) {
        Rectangle desired = new Rectangle(config.x, config.y, config.width,config.height);
        if (OperatingSystem.isX11() && Debug.GEOMETRY_WORKAROUND) {
          new GUIUtilities.UnixWorkaround(newView,"view", desired, config.extState);
        }
        else {
          newView.setBounds(desired);
          newView.setExtendedState(config.extState);
        }
       }
        else {GUIUtilities.centerOnScreen(newView,config.width,config.height);   
              newView.setExtendedState(config.extState);
              }
      }
      else
        GUIUtilities.centerOnScreen(newView);

      EditBus.send(new ViewUpdate(newView, ViewUpdate.CREATED));
      newView.setVisible(true);

      // show tip of the day
      if (newView == viewsFirst) {
        newView.getTextArea().requestFocus();

        // Don't show the welcome message if jEdit was started
        // with the -nosettings switch
        if (settingsDirectory != null && getBooleanProperty("firstTime"))
          new HelpViewer();
        else if (jEdit.getBooleanProperty("tip.show"))
          new TipOfTheDay(newView);

        setBooleanProperty("firstTime", false);
      }
      else
        GUIUtilities.requestFocus(newView, newView.getTextArea());

      return newView;
    } finally {
      if (view != null)
        view.hideWaitCursor();
    }
  } //}}}

  //{{{ closeView() method
  /**
   * Closes a view.
   * <p/>
   * jEdit will exit if this was the last open view.
   */
  public static void closeView(View view)
  {
    closeView(view, true);
  } //}}}

  //{{{ getViews() method
  /**
   * Returns an array of all open views.
   */
  public static View[] getViews()
  {
    View[] views = new View[viewCount];
    View view = viewsFirst;
    for (int i = 0; i < viewCount; i++) {
      views[i] = view;
      view = view.next;
    }
    return views;
  } //}}}

  //{{{ getViewCount() method
  /**
   * Returns the number of open views.
   */
  public static int getViewCount()
  {
    return viewCount;
  } //}}}

  //{{{ getFirstView() method
  /**
   * Returns the first view.
   */
  public static View getFirstView()
  {
    return viewsFirst;
  } //}}}

  //{{{ getLastView() method
  /**
   * Returns the last view.
   */
  public static View getLastView()
  {
    return viewsLast;
  } //}}}

  //{{{ getActiveView() method
  /**
   * Returns the currently focused view.
   *
   * @since jEdit 4.1pre1
   */
  public static View getActiveView()
  {
    if (activeView == null) {
      // eg user just closed a view and didn't focus another
      return viewsFirst;
    }
    else
      return activeView;
  } //}}}

  //}}}

  //{{{ Miscellaneous methods

  //{{{ isMainThread() method
  /**
   * Returns true if the currently running thread is the main thread.
   *
   * @since jEdit 4.2pre1
   */
  public static boolean isMainThread()
  {
    return (Thread.currentThread() == mainThread);
  } //}}}

  //{{{ isBackgroundMode() method
  /**
   * Returns true if jEdit was started with the <code>-background</code>
   * command-line switch.
   *
   * @since jEdit 4.0pre4
   */
  public static boolean isBackgroundModeEnabled()
  {
    return background;
  } //}}}

  //{{{ showMemoryStatusDialog() method
  /**
   * Performs garbage collection and displays a dialog box showing
   * memory status.
   *
   * @param view The view
   * @since jEdit 4.0pre1
   */
  public static void showMemoryDialog(View view)
  {
    Runtime rt = Runtime.getRuntime();
    int before = (int) (rt.freeMemory() / 1024);
    System.gc();
    int after = (int) (rt.freeMemory() / 1024);
    int total = (int) (rt.totalMemory() / 1024);

    JProgressBar progress = new JProgressBar(0, total);
    progress.setValue(total - after);
    progress.setStringPainted(true);
    progress.setString(jEdit.getProperty("memory-status.use",
            new Object[]{new Integer(total - after),new Integer(total)}));

    Object[] message = new Object[4];
    message[0] = getProperty("memory-status.gc",new Object[]{new Integer(after - before)});
    message[1] = Box.createVerticalStrut(12);
    message[2] = progress;
    message[3] = Box.createVerticalStrut(6);

    JOptionPane.showMessageDialog(view, message,
            jEdit.getProperty("memory-status.title"),
            JOptionPane.INFORMATION_MESSAGE);
  } //}}}

  //{{{ getJEditHome() method
  /**
   * Returns the jEdit install directory.
   */
  public static String getJEditHome()
  {
    return jEditHome;
  } //}}}

  //{{{ getSettingsDirectory() method
  /**
   * Returns the path of the directory where user-specific settings
   * are stored. This will be <code>null</code> if jEdit was
   * started with the <code>-nosettings</code> command-line switch; do not
   * blindly use this method without checking for a <code>null</code>
   * return value first.
   */
  public static String getSettingsDirectory()
  {
    return settingsDirectory;
  } //}}}

  //{{{ getJARCacheDirectory() method
  /**
   * Returns the directory where plugin cache files are stored.
   *
   * @since jEdit 4.2pre1
   */
  public static String getJARCacheDirectory()
  {
    return jarCacheDirectory;
  } //}}}

  //{{{ backupSettingsFile() method
  /**
   * Backs up the specified file in the settings directory.
   * You should call this on any settings files your plugin
   * writes.
   *
   * @param file The file
   * @since jEdit 4.0pre1
   */
  public static void backupSettingsFile(File file)
  {
    if (settingsDirectory == null)
      return;

    String backupDir = MiscUtilities.constructPath(settingsDirectory, "settings-backup");
    File dir = new File(backupDir);
    if (!dir.exists()) dir.mkdirs();

    // ... sweet. saveBackup() will create backupDir if it
    // doesn't exist.

    MiscUtilities.saveBackup(file.getPath(), 5, null, "~", backupDir);
  } //}}}

  //{{{ saveSettings() method
  /**
   * Saves all user preferences to disk.
   */
  public static void saveSettings()
  {
      System.out.println("jEdit:saveSettings()");
      if (settingsDirectory == null)
      return;

    Abbrevs.save();
    FavoritesVFS.saveFavorites();
    HistoryModel.saveHistory();
    Registers.saveRegisters();
    SearchAndReplace.save();
    BufferHistory.save();
    KillRing.getInstance().save();

    File file1 = new File(MiscUtilities.constructPath(settingsDirectory, "#properties#save#"));
    System.out.println("jEdit:saveSettings() file1:"+file1);
    File file2 = new File(MiscUtilities.constructPath(settingsDirectory, "properties"));
    System.out.println("jEdit:saveSettings() file2:"+file2);
    if (file2.exists() && file2.lastModified() != propsModTime) {
      Log.log(Log.WARNING, jEdit.class, file2 + " changed"
              + " on disk; will not save user properties");
      System.out.println("jEdit:saveSettings() " +file2 + " changed"
              + " on disk; will not save user properties");
    }
    else {
      backupSettingsFile(file2);

      try {
        OutputStream out = new FileOutputStream(file1);
        propMgr.saveUserProps(out);
        file2.delete();
        file1.renameTo(file2);
      } catch (IOException io) {
        Log.log(Log.ERROR, jEdit.class, io);
      }

      propsModTime = file2.lastModified();
      System.out.println("jEdit:saveSettings() properties was saved."); 
    }
  } //}}}

  //{{{ exit() method
  /**
   * Exits cleanly from jEdit, prompting the user if any unsaved files
   * should be saved first.
   *
   * @param view       The view from which this exit was called
   * @param reallyExit If background mode is enabled and this parameter
   *                   is true, then jEdit will close all open views instead of exiting
   *                   entirely.
   */
  public static void exit(View view, boolean reallyExit)
  {
    // Close dialog, view.close() call need a view...
    if (view == null)
      view = activeView;
    String ruleAction = (view.getBuffer().getBooleanProperty("newRule"))?"addRule":"EditRule";
    String path = view.getBuffer().getSymlinkPath();

     for (int i = 0; i < jars.size(); i++) {
       PluginJAR jar=(PluginJAR) jars.elementAt(i);
       jar.WindowClose(view);
     }
   // jEdit.closeBuffer(view,view.getBuffer());
    // Wait for pending I/O requests
    VFSManager.waitForRequests();
    // Send EditorExitRequested
    EditBus.send(new EditorExitRequested(view));
    // Even if reallyExit is false, we still exit properly
    // if background mode is off
    reallyExit |= !background;

    PerspectiveManager.savePerspective(false);

    try {
      PerspectiveManager.setPerspectiveEnabled(false);
      // Close all buffers
      if (!closeAllBuffers(view, reallyExit))
        return;
    } finally {
      PerspectiveManager.setPerspectiveEnabled(true);
    }
    System.out.println("i am closedAllBuffers");
    // If we are running in background mode and
    // reallyExit was not specified, then return here.
    if (!reallyExit) { // in this case, we can't directly call   view.close(); we have to call closeView()
      // for all open views
      System.out.println("if (!reallyExit) i am here !!!");
      view = viewsFirst;
      while (view != null) {
        closeView(view, false);
        view = view.next;
      }
      saveSettings();// Save settings in case user kills the backgrounded jEdit process
    }
    else {  // Save view properties here

      removeViewFromList( view);
       if (view != null) view.close();

      if (server != null) server.stopServer();// Stop server
      // Stop all plugins
      PluginJAR[] plugins = getPluginJARs();
      for (int i = 0; i < plugins.length; i++) {
        removePluginJAR(plugins[i], true);
      }
      EditBus.send(new EditorExiting(null));// Send EditorExiting
      saveSettings();// Save settings
      Log.closeStream();// Close activity log stream
      System.out.println("ByeBye...");
      // Byebye...
     // System.exit(0);
    }
    setWindowClosed(ruleAction);
    if (lcTimer!=null) {
        lcTimer.cancel();
        System.out.println("lcTimer was canceled.");
    }
  } //}}}
  //{{{ exitView() method
  /**
   * Exits cleanly from jEdit, prompting the user if any unsaved files
   * should be saved first.
   *
   * @param view       The view from which this exit was called
   * @param reallyExit If background mode is enabled and this parameter
   *                   is true, then jEdit will close all open views instead of exiting
   *                   entirely.
   */
  public static void exitView(View view, boolean reallyExit, String path)
  {
    // Close dialog, view.close() call need a view...
    if (view == null)
      view = activeView;
    String ruleAction = (view.getBuffer().getBooleanProperty("newRule"))?"addRule":"EditRule";
   // jEdit.closeBuffer(view,view.getBuffer());
    // Wait for pending I/O requests
    VFSManager.waitForRequests();
    // Send EditorExitRequested
    EditBus.send(new EditorExitRequested(view));
    // Even if reallyExit is false, we still exit properly
    // if background mode is off
    reallyExit |= !background;

    PerspectiveManager.savePerspective(false);

    try {
      PerspectiveManager.setPerspectiveEnabled(false);
      // Close all buffers
      if (!closeBuffer(view, view.getBuffer()))
        return;
    } finally {
      PerspectiveManager.setPerspectiveEnabled(true);
    }
    System.out.println("i am closedAllBuffers");
    // If we are running in background mode and
    // reallyExit was not specified, then return here.
    if (!reallyExit) { // in this case, we can't directly call   view.close(); we have to call closeView()
      // for all open views
      System.out.println("if (!reallyExit) i am here !!!");
      view = viewsFirst;
      while (view != null) {
        closeView(view, false);
        view = view.next;
      }
      saveSettings();// Save settings in case user kills the backgrounded jEdit process
    }
    else {  // Save view properties here

      removeViewFromList( view);
      for (int i = 0; i < jars.size(); i++) {
         PluginJAR jar=(PluginJAR) jars.elementAt(i);
         jar.WindowClose(view);
       }
       EditBus.send(new ViewUpdate(view,ViewUpdate.CLOSED, path));// Send EditorExiting
       if (view != null) view.close();

      // Stop all plugins
   /*   PluginJAR[] plugins = getPluginJARs();
      for (int i = 0; i < plugins.length; i++) {
        removePluginJAR(plugins[i], true);
      }  */

     // EditBus.send(new EditorExiting(null));// Send EditorExiting
      saveSettings();// Save settings
      Log.closeStream();// Close activity log stream
      System.out.println("ByeBye...");
      // Byebye...
     // System.exit(0);
    }
    setWindowClosed(ruleAction);
    if (lcTimer!=null) {
        lcTimer.cancel();
        System.out.println("lcTimer was canceled.");
    }
  } //}}}

  private static void setWindowClosed(String ruleAction) {
    windowClosed = ruleAction;
    if (viewCount==0) stopped = true;
  }

  public String isWindowClosed() {
    return windowClosed;
  }

  private static void unlockAllRules() {
    //System.out.println("$$$$$$$$Destroyed$$$$$$$$ " + bufferHash.size());
    Set ruleIds = null;
    if (bufferHash!=null && bufferHash.size()>0) {
      ruleIds = bufferHash.keySet();
      for (Iterator i = ruleIds.iterator();i.hasNext();)
        unlockRule((String)i.next());
    }
  }
  private static void unlockRule(String file) {
    HashMap h = new HashMap();
    h.put("file",file);
    HttpGet(h,UnLockServiceRule);
  }
  //{{{ getEditServer() method
  /**
   * Returns the edit server instance. You can use this to find out the
   * port number jEdit is listening on.
   *
   * @since jEdit 4.2pre10
   */
  public static EditServer getEditServer()
  {
    return server;
  } //}}}

  //}}}

  //{{{ Package-private members

  //{{{ updatePosition() method
  /**
   * If buffer sorting is enabled, this repositions the buffer.
   */
  static void updatePosition(String oldPath, Buffer buffer)
  {
    if ((VFSManager.getVFSForPath(oldPath).getCapabilities()  & VFS.CASE_INSENSITIVE_CAP) != 0) {
      oldPath = oldPath.toLowerCase();
    }
    bufferHash.remove(oldPath);
    String path = buffer.getSymlinkPath();
    if ((VFSManager.getVFSForPath(path).getCapabilities() & VFS.CASE_INSENSITIVE_CAP) != 0) {
      path = path.toLowerCase();
    }
    bufferHash.put(path, buffer);
    if (sortBuffers) {
      removeBufferFromList(buffer);
      addBufferToList(buffer);
    }
  } //}}}

  //{{{ addMode() method
  /**
   * Do not call this method. It is only public so that classes
   * in the org.gjt.sp.jedit.syntax package can access it.
   *
   * @param mode The edit mode
   */
  public static void addMode(Mode mode)
  {
    //Log.log(Log.DEBUG,jEdit.class,"Adding edit mode "
    // + mode.getName());

    modes.addElement(mode);
  } //}}}

  //{{{ loadMode() method
  /**
   * Loads an XML-defined edit mode from the specified reader.
   *
   * @param mode The edit mode
   */
  /* package-private */
  static void loadMode(Mode mode)
  {
    final String fileName = (String) mode.getProperty("file");

    Log.log(Log.NOTICE, jEdit.class, "Loading edit mode " + fileName);
    System.out.println("jEdit.loadMode() Loading edit mode " + fileName);  

    final XmlParser parser = new XmlParser();
    XModeHandler xmh = new XModeHandler(mode.getName())
    {
      public void error(String what, Object subst)
      {
        int line = parser.getLineNumber();
        int column = parser.getColumnNumber();
        String msg;
        if (subst == null) msg = jEdit.getProperty("xmode-error." + what);
        else {
          msg = jEdit.getProperty("xmode-error." + what,new String[]{subst.toString()});
          if (subst instanceof Throwable) Log.log(Log.ERROR, this, subst);
        }
        Object[] args = {fileName, new Integer(line),new Integer(column), msg};
        GUIUtilities.error(null, "xmode-error", args);
      }

      public TokenMarker getTokenMarker(String modeName)
      {
        Mode mode = getMode(modeName);
        if (mode == null) return null;
        else return mode.getTokenMarker();
      }
    };

    mode.setTokenMarker(xmh.getTokenMarker());
   BufferedReader grammar = null;
    parser.setHandler(xmh);
    URL url;HttpURLConnection urlcon = null;InputStream _in = null;
    try {int command = ParseXml;
      String content = "?username=" + jEdit.username + "&password=" + jEdit.password + "&file=" + fileName + "&command=" + command;
      url = new URL(jEdit.servletUrl, content); //url=new URL(path);
      urlcon = (HttpURLConnection) url.openConnection();
      System.out.println("jEdit open url connection: url="+url);
      _in = urlcon.getInputStream(); // _in = new FileInputStream(path);
      grammar = new BufferedReader(new InputStreamReader(_in));//new FileReader(fileName));
      String status = urlcon.getHeaderField("status");
      if (status.equals("ok")){

        long startTime = System.currentTimeMillis();

        parser.parse(null, null, grammar);

        int currentTime=(int)(System.currentTimeMillis()-startTime);
        System.out.println("jEdit parse time:"+currentTime+" ms");
      }
      else
        throw new FileNotFoundException(status);
      mode.setProperties(xmh.getModeProperties());
    } catch (Throwable e) {
      e.printStackTrace();// Log.log(Log.ERROR, jEdit.class, e);
      if (e instanceof XmlException) {
        XmlException xe = (XmlException) e;
        int line = xe.getLine();
        String message = xe.getMessage();
        Object[] args = {fileName, new Integer(line), null,message};
        GUIUtilities.error(null, "xmode-error", args);
      }
    } finally {
      try {
        if (_in != null) _in.close();
        if (urlcon != null) urlcon.disconnect();
        if (grammar != null) grammar.close();
      } catch (IOException io) {
        Log.log(Log.ERROR, jEdit.class, io);
      }
    }
  } //}}}

  //{{{ addPluginProps() method
  static void addPluginProps(Properties map)
  {
    propMgr.addPluginProps(map);
  } //}}}

  //{{{ removePluginProps() method
  static void removePluginProps(Properties map)
  {
    propMgr.removePluginProps(map);
  } //}}}

  //{{{ pluginError() method
  static void pluginError(String path, String messageProp,
                          Object[] args)
  {
    synchronized (pluginErrorLock) {
      if (pluginErrors == null)
        pluginErrors = new Vector();

      ErrorListDialog.ErrorEntry newEntry =
              new ErrorListDialog.ErrorEntry(path, messageProp, args);

      for (int i = 0; i < pluginErrors.size(); i++) {
        if (pluginErrors.get(i).equals(newEntry))
          return;
      }
      pluginErrors.addElement(newEntry);

      if (startupDone) {
        SwingUtilities.invokeLater(new Runnable()
        {
          public void run()
          {
            long startTime = System.currentTimeMillis();

            showPluginErrorDialog();

            int currentTime=(int)(System.currentTimeMillis()-startTime);
            System.out.println("jEdit show plugin error dialog time:"+currentTime+" ms");
          }
        });
      }
    }
  } //}}}

  //{{{ setActiveView() method
  static void setActiveView(View view)
  {
    jEdit.activeView = view;
  } //}}}

  //}}}

  //{{{ Private members

  //{{{ Static variables
  private static String jEditHome;
  private static String settingsDirectory;
  private static URL settingsURL;
  private static String jarCacheDirectory;
  private static long propsModTime;
  private static PropertyManager propMgr;
  private static EditServer server;
  private static java.util.Timer lcTimer;
  //private static LiveConnect lc;
  private static LiveConnectTimerTask lctt;
  private static boolean background;
  private static ActionContext actionContext;
  private static ActionSet builtInActionSet;
  private static Vector pluginErrors;
  private static Object pluginErrorLock = new Object();
  private static Vector jars;
  private static Vector modes;
  private static boolean saveCaret;
  private static InputHandler inputHandler;
  private static HashMap systemProperties = new HashMap();

  // buffer link list
  private static boolean sortBuffers;
  private static boolean sortByName;
  private static int bufferCount;
  private static Buffer buffersFirst;
  private static Buffer buffersLast;
  private static Map bufferHash;

  // makes openTemporary() thread-safe
  private static Object bufferListLock = new Object();

  // view link list
  private static int viewCount;
  private static View viewsFirst;
  private static View viewsLast;
  private static View activeView;

  private static boolean startupDone;
  private static boolean destroyed;
  private static Thread mainThread;
  //}}}
  private static final String addRule="addRule";
  private static final String editRule="EditRule";
    
  public jEdit()
  {
  }

  //{{{ usage() method
  private static void usage()
  {
    System.out.println("Usage: jedit [<options>] [<files>]");

    System.out.println(" <file> +marker:<marker>: Positions caret" + " at marker <marker>");
    System.out.println(" <file> +line:<line>: Positions caret" + " at line number <line>");
    System.out.println(" --: End of options");
    System.out.println(" -background: Run in background mode");
    System.out.println(" -nobackground: Disable background mode (default)");
    System.out.println(" -gui: Only if running in background mode; open initial view (default)");
    System.out.println(" -nogui: Only if running in background mode; don't open initial view");
    System.out.println(" -log=<level>: Log messages with level equal to or higher than this to");
    System.out.println("  standard error. <level> must be between 1 and 9. Default is 7.");
    System.out.println(" -newplainview: Client instance opens a new plain view");
    System.out.println(" -newview: Client instance opens a new view (default)");
    System.out.println(" -plugins: Load plugins (default)");
    System.out.println(" -noplugins: Don't load any plugins");
    System.out.println(" -restore: Restore previously open files (default)");
    System.out.println(" -norestore: Don't restore previously open files");
    System.out.println(" -reuseview: Client instance reuses existing view");
    System.out.println(" -quit: Quit a running instance");
    System.out.println(" -run=<script>: Run the specified BeanShell script");
    System.out.println(" -server: Read/write server info from/to $HOME/.jedit/server (default)");
    System.out.println(" -server=<name>: Read/write server info from/to $HOME/.jedit/<name>");
    System.out.println(" -noserver: Don't start edit server");
    System.out.println(" -settings=<path>: Load user-specific settings from <path>");
    System.out.println(" -nosettings: Don't load user-specific settings");
    System.out.println(" -startupscripts: Run startup scripts (default)");
    System.out.println(" -nostartupscripts: Don't run startup scripts");
    System.out.println(" -usage: Print this message and exit");
    System.out.println(" -version: Print jEdit version and exit");
    System.out.println(" -wait: Wait until the user closes the specified buffer in the server");
    System.out.println("  instance. Does nothing if passed to the initial jEdit instance.");
    System.out.println();
    System.out.println("Report bugs to Slava Pestov <slava@jedit.org>.");
  } //}}}

  //{{{ version() method
  private static void version()
  {
    System.out.println("jEdit " + getVersion());
  } //}}}

  //{{{ makeServerScript() method
  /**
   * Creates a BeanShell script that can be sent to a running edit server.
   */
  private static String makeServerScript(boolean wait,
                                         boolean restore, boolean newView,
                                         boolean newPlainView, String[] args,
                                         String scriptFile)
  {
    StringBuffer script = new StringBuffer();
    String userDir = System.getProperty("user.dir");
    script.append("parent = \"");
    script.append(MiscUtilities.charsToEscapes(userDir));
    script.append("\";\n");
    script.append("args = new String[");
    script.append(args.length);
    script.append("];\n");
    for (int i = 0; i < args.length; i++) {
      script.append("args[");
      script.append(i);
      script.append("] = ");

      if (args[i] == null)
        script.append("null");
      else {
        script.append('"');
        script.append(MiscUtilities.charsToEscapes(args[i]));
        script.append('"');
      }
      script.append(";\n");
    }

    script.append("view = jEdit.getLastView();\n");
    script.append("buffer = EditServer.handleClient("
            + restore + "," + newView + "," + newPlainView +
            ",parent,args);\n");
    script.append("if(buffer != null && " + wait + ") {\n");
    script.append("\tbuffer.setWaitSocket(socket);\n");
    script.append("\tdoNotCloseSocket = true;\n");
    script.append("}\n");
    script.append("if(view != jEdit.getLastView() && " + wait + ") {\n");
    script.append("\tjEditApplet.getLastView().setWaitSocket(socket);\n");
    script.append("\tdoNotCloseSocket = true;\n");
    script.append("}\n");
    script.append("if(doNotCloseSocket == void)\n");
    script.append("\tsocket.close();\n");

    if (scriptFile != null) {
      scriptFile = MiscUtilities.constructPath(userDir, scriptFile);
      script.append("BeanShell.runScript(view,\""
              + MiscUtilities.charsToEscapes(scriptFile)
              + "\",null,this.namespace);\n");
    }

    return script.toString();
  } //}}}

  //{{{ initMisc() method
  /**
   * Initialise various objects, register protocol handlers.
   */
  private static void initMisc()
  {
    jars = new Vector();
    actionContext = new ActionContext()
    {
      public void invokeAction(EventObject evt,
                               EditAction action)
      {
        View view = GUIUtilities.getView((Component) evt.getSource());
        boolean actionBarVisible;
        if (view.getActionBar() == null || !view.getActionBar().isShowing())
          actionBarVisible = false;
        else
          actionBarVisible = view.getActionBar().isVisible();

        view.getInputHandler().invokeAction(action);
        if (actionBarVisible) {
          ActionBar actionBar = view.getActionBar();// XXX: action bar might not be 'temp'
          if (actionBar != null)
            view.removeToolBar(actionBar);
        }
      }
    };

    bufferHash = new HashMap();
    inputHandler = new DefaultInputHandler(null);
    jEditHome = MiscUtilities.resolveSymlinks(jEditHome);

    // Add an EditBus component that will reload edit modes and
    // macros if they are changed from within the editor
    EditBus.addToBus(new SettingsReloader());
  } //}}}

  //{{{ initSystemProperties() method
  /**
   * Load system properties.
   */
  private static void initSystemProperties()
  {    
    propMgr = new PropertyManager();
    systemProperties.put("file.encoding", StringGet("file",FileEncoding));   
    systemProperties.put("line.separator",StringGet("file",LineSeparator));  //"\n"
    try {
      //////////Debug  todo debug - commect
//      String loc = "/home/stepanov/work/smsc/webapps/jedit/src";
//      propMgr.loadSystemProps(new FileInputStream(loc+"/org/gjt/sp/jedit/jedit.props"));
//      propMgr.loadSystemProps(new FileInputStream(loc+"/org/gjt/sp/jedit/jedit_gui.props"));
//      propMgr.loadSystemProps(new FileInputStream(loc+"/org/gjt/sp/jedit/jedit_keys.props"));
      //////////End Debug

      propMgr.loadSystemProps(jEdit.class.getResourceAsStream("/org/gjt/sp/jedit/jedit.props"));
      propMgr.loadSystemProps(jEdit.class.getResourceAsStream("/org/gjt/sp/jedit/jedit_gui.props"));
      propMgr.loadSystemProps(jEdit.class.getResourceAsStream("/org/gjt/sp/jedit/jedit_keys.props"));
    } catch (Exception e) {
      e.printStackTrace();
      Log.log(Log.ERROR, jEdit.class,"Error while loading system properties!");
      Log.log(Log.ERROR, jEdit.class,"One of the following property files could not be loaded:\n"
              + "- jedit.props\n"+ "- jedit_gui.props\n" + "- jedit_keys.props\n"+ "jedit.jar is probably corrupt.");
      Log.log(Log.ERROR, jEdit.class, e);
      jEdit.exit(activeView,true);
    }
  } //}}}

  //{{{ initSiteProperties() method
  /**
   * Load site properties.
   */
  private static void initSiteProperties()
  {
    // site properties are loaded as default properties, overwriting
    // jEdit's system properties
    String siteSettingsDirectory = MiscUtilities.constructPath(jEditHome, "properties");
    if (!(BoolGet(siteSettingsDirectory, Exists) && BoolGet(siteSettingsDirectory, IsDirectory)))
      return;

    String[] snippets = StringsGet(siteSettingsDirectory, ListFiles);//siteSettings.list();
    if (snippets == null)
      return;

    MiscUtilities.quicksort(snippets,new MiscUtilities.StringICaseCompare());

    for (int i = 0; i < snippets.length; ++i) {
      String snippet = snippets[i];
      if (!snippet.toLowerCase().endsWith(".props"))
        continue;

       String path = MiscUtilities.constructPath(siteSettingsDirectory, snippet);
       HashMap args=new HashMap();args.put("file",path);
        LinkedList list=(LinkedList) HttpGet(args,ParseXml);
        propMgr.loadSitePropsA(list);
        //propMgr.loadSiteProps(new FileInputStream(new File(path)));
    }
  } //}}}

  //{{{ initResources() method
  private static void initResources()
  {
    builtInActionSet = new ActionSet(null, null, null,
            jEdit.class.getResource("actions.xml"));
    builtInActionSet.setLabel(getProperty("action-set.jEdit"));
    builtInActionSet.load();
    actionContext.addActionSet(builtInActionSet);
    DockableWindowFactory.getInstance().loadDockableWindows(null,jEdit.class.getResource("dockables.xml"),null);
    ServiceManager.loadServices(null,jEdit.class.getResource("services.xml"),null);
  } //}}}

  //{{{ initPlugins() method
  /**
   * Loads plugins.
   */
  private static void initPlugins()
  {
    if (jEditHome != null) {
      addPluginJARPackages();
    }
    PluginJAR[] jars = getPluginJARs();
    for (int i = 0; i < jars.length; i++) {
      jars[i].checkDependencies();
    }
  } //}}}

  //{{{ initUserProperties() method
  /**
   * Loads user properties.
   */
  private static void initUserProperties()
  {
    if (settingsDirectory != null) {
      File file = new File(MiscUtilities.constructPath(settingsDirectory, "properties"));
      propsModTime = file.lastModified();

      try {
        propMgr.loadUserProps(new FileInputStream(file));
      } catch (FileNotFoundException fnf) {
        fnf.printStackTrace();//Log.log(Log.DEBUG,jEdit.class,fnf);
      } catch (Exception e) {
        e.printStackTrace(); //Log.log(Log.ERROR,jEdit.class,e);
      }
    }
  } //}}}

  //{{{ fontStyleToString() method
  private static String fontStyleToString(int style)
  {
    if (style == 0)
      return "PLAIN";
    else if (style == Font.BOLD)
      return "BOLD";
    else if (style == Font.ITALIC)
      return "ITALIC";
    else if (style == (Font.BOLD | Font.ITALIC))
      return "BOLDITALIC";
    else
      throw new RuntimeException("Invalid style: " + style);
  } //}}}

  //{{{ fontToString() method
  private static String fontToString(Font font)
  {
    return font.getFamily() + "-" + fontStyleToString(font.getStyle())+ "-"+ font.getSize();
  } //}}}

  //{{{ initPLAF() method
  /**
   * Sets the Swing look and feel.
   */
  private static void initPLAF()
  {
    Font primaryFont = jEdit.getFontProperty("metal.primary.font");
    if (primaryFont != null) {
      String primaryFontString =fontToString(primaryFont);

      systemProperties.put("swing.plaf.metal.controlFont", primaryFontString);
      systemProperties.put("swing.plaf.metal.menuFont", primaryFontString);
    }

    Font secondaryFont = jEdit.getFontProperty("metal.secondary.font");
    if (secondaryFont != null) {
      String secondaryFontString =fontToString(secondaryFont);
      systemProperties.put("swing.plaf.metal.systemFont", secondaryFontString);
      systemProperties.put("swing.plaf.metal.userFont", secondaryFontString);
    }

    try {
      String lf = getProperty("lookAndFeel");
      if (lf != null && lf.length() != 0)
        UIManager.setLookAndFeel(lf);
      else if (OperatingSystem.isMacOS()) {
        UIManager.setLookAndFeel(UIManager.getSystemLookAndFeelClassName());
      }
      else {
        UIManager.setLookAndFeel(UIManager.getCrossPlatformLookAndFeelClassName());
      }
    } catch (Exception e) {
      Log.log(Log.ERROR, jEdit.class, e);
    }

    UIDefaults defaults = UIManager.getDefaults();

    // give all Swing components our colors
    if (jEdit.getBooleanProperty("textColors")) {
      Color background = new javax.swing.plaf.ColorUIResource(jEdit.getColorProperty("view.bgColor"));
      Color foreground = new javax.swing.plaf.ColorUIResource(jEdit.getColorProperty("view.fgColor"));
      Color caretColor = new javax.swing.plaf.ColorUIResource(jEdit.getColorProperty("view.caretColor"));
      Color selectionColor = new javax.swing.plaf.ColorUIResource(jEdit.getColorProperty("view.selectionColor"));

      String[] prefixes = {"TextField", "TextArea", "List", "Table"};
      for (int i = 0; i < prefixes.length; i++) {
        String prefix = prefixes[i];
        defaults.put(prefix + ".disabledBackground", background);
        defaults.put(prefix + ".background", background);
        defaults.put(prefix + ".disabledForeground", foreground);
        defaults.put(prefix + ".foreground", foreground);
        defaults.put(prefix + ".caretForeground", caretColor);
        defaults.put(prefix + ".selectionForeground", foreground);
        defaults.put(prefix + ".selectionBackground", selectionColor);
        //defaults.put(prefix + ".inactiveForeground",foreground);
      }

      defaults.put("Tree.background", background);
      defaults.put("Tree.foreground", foreground);
      defaults.put("Tree.textBackground", background);
      defaults.put("Tree.textForeground", foreground);
      defaults.put("Tree.selectionForeground", foreground);
      defaults.put("Tree.selectionBackground", selectionColor);
    }

    defaults.remove("SplitPane.border");
    defaults.remove("SplitPaneDivider.border");

    JFrame.setDefaultLookAndFeelDecorated(getBooleanProperty("decorate.frames"));
    JDialog.setDefaultLookAndFeelDecorated(getBooleanProperty("decorate.dialogs"));
//below code access denided in applet
    // KeyboardFocusManager.setCurrentKeyboardFocusManager(
    //  new MyFocusManager());
  } //}}}

  protected static final int Delete = 1;
  protected static final int CanRead = 2;
  protected static final int IsDirectory = 3;
  protected static final int MkDir = 4;
  protected static final int MkDirs = 5;
  protected static final int Exists = 6;
  protected static final int ListFiles = 7;
  protected static final int IsHidden = 8;
  protected static final int CanonPath = 9;
  protected static final int ParseXml = 10;
  protected static final int LastModified = 11;
  protected static final int CanWrite = 12;
  protected static final int Length = 13;
  protected static final int NotHiddenFiles = 14;
  protected static final int RenameTo = 15;
  protected static final int SaveBackup = 16;
  protected static final int Write = 17;
  protected static final int SetPermissions = 18;
  protected static final int GetPermissions = 19;
  protected static final int FileEncoding = 20;
  protected static final int LineSeparator = 21;
  protected static final int SeparatorChar = 22;
  protected static final int OsName = 23;
  protected static final int Transport = 24;
  protected static final int NewRule = 25;
  protected static final int UpdateRule = 26;
  protected static final int ExistRule = 27;
  protected static final int LoadRule = 28;
  protected static final int LoadNewRule = 29;
  protected static final int AddRule = 30;
  protected static final int RootElement = 31;
  protected static final int Title = 32;
  protected static final int UnLockServiceRule = 34;
  protected static final int RuleHeaderLineNumber = 35;
  protected static final int getRuleStateAndLock = 36;

  public static int getRootElement()
  {
    return RootElement;
  }

  public static int getAddRule()
  {
    return AddRule;
  }

  public static int getLoadNewRule()
  {
    return LoadNewRule;
  }

  public static int getLoadRule()
  {
    return LoadRule;
  }

  public static int getExistRule()
  {
    return ExistRule;
  }
   public static int getUpdateRule()
   {
     return UpdateRule;
   }
  public static int getExists()
  {
    return Exists;
  }

  public static int getCanRead()
  {
    return CanRead;
  }

  public static int getCanWrite()
  {
    return CanWrite;
  }

  public static int getIsDirectory()
  {
    return IsDirectory;
  }

  public static int getIsHidden()
  {
    return IsHidden;
  }

  public static int getLastModified()
  {
    return LastModified;
  }

  public static int getLength()
  {
    return Length;
  }

  public static int getNotHiddenFiles()
  {
    return NotHiddenFiles;
  }

  public static int getCanonPath()
  {
    return CanonPath;
  }

  public static int getDelete()
  {
    return Delete;
  }

  public static int getMkDir()
  {
    return MkDir;
  }

  public static int getMkDirs()
  {
    return MkDirs;
  }

  public static int getRenameTo()
  {
    return RenameTo;
  }

  public static int getSaveBackup()
  {
    return SaveBackup;
  }

  public static int getParseXml()
  {
    return ParseXml;
  }

  public static int getWrite()
  {
    return Write;
  }

  public static int getListFiles()
  {
    return ListFiles;
  }

  public static int getPermissions()
  {
    return GetPermissions;
  }

  public static int getRuleHeaderLineNumber() {
    return RuleHeaderLineNumber;
  }
  //{{{ SaveBackupGet() method
  /**
   * implemented in servlet MiscUtilities.saveBackup method
   * send file command in servlet via GET method .
   * return true if success and false  if otherwise
   */
  public static boolean SaveBackupGet(final String file, int backups,
                                      String backupPrefix, String backupSuffix,
                                      String backupDirectory, int backupTimeDistance)
  {
    System.out.println("jEdit save backup ...");
    HashMap args = new HashMap();
    args.put("bs", String.valueOf(backups));
    args.put("bPrx", backupPrefix);
    args.put("bSix", backupSuffix);
    args.put("bDir", backupDirectory);
    args.put("bTmDst", String.valueOf(backupTimeDistance));
    args.put("file", file);

    LinkedList list = (LinkedList) HttpGet(args, SaveBackup);
    System.out.println("jEdit backup was saved. ");  
    boolean result = false;
    String inputLine;
    inputLine = (String) list.get(0);
    if (inputLine.equals("true")) result = true;

    System.out.println("jEdit save backup result=" + result);
    return result;
  }
  //{{{ SetPermissionsGet() method
  /**
   * implemented in servlet File.renameTo(toFile) method
   * send file command in servlet via GET method .
   * return true if success and false  if otherwise
   */
  public static boolean SetPermissionsGet(final String file, final int command, final int param)
  {
    HashMap args = new HashMap();
    args.put("intparam", String.valueOf(param));
    args.put("file", file);
    LinkedList list = (LinkedList) HttpGet(args, command);
    boolean result = false;
    String inputLine;
    inputLine = (String) list.get(0);
    if (inputLine.equals("true")) result = true;
    return result;
  }
  //{{{ RenameGet() method
  /**
   * implemented in servlet File.renameTo(toFile) method
   * send file command in servlet via GET method .
   * return true if success and false  if otherwise
   */
  public static boolean RenameGet(final String file, final int command, final String renameto)
  {
    HashMap args = new HashMap();
    args.put("renameto", renameto);
    args.put("file", file);
    LinkedList list = (LinkedList) HttpGet(args, command);
    boolean result = false;
    String inputLine;
    inputLine = (String) list.get(0);
    System.out.println("RenameGet response ; inputLine= " + inputLine);
    if (inputLine.equals("true")) result = true;
    return result;
  }
  //{{{ BoolGet() method
  /**
   * send file command in servlet via GET method .
   * return true if success and false  if otherwise
   */
  public static boolean BoolGet(final String file, final int command)
  {
    System.out.println("jEdit.BoolGet() ... file="+file+" command="+command);
    HashMap args = new HashMap();
    args.put("file", file);
    LinkedList list = (LinkedList) HttpGet(args, command);
    boolean result = false;
    String inputLine;
    inputLine = (String) list.get(0);
    if (inputLine.equals("true")) result = true;
    return result;
  }
  //{{{ LongGet() method
  /**
   * send file command in servlet via GET method .
   * return long value if success and if otherwise
   */
  public static long LongGet(final String file, final int command)
  {
    System.out.println("jEdit.LongGet() ... file="+file+" command="+command);
    HashMap args = new HashMap();
    args.put("file", file);
    LinkedList list = (LinkedList) HttpGet(args, command);
    long result;
    String inputLine;
    inputLine = (String) list.get(0);
    result = Long.parseLong(inputLine);
    return result;
  }
  //{{{ StringGet() method
  /**
   * send file command in servlet via GET method .
   * return String if success and null  if otherwise
   */
  public static String StringGet(final String file, final int command)
  {
    System.out.println("jEdit.StringGet() ... file="+file+" command="+command);
    HashMap args = new HashMap();
    args.put("file", file);
    return (String) HttpGet(args, command).get(0);
  }
//{{{ StringGet() method
  /**
   * send file command in servlet via GET method .
   * return String if success and null  if otherwise
   */
  public static String LongStringGet(final String file, final int command)
  {
    System.out.println("jEdit.StringGet() ... file="+file+" command="+command);
    HashMap args = new HashMap();
    args.put("file", file);
    LinkedList list = (LinkedList) HttpGet(args, command);
    StringBuffer buf = new StringBuffer();
    for (int i = 0; i < list.size(); i++) {
      buf.append((String) list.get(i));
      buf.append("\n");
    }
    return buf.toString();
  }

  //{{{ StringsGet() method
  /**
   * send file command in servlet via GET method .
   * return array of String if success and null  if otherwise
   */
  public static String[] StringsGet(final String file, final int command)
  {
    System.out.println("jEdit.StringsGet() ... file="+file+" command="+command);
    HashMap args = new HashMap();
    args.put("file", file);
    LinkedList list = (LinkedList) HttpGet(args, command);
    String[] names = new String[list.size()];
    for (int i = 0; i < list.size(); i++) {
      names[i] = (String) list.get(i);
      System.out.println("StringsGet" + (String) list.get(i));
    }
    return names;
  }
  //{{{ FilesGet() method
  /**
   * send file command in servlet via GET method .
   * return array of Files if success and null  if otherwise
   */
  public static File[] FilesGet(final String file, final int command)
  {
    System.out.println("jEdit.FilesGet() ... file="+file+" command="+command);
    HashMap args = new HashMap();
    args.put("file", file);
    LinkedList list = (LinkedList) HttpGet(args, command);
    File[] files = new File[list.size()];
    for (int i = 0; i < list.size(); i++) {
      files[i] = new File((String) list.get(i));
    }
//  System.out.println("FilesGet files= "+files);
    return files;
  }
  //{{{ StringGet() method
  /**
   * send file command in servlet via GET method .
   * return String if success and null  if otherwise
   */
  public static LinkedList GetNewRuleInfo(final String file, final int command)
  {
    System.out.println("jEdit.GetNewRuleInfo() ... file="+file+" command="+command);
    HashMap args = new HashMap();
    args.put("file", file);
    LinkedList list = (LinkedList) HttpGet(args, command);
    return list;
  }

  public static Object getObject(final String file,  final int command)
  {
    System.out.println("jEdit.getObject() ... file="+file+" command="+command);
    HashMap args = new HashMap();
    args.put("file", file);
    LinkedList list = (LinkedList) HttpGet(args, command);
    return list.get(0);
  }
  //{{{ HttpGet() method
  /**
   * send file command in servlet via GET method .
   * return true if success and false  if otherwise
   */
  public static List HttpGet(Map args, final int command)
  {
    System.out.println("jEdit.HttpGet() command="+command);
    URL url = null;
    HttpURLConnection urlcon = null;
    BufferedReader in = null;
    ObjectInputStream input = null;
    String content = "?username=" + jEdit.username + "&password=" + jEdit.password + "&command=" + command;
    //System.out.println("content: "+content);
    StringBuffer buf = new StringBuffer(content);
    for (Iterator it = args.keySet().iterator(); it.hasNext();) {
      String s = (String) it.next();
      buf.append("&");
      buf.append(s);
      buf.append("=");
      buf.append((String) args.get(s));
      //System.out.println("Parameters: "+s+" = " + (String) args.get(s));
    }
    content = buf.toString();
    String inputLine = "";
    LinkedList list = new LinkedList();
    try {
      url = new URL(jEdit.servletUrl, content);
      urlcon = (HttpURLConnection) url.openConnection();
      System.out.println("jEdit open connection: url="+url);
      if (command == ParseXml) {
        String status = urlcon.getHeaderField("status");
        if (!status.equals("ok")) throw new FileNotFoundException(status);
      }
      if (command == getRuleStateAndLock) {
        input = new ObjectInputStream(urlcon.getInputStream());
        list.add(input.readObject());
      } else {
        in = new BufferedReader(new InputStreamReader(urlcon.getInputStream()));

        long startTime = System.currentTimeMillis();

        while ((inputLine = in.readLine()) != null) {
            list.add(inputLine);
        }

        int currentTime=(int)(System.currentTimeMillis()-startTime);
        System.out.println("jEdit read time:"+currentTime+" ms");
          
      }
    } catch (MalformedURLException e) {
      e.printStackTrace();
/*  }  catch (ProtocolException e) {
      e.printStackTrace();
      */
    } catch (IOException e) {
      e.printStackTrace();
    } catch (ClassNotFoundException e) {
      e.printStackTrace();
    } finally {
      try {
        if (in != null) in.close();
        if (urlcon != null) urlcon.disconnect();
        if (input !=null) input.close();
      } catch (IOException e) {
        e.printStackTrace();
      }
    }

    return list;
  }
  //{{{ runStartupScripts() method
  /**
   * Runs scripts in a directory.
   */
  private static void runStartupScripts(String directoryName)
  {
    if (!BoolGet(directoryName, IsDirectory)) // if (!directory.isDirectory())
      return;
    File directory = new File(directoryName);
    File[] snippets = FilesGet(directoryName, ListFiles);//directory.listFiles();
    if (snippets == null)
      return;
    MiscUtilities.quicksort(snippets,new MiscUtilities.StringICaseCompare());

    for (int i = 0; i < snippets.length; ++i) {
      File snippet = snippets[i];
      Macros.Handler handler = Macros.getHandlerForPathName(snippet.getPath());
      if (handler == null)
        continue;

      try {
        File result = new File(MiscUtilities.constructPath(directory.getName(), snippet.getPath()));
        Macros.Macro newMacro = handler.createMacro(result.getName(),result.getPath());
        handler.runMacro(null, newMacro, false);
      } catch (Exception e) {
        e.printStackTrace(); //Log.log(Log.ERROR,jEdit.class,e);
      }
    }
  } //}}}

  //{{{ initProxy() method
  private static void initProxy()
  {
    boolean socksEnabled = jEdit.getBooleanProperty("socks.enabled");
    if (!socksEnabled) {
      // Log.log(Log.DEBUG,jEdit.class,"SOCKS proxy disabled");
      systemProperties.remove("socksProxyHost");
      systemProperties.remove("socksProxyPort");
    }
    else {
      String socksHost = jEdit.getProperty("firewall.socks.host");
      if (socksHost != null) {
        systemProperties.put("socksProxyHost", socksHost);
      }
      String socksPort = jEdit.getProperty("firewall.socks.port");
      if (socksPort != null)
        systemProperties.put("socksProxyPort", socksPort); // System.setProperty("socksProxyPort", socksPort);
    }

    boolean httpEnabled = jEdit.getBooleanProperty("firewall.enabled");
    if (!httpEnabled) {
      // Log.log(Log.DEBUG, jEdit.class, "HTTP proxy disabled");
      systemProperties.remove("proxySet");
      systemProperties.remove("proxyHost");
      systemProperties.remove("proxyPort");
      systemProperties.remove("http.proxyHost");
      systemProperties.remove("http.proxyPort");
      systemProperties.remove("http.nonProxyHosts");
      Authenticator.setDefault(null);
    }
    else {
      // set proxy host
      String host = jEdit.getProperty("firewall.host");
      if (host == null)
        return;

      systemProperties.put("http.proxyHost", host); //System.setProperty("http.proxyHost", host);
      // Log.log(Log.DEBUG, jEdit.class, "HTTP proxy enabled: " + host);
      // set proxy port
      String port = jEdit.getProperty("firewall.port");
      if (port != null)
        systemProperties.put("http.proxyPort", port);// System.setProperty("http.proxyPort", port);

      // set non proxy hosts list
      String nonProxyHosts = jEdit.getProperty("firewall.nonProxyHosts");
      if (nonProxyHosts != null)
        systemProperties.put("http.nonProxyHosts", nonProxyHosts);// System.setProperty("http.nonProxyHosts", nonProxyHosts);

      // set proxy authentication
      String username = jEdit.getProperty("firewall.user");
      String password = jEdit.getProperty("firewall.password");

      // null not supported?
      if (password == null)
        password = "";

      if (username == null || username.length() == 0) {
        // Log.log(Log.DEBUG, jEdit.class, "HTTP proxy without user");
        Authenticator.setDefault(new FirewallAuthenticator(null));
      }
      else {
        // Log.log(Log.DEBUG, jEdit.class, "HTTP proxy user: " + username);
        PasswordAuthentication pw = new PasswordAuthentication(username, password.toCharArray());
        Authenticator.setDefault(new FirewallAuthenticator(pw));
      }
    }
  } //}}}

  //{{{ FirewallAuthenticator class
  static class FirewallAuthenticator extends Authenticator
  {
    PasswordAuthentication pw;

    public FirewallAuthenticator(PasswordAuthentication pw)
    {
      this.pw = pw;
    }

    protected PasswordAuthentication getPasswordAuthentication()
    {
      return pw;
    }
  } //}}}

  //{{{ finishStartup() method
  private static void finishStartup(final boolean gui, final boolean restore,
                                    final String userDir, final String[] args,final String userFile)
  {
    SwingUtilities.invokeLater(new Runnable()
    {
      public void run()
      {
        long startTime = System.currentTimeMillis();

        Buffer buffer = openFiles(null, userDir, args);
        int count = getBufferCount();
      //todo this changed for cutting FileBrowser dialog
      /*  if (count == 0)
          newFile(null, userDir);    */
        System.out.println("before openFile userDir= "+userDir+" userFile= "+userFile);
        buffer=openFile(null, userDir, userFile , false, null);
      //  todo end
        View view = null;
        boolean restoreFiles = restore
                && jEdit.getBooleanProperty("restore")
                && (getBufferCount() == 0 ||
                jEdit.getBooleanProperty("restore.cli"));

        if (gui || count != 0) {
          view = PerspectiveManager
                  .loadPerspective(restoreFiles);

          if (view == null)
            view = newView(null, buffer);
          else if (buffer != null)
            view.setBuffer(buffer);
        }

        // Start I/O threads
        EditBus.send(new EditorStarted(null));
        System.out.println("finishStartup before VFSManager.start.. isNotReload="+isNotReload);
        System.out.println("voobshe sovsem eshe izmenil......");
        VFSManager.start();

        // Start edit server
        if (server != null)
          server.start();

        GUIUtilities.hideSplashScreen();

        Log.log(Log.MESSAGE, jEdit.class, "Startup "
                + "complete");

        //{{{ Report any plugin errors
        if (pluginErrors != null) {
          showPluginErrorDialog();
        } //}}}

        startupDone = true;

        // in one case not a single AWT class will
        // have been touched (splash screen off +
        // -nogui -nobackground switches on command
        // line)
        Toolkit.getDefaultToolkit();

        int currentTime=(int)(System.currentTimeMillis()-startTime);
        System.out.println("jEdit finish startup time:"+currentTime+" ms");   
      }
    });
  } //}}}
  //{{{ finishStartup() method
  private static void finishStartupNext(final boolean gui, final boolean restore,
                                        final String userDir,final String[] args,final String userFile)
  {
    SwingUtilities.invokeLater(new Runnable()
    {
      public void run()
      {
       long startTime = System.currentTimeMillis();

       // Buffer buffer = openFiles(null, userDir, args);
        int count = getBufferCount();
      //todo this changed for cutting FileBrowser dialog
        System.out.println("before openFile userDir= "+userDir+" userFile= "+userFile);
      String path;
      if (jEdit.getBooleanProperty("bufferWorkWithId")) path=userFile;
      else path = MiscUtilities.constructPath(userDir, userFile);
      synchronized (bufferListLock) {
      Buffer buffer = getBuffer(path);
      if (buffer != null) {
        View[] views= jEdit.getViews();
        for (int i = 0; i < views.length; i++) {
          View view = views[i];
          if (view.getBuffer()==buffer) {
             for (int j = 0; j < jars.size(); j++) {
           PluginJAR jar=(PluginJAR) jars.elementAt(j);
           jar.WindowClose(view);
         }
            jEdit.setActiveView(view); view.requestFocus();break;
          }
        }

      } //if (buffer != null)
      else {
       Hashtable props = new Hashtable();

       buffer=openFile(null, userDir, userFile , false, props);
      //  todo end
        View view = null;
        boolean restoreFiles = restore
                && jEdit.getBooleanProperty("restore")
                && (getBufferCount() == 0 ||
                jEdit.getBooleanProperty("restore.cli"));

        if (gui || count != 0) {
          view = PerspectiveManager
                  .loadPerspective(restoreFiles);

          if (view == null)
            view = newView(null, buffer);
          else if (buffer != null)
            view.setBuffer(buffer);
        }
        for (int i = 0; i < jars.size(); i++) {
           PluginJAR jar=(PluginJAR) jars.elementAt(i);
           jar.WindowOpen(view);
         }
      } //else

  } // synchronized (bufferListLock)
        // Start I/O threads
        EditBus.send(new EditorStarted(null));
        if (!startupDone) VFSManager.start();

        // Start edit server
        if (server != null)
          server.start();

        //if (lcTimer!=null) lcTimer.start();
        GUIUtilities.hideSplashScreen();

        Log.log(Log.MESSAGE, jEdit.class, "Startup "
                + "complete");

        //{{{ Report any plugin errors
        if (pluginErrors != null) {
          showPluginErrorDialog();
        } //}}}

        startupDone = true;

        // in one case not a single AWT class will
        // have been touched (splash screen off +
        // -nogui -nobackground switches on command
        // line)
        Toolkit.getDefaultToolkit();

        int currentTime=(int)(System.currentTimeMillis()-startTime);
        System.out.println("jEdit finish startup next time:"+currentTime+" ms");   
      }
    });
  } //}}}
  //{{{ finishStartup() method

  //{{{ showPluginErrorDialog() method
  private static void showPluginErrorDialog()
  {
    if (pluginErrors == null)
      return;

    String caption = getProperty("plugin-error.caption" + (pluginErrors.size() == 1 ? "-1" : ""));
    Frame frame = (PluginManager.getInstance() == null ? (Frame) viewsFirst : (Frame) PluginManager.getInstance());
    new ErrorListDialog(frame,getProperty("plugin-error.title"),caption, pluginErrors, true);
    pluginErrors = null;
  } //}}}

  //{{{ getNotLoadedPluginJARs() method
/* private static void getNotLoadedPluginJARs(Vector returnValue,
  String dir, String[] list)
 {
loop:  for(int i = 0; i < list.length; i++)
  {
   String name = list[i];
   if(!name.toLowerCase().endsWith(".jar"))
    continue loop;

   String path = MiscUtilities.constructPath(dir,name);

   for(int j = 0; j < jars.size(); j++)
   {
    PluginJAR jar = (PluginJAR)
     jars.elementAt(j);
    String jarPath = jar.getPath();
    String jarName = MiscUtilities.getFileName(jarPath);

    if(path.equals(jarPath))
     continue loop;
    else if(!new File(jarPath).exists()
     && name.equals(jarName))
     continue loop;
   }

   returnValue.addElement(path);
  }
 } //}}}
  */
  //{{{ gotoMarker() method
  private static void gotoMarker(final View view, final Buffer buffer,
                                 final String marker)
  {
    VFSManager.runInAWTThread(new Runnable()
    {
      public void run()
      {
        int pos;

        // Handle line number
        if (marker.startsWith("+line:")) {
          try {
            int line = Integer.parseInt(marker.substring(6));
            pos = buffer.getLineStartOffset(line - 1);
          } catch (Exception e) {
            return;
          }
        }
        // Handle marker
        else if (marker.startsWith("+marker:")) {
          if (marker.length() != 9)
            return;

          Marker m = buffer.getMarker(marker.charAt(8));
          if (m == null)
            return;
          pos = m.getPosition();
        }
        // Can't happen
        else
          throw new InternalError();

        if (view != null && view.getBuffer() == buffer)
          view.getTextArea().setCaretPosition(pos);
        else {
          buffer.setIntegerProperty(Buffer.CARET, pos);
          buffer.unsetProperty(Buffer.SCROLL_VERT);
        }
      }
    });
  } //}}}

  //{{{ addBufferToList() method
  private static void addBufferToList(Buffer buffer)
  {
    synchronized (bufferListLock) {
      String symlinkPath = buffer.getSymlinkPath();
      if ((VFSManager.getVFSForPath(symlinkPath).getCapabilities()
              & VFS.CASE_INSENSITIVE_CAP) != 0) {
        symlinkPath = symlinkPath.toLowerCase();
      }
      System.out.println("jEdit.addBufferToList symlinkPath= "+symlinkPath);
      // if only one, clean, 'untitled' buffer is open, we
      // replace it
      if (viewCount <= 1 && buffersFirst != null
              && buffersFirst == buffersLast
              && buffersFirst.isUntitled()
              && !buffersFirst.isDirty()) {
        Buffer oldBuffersFirst = buffersFirst;
        buffersFirst = buffersLast = buffer;
        DisplayManager.bufferClosed(oldBuffersFirst);
        EditBus.send(new BufferUpdate(oldBuffersFirst,null, BufferUpdate.CLOSED));
        bufferHash.clear();
        bufferHash.put(symlinkPath, buffer);
        return;
      }

      bufferCount++;
      bufferHash.put(symlinkPath, buffer);
      if (buffersFirst == null) {
        buffersFirst = buffersLast = buffer;
        return;
      }
      //{{{ Sort buffer list
      else if (sortBuffers) {
        String str11, str12;
        if (sortByName) {
          str11 = buffer.getName();
          str12 = buffer.getDirectory();
        }
        else {
          str11 = buffer.getDirectory();
          str12 = buffer.getName();
        }

        Buffer _buffer = buffersFirst;
        while (_buffer != null) {
          String str21, str22;
          if (sortByName) {
            str21 = _buffer.getName();
            str22 = _buffer.getDirectory();
          }
          else {
            str21 = _buffer.getDirectory();
            str22 = _buffer.getName();
          }

          int comp = MiscUtilities.compareStrings(str11, str21, true);
          if (comp < 0 || (comp == 0 && MiscUtilities.compareStrings(str12, str22, true) < 0)) {
            buffer.next = _buffer;
            buffer.prev = _buffer.prev;
            _buffer.prev = buffer;
            if (_buffer != buffersFirst)
              buffer.prev.next = buffer;
            else
              buffersFirst = buffer;
            return;
          }

          _buffer = _buffer.next;
        }
      } //}}}

      buffer.prev = buffersLast;
      // fixes the hang that can occur if we 'save as' to a
      // new filename which requires re-sorting
      buffer.next = null;
      buffersLast.next = buffer;
      buffersLast = buffer;
    }
  } //}}}

  //{{{ removeBufferFromList() method
  private static void removeBufferFromList(Buffer buffer)
  {
    synchronized (bufferListLock) {
      bufferCount--;

      String path = buffer.getPath();
      if (OperatingSystem.isCaseInsensitiveFS())
        path = path.toLowerCase();

      bufferHash.remove(path);

      if (buffer == buffersFirst && buffer == buffersLast) {
        buffersFirst = buffersLast = null;
        return;
      }

      if (buffer == buffersFirst) {
        buffersFirst = buffer.next;
        buffer.next.prev = null;
      }
      else {
        buffer.prev.next = buffer.next;
      }

      if (buffer == buffersLast) {
        buffersLast = buffersLast.prev;
        buffer.prev.next = null;
      }
      else {
        buffer.next.prev = buffer.prev;
      }

      // fixes the hang that can occur if we 'save as' to a new
      // filename which requires re-sorting
      buffer.next = buffer.prev = null;
    }
  } //}}}

  //{{{ addViewToList() method
  private static void addViewToList(View view)
  {
    viewCount++;

    if (viewsFirst == null) {
      viewsFirst = viewsLast = view;  }
    else {
      view.prev = viewsLast;
      viewsLast.next = view;
      viewsLast = view;
    }
  } //}}}

  //{{{ removeViewFromList() method
  private static void removeViewFromList(View view)
  {
    viewCount--;

    if (viewsFirst == viewsLast) {
      viewsFirst = viewsLast = null;
      return;
    }

    if (view == viewsFirst) {
      viewsFirst = view.next;
      view.next.prev = null;
    }
    else {
      view.prev.next = view.next;
    }

    if (view == viewsLast) {
      viewsLast = viewsLast.prev;
      view.prev.next = null;
    }
    else {
      view.next.prev = view.prev;
    }
  } //}}}

  //{{{ closeView() method
  /**
   * closeView() used by exit().
   */
  private static void closeView(View view, boolean callExit)
  {
    PerspectiveManager.setPerspectiveDirty(true);
    String ruleAction = (view.getBuffer().getBooleanProperty("newRule"))?addRule:editRule;
    System.out.println("before if (viewsFirst == viewsLast && callExit)  !!!!");
    if (viewsFirst == viewsLast && callExit) {
      System.out.println("before exit !!!!");
      //EditBus.send(new ViewUpdate(view, ViewUpdate.CLOSED));
      exitView(view, false, view.getBuffer().getSymlinkPath() );   }
    else {
      if (closeBuffer(view, view.getBuffer())) {
      EditBus.send(new ViewUpdate(view, ViewUpdate.CLOSED));

      view.close();
      removeViewFromList(view);

      if (view == activeView)
        activeView = null;

      setWindowClosed(ruleAction);
      //if (lcTimer!=null) lcTimer.stopLC();
      }
    }
  } //}}}

  //{{{ loadModeCatalog() method
  /**
   * Loads a mode catalog file.
   *
   * @since jEdit 3.2pre2
   */
  private static void loadModeCatalog(String path, boolean resource)
  {
    Log.log(Log.MESSAGE, jEdit.class, "Loading mode catalog file " + path);

    ModeCatalogHandler handler = new ModeCatalogHandler(MiscUtilities.getParentOfPath(path), resource);
    XmlParser parser = new XmlParser();
    parser.setHandler(handler);
    BufferedReader in = null;
    URL url;
    HttpURLConnection urlcon = null;
    try {
      InputStream _in;
      if (resource)
        _in = jEdit.class.getResourceAsStream(path);
      else {
        int command = ParseXml;
        String content = "?username=" + jEdit.username + "&password=" + jEdit.password + "&file=" + path + "&command=" + command;
        url = new URL(jEdit.servletUrl, content); //url=new URL(path);
        urlcon = (HttpURLConnection) url.openConnection();
        System.out.println("jEdit open connection: url="+url);
        _in = urlcon.getInputStream(); // _in = new FileInputStream(path);
      }
      in = new BufferedReader(new InputStreamReader(_in));
      String status = urlcon.getHeaderField("status");
      if (status.equals("ok")){
          long startTime = System.currentTimeMillis();

          parser.parse(null, null, in);

          int currentTime=(int)(System.currentTimeMillis()-startTime);
          System.out.println("jEdit parse time:"+currentTime+" ms");
      }
      else
        throw new FileNotFoundException(status);
// parser.parse(null, null, in);
    } catch (MalformedURLException e) {
      e.printStackTrace();
/*  }  catch (ProtocolException e) {
      e.printStackTrace();
      */
    } catch (IOException e) {
      e.printStackTrace();
    } catch (XmlException xe) {
      int line = xe.getLine();
      String message = xe.getMessage();
      Log.log(Log.ERROR, jEdit.class, path + ":" + line
              + ": " + message);
    } catch (Exception e) {
      e.printStackTrace();
//Log.log(Log.ERROR,jEdit.class,e);
    } finally {
      try {
        if (in != null) in.close();
        if (urlcon != null) urlcon.disconnect();
      } catch (IOException io) {
        Log.log(Log.ERROR, jEdit.class, io);
      }
    }
  } //}}}

  //{{{ initKeyBindings() method
  /**
   * Loads all key bindings from the properties.
   *
   * @since 3.1pre1
   */
  private static void initKeyBindings()
  {
    inputHandler.removeAllKeyBindings();

    ActionSet[] actionSets = getActionSets();
    for (int i = 0; i < actionSets.length; i++) {
      actionSets[i].initKeyBindings();
    }
  } //}}}

  //}}}

  //{{{ MyFocusManager class
  static class MyFocusManager extends DefaultKeyboardFocusManager
  {
    MyFocusManager()
    {
      setDefaultFocusTraversalPolicy(new LayoutFocusTraversalPolicy());
    }

    public boolean postProcessKeyEvent(KeyEvent evt)
    {
      System.out.println("jEdit.MyFocusManager.postProcessKeyEvent start");
      if (!evt.isConsumed()) {
        Component comp = (Component) evt.getSource();
        if (!comp.isShowing())
          return true;

        for (; ;) {
          if (comp instanceof View) {
            ((View) comp).processKeyEvent(evt, View.VIEW);
            return true;
          }
          else if (comp == null || comp instanceof Window
                  || comp instanceof JEditTextArea) {
            break;
          }
          else
            comp = comp.getParent();
        }
      }

      return super.postProcessKeyEvent(evt);
    }
  } //}}}

  public static HashMap getSystemProperties()
  {
    return systemProperties;
  }
}
