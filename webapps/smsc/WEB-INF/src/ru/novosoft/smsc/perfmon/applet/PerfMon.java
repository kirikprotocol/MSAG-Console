package ru.novosoft.smsc.perfmon.applet;

import java.awt.*;
import java.awt.event.*;
import java.applet.*;
import java.util.*;
import java.text.*;
import java.net.Socket;
import java.io.ObjectInputStream;
import java.io.DataInputStream;

import ru.novosoft.smsc.perfmon.PerfSnap;


public class PerfMon extends Applet implements Runnable, MouseListener, ActionListener, ItemListener {
//  PerformanceInfoPanel info_p;
//  StatInfoPanel stat_p;
  public static final int VIEWMODE_IO = 0;
  public static final int VIEWMODE_SEPARATE = 1;
  PerformanceLabel uptimeLabel;
  PerformanceLabel sctimeLabel;
  PerformanceBar perfbar;
  PerfInfoTable perfTable;
  PerformanceGraph perfGraph;
  Label connectingLabel;
  PopupMenu popupMenu;
  MenuItem menuSwitch;
  CheckboxMenuItem menuInput;
  CheckboxMenuItem menuOutput;
  CheckboxMenuItem menuSubmitErr;
  CheckboxMenuItem menuSubmit;
  CheckboxMenuItem menuRetry;
  CheckboxMenuItem menuDeliverErr;
  CheckboxMenuItem menuDeliver;

  public static ResourceBundle localeText;
  public static Locale locale;
  public static SimpleDateFormat dateFormat;
  public static SimpleDateFormat gridFormat;
  int pixPerSecond = 2;
  int scale = 200;
  int block = 8;
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

  public void init() {
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

    Thread thr = new Thread(this);
    thr.start();
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
    }
    add( popupMenu );
    perfbar = new PerformanceBar(scale, block, snap);
    perfbar.setFont(new Font("dialog", Font.PLAIN, 10));
    perfbar.addMouseListener(this);
    perfTable = new PerfInfoTable(snap);
    perfGraph = new PerformanceGraph(scale, block, pixPerSecond, vLightGrid, vMinuteGrid, snap);
    perfGraph.setFont(new Font("dialog", Font.PLAIN, 10));
    perfGraph.addMouseListener(this);

    uptimeLabel = new PerformanceLabel(snap.strUptime);
    sctimeLabel = new PerformanceLabel(snap.strSctime);


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


    lg = new LabelGroup();
    lg.setLayout(new BorderLayout());
    lg.add(perfbar, BorderLayout.CENTER);

    gbc.gridy = 2;
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
    gbc.gridy = 3;
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
        perfbar.setSnap(snap);
        perfTable.setSnap(snap);
        perfGraph.addSnap(snap);
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
    System.out.println("Mouse clicked: "+e.getButton());
    if( e.getButton() ==  MouseEvent.BUTTON3 ) {
      popupMenu.show( this, e.getX()+e.getComponent().getX(), e.getY()+e.getComponent().getY() );
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
      } else {
        viewMode = VIEWMODE_IO;
        popupMenu.remove(menuSubmit );
        popupMenu.remove(menuSubmitErr );
        popupMenu.remove(menuRetry );
        popupMenu.remove(menuDeliver );
        popupMenu.remove(menuDeliverErr );
        popupMenu.add(menuInput);
        popupMenu.add(menuOutput);
      }
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
    }
  }
}
