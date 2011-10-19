package ru.novosoft.smsc.web.applets.topmon;


import ru.novosoft.smsc.admin.topmon.TopSnap;
import ru.novosoft.smsc.util.applet.LabelGroup;

import java.applet.Applet;
import java.awt.*;
import java.awt.event.*;
import java.io.DataInputStream;
import java.io.IOException;
import java.net.Socket;
import java.util.Locale;
import java.util.ResourceBundle;

/**
 * Created by: Serge Lugovoy
 * Date: 14.05.2004
 * Time: 14:41:40
 */
public class TopMon extends Applet implements Runnable, MouseListener, ActionListener, ItemListener {
  private static ResourceBundle localeText;
  private static ResourceBundle messagesText;
  private static Locale locale;
  private Label connectingLabel;
  private SmeTopGraph smeTopGraph;
  //  private ErrTopGraph errTopGraph;
//  private ScreenSplitter screenSplitter;
  private int maxSpeed = 100;
  private int graphScale = 2;
  private int graphGrid = 5;
  private int graphHiGrid = 25;
  private int graphHead = 50;

  public void init() {
    System.out.println("Initing...");
    locale = new Locale(getParameter("locale.language").toLowerCase());
    localeText = ResourceBundle.getBundle("ru.novosoft.smsc.web.applets.topmon.text", locale);
    messagesText = ResourceBundle.getBundle("locales.messages", locale);
    try{
      maxSpeed = Integer.valueOf(getParameter("max.speed"));
    }catch (Exception ignored){}

    try{
      graphScale = Integer.valueOf(getParameter("graph.scale"));
    }catch (Exception ignored){}

    try{
      graphGrid = Integer.valueOf(getParameter("graph.grid"));
    }catch (Exception ignored){}

    try{
      graphHiGrid = Integer.valueOf(getParameter("grap37h.higrid"));
    }catch (Exception ignored){}

    try{
      graphHead = Integer.valueOf(getParameter("graph.head"));
    }catch (Exception ignored){}

    setFont(new Font("dialog", Font.BOLD, 12));
    setLayout(new GridBagLayout());
    setBackground(SystemColor.control);
    GridBagConstraints gbc = new GridBagConstraints();
    gbc.fill = GridBagConstraints.BOTH;
    connectingLabel = new Label(localeText.getString("connecting"));
    add(connectingLabel, gbc);
    validate();
  }

  void gotFirstSnap(TopSnap snap) {
    remove(connectingLabel);
    GridBagConstraints gbc = new GridBagConstraints();
    gbc.fill = GridBagConstraints.BOTH;
    SnapHistory snapHistory = new SnapHistory();
    smeTopGraph = new SmeTopGraph(snap, maxSpeed, graphScale, graphGrid, graphHiGrid, graphHead, localeText, messagesText, snapHistory);
//    errTopGraph = new ErrTopGraph(snap);

    LabelGroup lgSme = new LabelGroup(localeText.getString("sme.top.label"), LabelGroup.NORTHWEST);
    lgSme.setLayout(new BorderLayout());
    lgSme.add(smeTopGraph, BorderLayout.CENTER);

/*    LabelGroup lgErr = new LabelGroup(localeText.getString("err.top.label"), LabelGroup.NORTHWEST);
    lgErr.setLayout(new BorderLayout());
    lgErr.add(errTopGraph, BorderLayout.CENTER);

    screenSplitter = new ScreenSplitter(lgSme, lgErr);
    screenSplitter.setFocusable(false);
*/
    gbc.gridy = 1;
    gbc.gridx = 1;
    gbc.gridwidth = 1;
    gbc.weightx = 1;
    gbc.weighty = 1;
    gbc.fill = GridBagConstraints.BOTH;
//    add(screenSplitter, gbc);
    //add(lgSme, gbc);
    ScrollPane pane = new ScrollPane();
    pane.add(lgSme, BorderLayout.CENTER);
    add(pane, gbc);
    smeTopGraph.requestFocus();

    validate();
  }

  private boolean isStopping = false;

  public void run() {
    Socket sock = null;
    DataInputStream is = null;
    isStopping = false;
    try {
/*      TopSnap snap = new TopSnap();
      gotFirstSnap(snap); */
      while (!isStopping) {
/*        synchronized(this){
          try {
            wait(1000);
          } catch (InterruptedException e) {
          }
        }*/
        try {
          sock = new Socket(getParameter("host"), Integer.valueOf(getParameter("port")));
          is = new DataInputStream(sock.getInputStream());
          TopSnap snap = new TopSnap();
          snap.read(is);
          gotFirstSnap(snap);
          while (!isStopping) {
            snap.read(is);
            smeTopGraph.setSnap(snap);
//              System.out.println("Got snap: ls="+snap.last[PerfSnap.IDX_DELIVER]+" le="+snap.last[PerfSnap.IDX_DELIVERERR]+" upt="+snap.uptime+" tm="+(new Date(snap.sctime*1000)).toString());
          }
        } catch (IOException ex) {
          removeAll();
          GridBagConstraints gbc = new GridBagConstraints();
          gbc.fill = GridBagConstraints.BOTH;
          add(connectingLabel, gbc);
          validate();
          invalidate();
          try {
            Thread.sleep(10000);
          } catch (InterruptedException ignored) {
          }
          ex.printStackTrace(System.out);
          System.out.println("I/O error: " + ex.getMessage() + ". Reconnecting...");
        }
      }
    } catch (Exception e) {
      e.printStackTrace();
    } finally {
      if (is != null)
        try {
          is.close();
        } catch (Exception ignored) {
        }
      if (sock != null)
        try {
          sock.close();
        } catch (Exception ignored) {
        }
    }
    System.out.println("Connection thread stopped");
  }

  private Image offscreen;
  private final Object semaphore = new Object();

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
    Image screen;
    synchronized (semaphore) {
      screen = offscreen;
      if (offscreen == null) {
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
  }

  public void itemStateChanged(ItemEvent e) {
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
