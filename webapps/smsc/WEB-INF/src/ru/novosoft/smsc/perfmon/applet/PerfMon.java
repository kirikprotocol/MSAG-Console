package ru.novosoft.smsc.perfmon.applet;

import java.awt.*;
import java.awt.event.*;
import java.applet.*;
import java.util.*;
import java.text.*;
import java.net.Socket;
import java.io.ObjectInputStream;
import java.io.DataInputStream;
import java.io.IOException;

import ru.novosoft.smsc.perfmon.PerfSnap;
import ru.novosoft.smsc.util.applet.LabelGroup;
import ru.novosoft.smsc.util.applet.AdvancedLabel;


public class PerfMon extends Applet implements Runnable, MouseListener, ActionListener, ItemListener {
//  PerformanceInfoPanel info_p;
//  StatInfoPanel stat_p;
  public static final int VIEWMODE_IO = 0;
  public static final int VIEWMODE_SEPARATE = 1;
  AdvancedLabel uptimeLabel;
  AdvancedLabel sctimeLabel;
  PerformanceBar perfbar;
  PerfInfoTable perfTable;
  PerfQueueTable perfQueue;
  PerformanceGraph perfGraph;
  Label connectingLabel;
  PopupMenu popupMenu;
  MenuItem menuSwitch;
  CheckboxMenuItem menuInput;
  CheckboxMenuItem menuOutput;
  CheckboxMenuItem menuSubmitErr;
  CheckboxMenuItem menuSubmit;
  CheckboxMenuItem menuRetry;
  CheckboxMenuItem menuTempErr;
  CheckboxMenuItem menuDeliverErr;
  CheckboxMenuItem menuDeliver;
  Menu     menuIncrease;
  Menu     menuDecrease;
  MenuItem menuIncrScale;
  MenuItem menuIncrBlock;
  MenuItem menuIncrPix;
  MenuItem menuDecrScale;
  MenuItem menuDecrBlock;
  MenuItem menuDecrPix;

  public static ResourceBundle localeText;
  public static Locale locale;
  public static SimpleDateFormat dateFormat;
  public static SimpleDateFormat gridFormat;
  public static int pixPerSecond = 2;
  public static int scale = 200;
  public static int block = 8;
  int vLightGrid = 4;
  int vMinuteGrid = 12;

  public static int viewMode = VIEWMODE_IO;
  public static boolean viewInputEnabled = true;
  public static boolean viewOutputEnabled = true;
  public static boolean viewSubmitEnabled = true;
  public static boolean viewSubmitErrEnabled = true;
  public static boolean viewRetryEnabled = true;
  public static boolean viewDeliverEnabled = true;
  public static boolean viewDeliverErrEnabled = true;
  public static boolean viewTempErrEnabled = true;

  public void init() {
    System.out.println("Initing...");
    locale = new Locale(getParameter("locale.country").toLowerCase(), getParameter("locale.language").toLowerCase());
    localeText = ResourceBundle.getBundle("ru.novosoft.smsc.perfmon.applet.text", locale);
    dateFormat = new SimpleDateFormat(localeText.getString("sctime"));
    gridFormat = new SimpleDateFormat(localeText.getString("gridtime"));

    try {
      pixPerSecond = Integer.valueOf(getParameter("pixPerSecond")).intValue();
    } catch(Exception ex) {
    }
    try {
      scale = Integer.valueOf(getParameter("scale")).intValue();
    } catch(Exception ex) {
    }
    try {
      block = Integer.valueOf(getParameter("block")).intValue();
    } catch(Exception ex) {
    }
    try {
      vLightGrid = Integer.valueOf(getParameter("vLightGrid")).intValue();
    } catch(Exception ex) {
    }

    try {
      vMinuteGrid = Integer.valueOf(getParameter("vMinuteGrid")).intValue();
    } catch(Exception ex) {
    }

    setFont(new Font("dialog", Font.BOLD, 12));
    setLayout(new GridBagLayout());
    setBackground(SystemColor.control);
    GridBagConstraints gbc = new GridBagConstraints();
    gbc.fill = GridBagConstraints.BOTH;

    connectingLabel = new Label(localeText.getString("connecting"));
    add(connectingLabel, gbc);

    validate();
  }

  protected void gotFirstSnap(PerfSnap snap) {
    remove(connectingLabel);

    GridBagConstraints gbc = new GridBagConstraints();
    gbc.fill = GridBagConstraints.BOTH;

    popupMenu = new PopupMenu(localeText.getString("popup.options"));
    menuSwitch = new MenuItem(localeText.getString("popup.switch"));
    menuSwitch.addActionListener(this);
    menuInput = new CheckboxMenuItem(localeText.getString("popup.input"), viewInputEnabled);
    menuInput.addItemListener(this);
    menuOutput = new CheckboxMenuItem(localeText.getString("popup.output"), viewOutputEnabled);
    menuOutput.addItemListener(this);
    menuSubmit = new CheckboxMenuItem(localeText.getString("popup.submit"), viewSubmitEnabled);
    menuSubmit.addItemListener(this);
    menuSubmitErr = new CheckboxMenuItem(localeText.getString("popup.submiterr"), viewSubmitErrEnabled);
    menuSubmitErr.addItemListener(this);
    menuRetry = new CheckboxMenuItem(localeText.getString("popup.retry"), viewRetryEnabled);
    menuRetry.addItemListener(this);
    menuDeliver = new CheckboxMenuItem(localeText.getString("popup.deliver"), viewDeliverEnabled);
    menuDeliver.addItemListener(this);
    menuDeliverErr = new CheckboxMenuItem(localeText.getString("popup.delivererr"), viewDeliverErrEnabled);
    menuDeliverErr.addItemListener(this);
    menuTempErr = new CheckboxMenuItem(localeText.getString("popup.temperr"), viewTempErrEnabled);
    menuTempErr.addItemListener(this);

    menuIncrease = new Menu(localeText.getString("popup.increase"));
    menuDecrease = new Menu(localeText.getString("popup.decrease"));

    menuIncrScale = new MenuItem(localeText.getString("popup.scale"));
    menuIncrScale.addActionListener(this);
    menuIncrBlock = new MenuItem(localeText.getString("popup.block"));
    menuIncrBlock.addActionListener(this);
    menuIncrPix = new MenuItem(localeText.getString("popup.pix"));
    menuIncrPix.addActionListener(this);
    menuDecrScale = new MenuItem(localeText.getString("popup.scale"));
    menuDecrScale.addActionListener(this);
    menuDecrBlock = new MenuItem(localeText.getString("popup.block"));
    menuDecrBlock.addActionListener(this);
    menuDecrPix = new MenuItem(localeText.getString("popup.pix"));
    menuDecrPix.addActionListener(this);

    menuIncrease.add(menuIncrScale);
    menuIncrease.add(menuIncrBlock);
    menuIncrease.add(menuIncrPix);

    menuDecrease.add(menuDecrScale);
    menuDecrease.add(menuDecrBlock);
    menuDecrease.add(menuDecrPix);

    popupMenu.add(menuIncrease);
    popupMenu.add(menuDecrease);
    popupMenu.add(menuSwitch);
    popupMenu.add(new MenuItem("-"));
    if( viewMode == VIEWMODE_IO ) {
      popupMenu.add(menuInput );
      popupMenu.add(menuOutput );
    } else {
      popupMenu.add(menuSubmit );
      popupMenu.add(menuSubmitErr );
      popupMenu.add(menuRetry );
      popupMenu.add(menuDeliver );
      popupMenu.add(menuDeliverErr );
      popupMenu.add(menuTempErr );
    }
    add( popupMenu );
    perfbar = new PerformanceBar(snap);
    perfbar.setFont(new Font("dialog", Font.PLAIN, 10));
    perfbar.addMouseListener(this);
    perfTable = new PerfInfoTable(snap);
    perfQueue = new PerfQueueTable(snap);
    perfGraph = new PerformanceGraph(vLightGrid, vMinuteGrid, snap);
    perfGraph.setFont(new Font("dialog", Font.PLAIN, 10));
    perfGraph.addMouseListener(this);

    uptimeLabel = new AdvancedLabel(snap.strUptime);
    sctimeLabel = new AdvancedLabel(snap.strSctime);


    Panel p = new Panel(new GridLayout(1, 2));
    // uptime
    LabelGroup lg = new LabelGroup(localeText.getString("lab.uptime"), LabelGroup.NORTHWEST);
    lg.setLayout(new BorderLayout());
    lg.add(uptimeLabel, BorderLayout.CENTER);
    p.add(lg);
    // sctime
    lg = new LabelGroup(localeText.getString("lab.sctime"), LabelGroup.NORTHWEST);
    lg.setLayout(new BorderLayout());
    lg.add(sctimeLabel, BorderLayout.CENTER);
    p.add(lg);

    gbc.gridx = 1;
    gbc.gridy = 1;
    gbc.gridwidth = 2;
    gbc.weighty = 0;
    gbc.fill = GridBagConstraints.HORIZONTAL;
    add(p, gbc);

    lg = new LabelGroup(localeText.getString("glab.pque"), LabelGroup.NORTHWEST);
    lg.setLayout(new BorderLayout());
    lg.add(perfQueue, BorderLayout.CENTER);

    gbc.gridx = 1;
    gbc.gridy = 2;
    gbc.gridwidth = 2;
    gbc.weighty = 0;
    gbc.fill = GridBagConstraints.HORIZONTAL;
    add(lg, gbc);

    lg = new LabelGroup();
    lg.setLayout(new BorderLayout());
    lg.add(perfbar, BorderLayout.CENTER);

    gbc.gridy = 3;
    gbc.gridx = 1;
    gbc.gridwidth = 1;
    gbc.weighty = 3;
    gbc.fill = GridBagConstraints.BOTH;
    add(lg, gbc);

    lg = new LabelGroup();
    lg.setLayout(new BorderLayout());
    lg.add(perfGraph, BorderLayout.CENTER);
    gbc.gridx = 2;
    gbc.weightx = 3;
    add(lg, gbc);

    lg = new LabelGroup(localeText.getString("glab.pinfo"), LabelGroup.NORTHWEST);
    lg.setLayout(new BorderLayout());
    lg.add(perfTable, BorderLayout.CENTER);

    gbc.gridx = 1;
    gbc.gridy = 4;
    gbc.gridwidth = 2;
    gbc.weighty = 0;
    gbc.fill = GridBagConstraints.HORIZONTAL;
    add(lg, gbc);
    validate();
  }

  boolean isStopping = false;

  public void run() {
    Socket sock = null;
    DataInputStream is = null;
    isStopping = false;
    try {
      while(!isStopping) {
        try {
          sock = new Socket(getParameter("host"), Integer.valueOf(getParameter("port")).intValue());
          is = new DataInputStream(sock.getInputStream());
          PerfSnap snap = new PerfSnap();
          snap.read(is);
          snap.calc();
          gotFirstSnap(snap);
          while(!isStopping) {
            snap.read(is);
            snap.calc();
//              System.out.println("Got snap: ls="+snap.last[PerfSnap.IDX_DELIVER]+" le="+snap.last[PerfSnap.IDX_DELIVERERR]+" upt="+snap.uptime+" tm="+(new Date(snap.sctime*1000)).toString());
            uptimeLabel.setText(snap.strUptime);
            sctimeLabel.setText(snap.strSctime);
            perfQueue.setSnap(snap);
            perfbar.setSnap(snap);
            perfTable.setSnap(snap);
            perfGraph.addSnap(snap);
          }
        } catch (IOException ex) {
          removeAll();
          GridBagConstraints gbc = new GridBagConstraints();
          gbc.fill = GridBagConstraints.BOTH;
          add(connectingLabel, gbc);
          validate();
          invalidate();
          try {
            Thread.currentThread().sleep(10000);
          } catch (InterruptedException e1) {
          }
          ex.printStackTrace( System.out );
          System.out.println( "I/O error: "+ex.getMessage()+". Reconnecting..." );
        }
      }
    } catch(Exception e) {
      e.printStackTrace();
    } finally {
      if(is != null)
        try {
          is.close();
        } catch(Exception ee) {
        }
      ;
      if(sock != null)
        try {
          sock.close();
        } catch(Exception ee) {
        }
      ;
    }
    System.out.println("Connection thread stopped");
  }

  Image offscreen;
  Object semaphore = new Object();

  public void invalidate() {
//      synchronized (semaphore) {
    offscreen = null;
//      }
    super.invalidate();
  }

  public void update(Graphics gg) {
    paint(gg);
  }

  public void paint(Graphics gg) {
    Dimension sz = getSize();
    Image screen = null;
    synchronized(semaphore) {
      screen = offscreen;
      if(offscreen == null) {
        offscreen = createImage(sz.width, sz.height);
        screen = offscreen;
      }
    }
    Graphics g = screen.getGraphics();

    super.paint(g);

    gg.drawImage(screen, 0, 0, null);
    g.dispose();
  }

  public void mouseClicked(MouseEvent e) {
    if( (e.getModifiers()&InputEvent.BUTTON3_MASK) != 0 ) {
      System.out.println("Modifiers: "+e.getModifiers());
      popupMenu.show( this, e.getX()+e.getComponent().getBounds().x, e.getY()+e.getComponent().getBounds().y );
    }
  }

  public void mousePressed(MouseEvent e) {
  }

  public void mouseReleased(MouseEvent e) {
  }

  public void mouseEntered(MouseEvent e) {
  }

  public void mouseExited(MouseEvent e) {
  }

  public void actionPerformed(ActionEvent e) {
    if( e.getSource() == this.menuSwitch ) {
      if( viewMode == VIEWMODE_IO ) {
        viewMode = VIEWMODE_SEPARATE;
        popupMenu.remove(menuInput);
        popupMenu.remove(menuOutput);
        popupMenu.add(menuSubmit );
        popupMenu.add(menuSubmitErr );
        popupMenu.add(menuRetry );
        popupMenu.add(menuDeliver );
        popupMenu.add(menuDeliverErr );
        popupMenu.add(menuTempErr );
      } else {
        viewMode = VIEWMODE_IO;
        popupMenu.remove(menuSubmit );
        popupMenu.remove(menuSubmitErr );
        popupMenu.remove(menuRetry );
        popupMenu.remove(menuDeliver );
        popupMenu.remove(menuDeliverErr );
        popupMenu.remove(menuTempErr );
        popupMenu.add(menuInput);
        popupMenu.add(menuOutput);
      }
    } else if( e.getSource() == menuIncrScale ) {
      scale+=10;
      perfbar.invalidate();
      perfGraph.invalidate();
    } else if( e.getSource() == menuIncrBlock ) {
      block++;
      perfbar.invalidate();
      perfGraph.invalidate();
    } else if( e.getSource() == menuIncrPix ) {
      pixPerSecond++;
      perfbar.invalidate();
      perfGraph.invalidate();
    } else if( e.getSource() == menuDecrScale ) {
      if( scale >= 20 ) scale-=10;
      perfbar.invalidate();
      perfGraph.invalidate();
    } else if( e.getSource() == menuDecrBlock ) {
      if( block > 4 ) block--;
      perfbar.invalidate();
      perfGraph.invalidate();
    } else if( e.getSource() == menuDecrPix ) {
      if( pixPerSecond > 2) pixPerSecond--;
      perfbar.invalidate();
      perfGraph.invalidate();
    }
  }

  public void itemStateChanged(ItemEvent e) {
    if(e.getSource() == menuInput) {
      viewInputEnabled = menuInput.getState();
    } else if(e.getSource() == menuOutput) {
      viewOutputEnabled = menuOutput.getState();
    } else if(e.getSource() == menuSubmit) {
      viewSubmitEnabled = menuSubmit.getState();
    } else if(e.getSource() == menuSubmitErr) {
      viewSubmitErrEnabled = menuSubmitErr.getState();
    } else if(e.getSource() == menuRetry) {
      viewRetryEnabled = menuRetry.getState();
    } else if(e.getSource() == menuDeliver) {
      viewDeliverEnabled = menuDeliver.getState();
    } else if(e.getSource() == menuDeliverErr) {
      viewDeliverErrEnabled = menuDeliverErr.getState();
    } else if(e.getSource() == menuTempErr) {
      viewTempErrEnabled = menuTempErr.getState();
    }
  }

  public void start() {
    System.out.println("Starting...");
    Thread thr = new Thread(this);
    thr.start();
  }

  public void stop() {
    System.out.println("Stoping...");
    isStopping = true;
  }

  public void destroy() {
    System.out.println("Destroying...");
    isStopping = true;
  }

}
