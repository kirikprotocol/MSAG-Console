package ru.sibinco.scag.perfmon.applet;

//import org.apache.log4j.Logger;
import ru.sibinco.lib.backend.applet.AdvancedLabel;
import ru.sibinco.lib.backend.applet.LabelGroup;
import ru.sibinco.scag.perfmon.PerfSnap;
import ru.sibinco.scag.util.RemoteResourceBundle;

import javax.swing.*;
import javax.swing.event.ChangeEvent;
import javax.swing.event.ChangeListener;
import java.applet.Applet;
import java.awt.*;
import java.awt.event.*;
import java.io.*;
import java.net.Socket;
import java.text.SimpleDateFormat;
import java.util.Locale;

public class PerfMon extends Applet implements Runnable, MouseListener, ActionListener, ItemListener {

//    private final Logger logger = Logger.getLogger(this.getClass());

    public static final int VIEWMODE_IO = 0;
    public static final int VIEWMODE_SEPARATE = 1;
    public static final String smppStatMode = "smpp";
    public static final String httpStatMode = "http";
    public static final String mmsStatMode = "mms";
    public static String statMode = smppStatMode;

    AdvancedLabel uptimeLabel;
    AdvancedLabel sctimeLabel;

    AdvancedLabel sessionCountLabel;
    AdvancedLabel sessionLoadedCountLabel;
    AdvancedLabel sessionLockedCountLabel;

    AdvancedLabel sessionCountLabelTitle;
    AdvancedLabel sessionLoadedCountLabelTitle;
    AdvancedLabel sessionLockedCountLabelTitle;

    AdvancedLabel queueReqLabel;
    AdvancedLabel queueResLabel;
    AdvancedLabel queueLCMLabel;
    AdvancedLabel queueReqLabelTitle;
    AdvancedLabel queueResLabelTitle;
    AdvancedLabel queueLCMLabelTitle;

    PerformanceBar perfbar;
    PerfInfoTable perfTable;
    PerformanceGraph perfGraph;
    Label connectingLabel;
    PopupMenu popupMenu;
    MenuItem menuSwitch;

    CheckboxMenuItem menuInput;
    CheckboxMenuItem menuOutput;
    CheckboxMenuItem menuTransaction;

    //smpp
    CheckboxMenuItem menuAccepted;
    CheckboxMenuItem menuRejected;
    CheckboxMenuItem menuDelivered;
    CheckboxMenuItem menuGwRejected;
    CheckboxMenuItem menuFailed;

    //http
    CheckboxMenuItem menuRequest;
    CheckboxMenuItem menuRequestRejected;
    CheckboxMenuItem menuResponse;
    CheckboxMenuItem menuResponseRejected;
    CheckboxMenuItem menuDeliveredHTTP;
    CheckboxMenuItem menuFailedHTTP;

    //MenuItem statisticsMode;
    Menu menuIncrease;
    Menu menuDecrease;
    MenuItem menuIncrScale;
    MenuItem menuIncrBlock;
    MenuItem menuIncrPix;
    MenuItem menuDecrScale;
    MenuItem menuDecrBlock;
    MenuItem menuDecrPix;

    public static RemoteResourceBundle localeText;
    public static Locale locale;
    public static SimpleDateFormat dateFormat;
    public static SimpleDateFormat gridFormat;
    public static int pixPerSecond = 2;
    public static int scale = 200;
    public static int block = 8;
    int vLightGrid = 4;
    int vMinuteGrid = 12;
    ContextInfo contextInfo;

    public static int viewMode = VIEWMODE_IO;
    public static boolean viewInputEnabled = true;
    public static boolean viewOutputEnabled = true;
    public static boolean viewTransactionEnabled = true;
    //smpp
    public static boolean viewAcceptedEnabled = true;
    public static boolean viewRejectedEnabled = true;
    public static boolean viewDeliveredEnabled = true;
    public static boolean viewGwRejectedEnabled = true;
    public static boolean viewFailedEnabled = true;
    //http
    public static boolean viewRequestEnabled = true;
    public static boolean viewRequestRejectedEnabled = true;
    public static boolean viewResponseEnabled = true;
    public static boolean viewResponseRejectedEnabled = true;
    public static boolean viewDeliveredHTTPEnabled = true;
    public static boolean viewFailedHTTPEnabled = true;

    String test;

    public void init() {
        System.out.println("new PerfMon.init() Initing...");
        localeText = new RemoteResourceBundle(getCodeBase(),getParameter("resource_servlet_uri"));
        locale=localeText.getLocale();
        dateFormat = new SimpleDateFormat(localeText.getString("sctime"),locale);
        gridFormat = new SimpleDateFormat(localeText.getString("gridtime"),locale);

        //http
        try {
            pixPerSecond = Integer.valueOf(getParameter("http.pixPerSecond")).intValue();
        } catch (Exception ex) {
        }
        try {
            scale = Integer.valueOf(getParameter("http.scale")).intValue();
        } catch (Exception ex) {
        }
        try {
            block = Integer.valueOf(getParameter("http.block")).intValue();
        } catch (Exception ex) {
        }
        contextInfo = new ContextInfo();
        contextInfo.initHttp();

        //smpp
        try {
            pixPerSecond = Integer.valueOf(getParameter("smpp.pixPerSecond")).intValue();
        } catch (Exception ex) {
        }
        try {
            scale = Integer.valueOf(getParameter("smpp.scale")).intValue();
        } catch (Exception ex) {
        }
        try {
            block = Integer.valueOf(getParameter("smpp.block")).intValue();
        } catch (Exception ex) {
        }

        contextInfo.initSmpp();
        try {
            vLightGrid = Integer.valueOf(getParameter("vLightGrid")).intValue();
        } catch (Exception ex) {
        }

        try {
            vMinuteGrid = Integer.valueOf(getParameter("vMinuteGrid")).intValue();
        } catch (Exception ex) {
        }

        test = "qwe";
        try {
            test = getParameter("test");
        } catch (Exception ex) {
        }

        System.out.println( "PerfMon.init() test='" + test + "'" );

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
        //System.out.println("gotFirstSnap() start");
        remove(connectingLabel);

        GridBagConstraints gbc = new GridBagConstraints();
        gbc.fill = GridBagConstraints.BOTH;

        popupMenu = new PopupMenu(localeText.getString("popup.options"));
        menuSwitch = new MenuItem(localeText.getString("popup.switch"));
        menuSwitch.addActionListener(this);
        //statisticsMode = new MenuItem(localeText.getString("popup.statistics_mode")+" "+ (statMode.equals(smppStatMode)?localeText.getString("popup.http"):localeText.getString("popup.smpp")));
        //statisticsMode.addActionListener(this);

        menuInput = new CheckboxMenuItem(localeText.getString("popup.input"), viewInputEnabled);
        menuInput.addItemListener(this);
        menuOutput = new CheckboxMenuItem(localeText.getString("popup.output"), viewOutputEnabled);
        menuOutput.addItemListener(this);
        menuTransaction = new CheckboxMenuItem(localeText.getString("popup.transaction"), viewTransactionEnabled);
        menuTransaction.addItemListener(this);

        //smpp
        menuAccepted = new CheckboxMenuItem(localeText.getString("popup.smpp.accepted"), viewAcceptedEnabled);
        menuAccepted.addItemListener(this);
        menuRejected = new CheckboxMenuItem(localeText.getString("popup.smpp.rejected"), viewRejectedEnabled);
        menuRejected.addItemListener(this);
        menuDelivered = new CheckboxMenuItem(localeText.getString("popup.smpp.delivered"), viewDeliveredEnabled);
        menuDelivered.addItemListener(this);
        menuGwRejected = new CheckboxMenuItem(localeText.getString("popup.smpp.gwrejected"), viewGwRejectedEnabled);
        menuGwRejected.addItemListener(this);
        menuFailed = new CheckboxMenuItem(localeText.getString("popup.smpp.failed"), viewFailedEnabled);
        menuFailed.addItemListener(this);

        //http
        menuRequest = new CheckboxMenuItem(localeText.getString("popup.http.request"), viewRequestEnabled);
        menuRequest.addItemListener(this);
        menuRequestRejected = new CheckboxMenuItem(localeText.getString("popup.http.requestrejected"), viewRequestRejectedEnabled);
        menuRequestRejected.addItemListener(this);
        menuResponse = new CheckboxMenuItem(localeText.getString("popup.http.response"), viewResponseEnabled);
        menuResponse.addItemListener(this);
        menuResponseRejected = new CheckboxMenuItem(localeText.getString("popup.http.responserejected"), viewResponseRejectedEnabled);
        menuResponseRejected.addItemListener(this);
        menuDeliveredHTTP = new CheckboxMenuItem(localeText.getString("popup.http.delivered"), viewDeliveredHTTPEnabled);
        menuDeliveredHTTP.addItemListener(this);
        menuFailedHTTP = new CheckboxMenuItem(localeText.getString("popup.http.failed"), viewFailedHTTPEnabled);
        menuFailedHTTP.addItemListener(this);

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
        //popupMenu.add(statisticsMode);
        popupMenu.add(menuSwitch);
        popupMenu.add(new MenuItem("-"));
        if (viewMode == VIEWMODE_IO) {
            popupMenu.add(menuInput);
            popupMenu.add(menuOutput);
            popupMenu.add(menuTransaction);
        } else {
            if (statMode.equals(smppStatMode)) {
              popupMenu.add(menuAccepted);
              popupMenu.add(menuRejected);
              popupMenu.add(menuDelivered);
              popupMenu.add(menuGwRejected);
              popupMenu.add(menuFailed);
            } else {
              popupMenu.add(menuRequest);
              popupMenu.add(menuRequestRejected);
              popupMenu.add(menuResponse);
              popupMenu.add(menuResponseRejected);
              popupMenu.add(menuDeliveredHTTP);
              popupMenu.add(menuFailedHTTP);
            }
        }
        add(popupMenu);
        perfbar = new PerformanceBar(snap);
        perfbar.setFont(new Font("dialog", Font.PLAIN, 10));
        perfbar.addMouseListener(this);
        perfTable = new PerfInfoTable(snap);
        perfGraph = new PerformanceGraph(vLightGrid, vMinuteGrid, snap);
        perfGraph.setFont(new Font("dialog", Font.PLAIN, 10));
        perfGraph.addMouseListener(this);

        uptimeLabel = new AdvancedLabel(snap.strUptime);
        sctimeLabel = new AdvancedLabel(snap.strSctime);

        sessionCountLabel = new AdvancedLabel(snap.strSessionCount);
        sessionLoadedCountLabel = new AdvancedLabel(snap.strSessionLoadedCount);
	    sessionLockedCountLabel = new AdvancedLabel(snap.strSessionLockedCount);

        sessionCountLabelTitle = new AdvancedLabel( localeText.getString("lab.sesscount.total") );
        sessionLoadedCountLabelTitle = new AdvancedLabel( localeText.getString("lab.sesscount.loaded") );
        sessionLockedCountLabelTitle = new AdvancedLabel( localeText.getString("lab.sesscount.locked") );
//        sessionCountLabelTitle = new AdvancedLabel( "Total" );
//	    sessionLockedCountLabelTitle = new AdvancedLabel( "Locked" );

        queueReqLabel = new AdvancedLabel("");
    	queueResLabel = new AdvancedLabel("");
	    queueLCMLabel = new AdvancedLabel("");
        queueReqLabelTitle = new AdvancedLabel( localeText.getString("lab.queuelimit.req") );
        queueResLabelTitle = new AdvancedLabel( localeText.getString("lab.queuelimit.res") );
        queueLCMLabelTitle = new AdvancedLabel( localeText.getString("lab.queuelimit.lcm") );
//        queueReqLabelTitle = new AdvancedLabel( "Req" );
//        queueResLabelTitle = new AdvancedLabel( "Res" );
//        queueLCMLabelTitle = new AdvancedLabel( "Lcm" );




        Panel p = new Panel(new GridLayout(2, 2));
// uptime
        LabelGroup lg = new LabelGroup(localeText.getString("lab.uptime"), LabelGroup.NORTHWEST);
//        lg = new LabelGroup( test, LabelGroup.NORTHWEST);
        lg.setLayout(new BorderLayout());
        lg.add(uptimeLabel, BorderLayout.CENTER);
        p.add(lg);
// sctime
        lg = new LabelGroup(localeText.getString("lab.sctime"), LabelGroup.NORTHWEST);
        lg.setLayout(new BorderLayout());
        lg.add(sctimeLabel, BorderLayout.CENTER);
        p.add(lg);
// sesscount
        lg = new LabelGroup(localeText.getString("lab.sesscount"), LabelGroup.NORTHWEST);
    	lg.setLayout(new BorderLayout());
    	Panel pSession = new Panel(new GridLayout(1, 3));

        Panel pSessionTotal = new Panel(new BorderLayout());
        pSessionTotal.add(sessionCountLabelTitle, BorderLayout.WEST);
        pSessionTotal.add(sessionCountLabel, BorderLayout.CENTER);

        Panel pSessionLoaded = new Panel(new BorderLayout());
        pSessionLoaded.add(sessionLoadedCountLabelTitle, BorderLayout.WEST);
        pSessionLoaded.add(sessionLoadedCountLabel, BorderLayout.CENTER);

        Panel pSessionLocked = new Panel(new BorderLayout());
        pSessionLocked.add(sessionLockedCountLabelTitle, BorderLayout.WEST);
        pSessionLocked.add(sessionLockedCountLabel, BorderLayout.CENTER);

        pSession.add(pSessionTotal);
        pSession.add(pSessionLoaded);
        pSession.add(pSessionLocked);

        lg.add(pSession, BorderLayout.CENTER);
        p.add(lg);
// queues sizes
        lg = new LabelGroup(localeText.getString("lab.queuelimit"), LabelGroup.NORTHWEST);
    	lg.setLayout(new BorderLayout());
	    Panel pQueue = new Panel(new GridLayout(1, 3));

        Panel pQueueReq = new Panel(new BorderLayout());
        pQueueReq.add(queueReqLabelTitle, BorderLayout.WEST);
        pQueueReq.add(queueReqLabel, BorderLayout.CENTER);

        Panel pQueueRes = new Panel(new BorderLayout());
        pQueueRes.add(queueResLabelTitle, BorderLayout.WEST);
        pQueueRes.add(queueResLabel, BorderLayout.CENTER);

        Panel pQueueLcm = new Panel(new BorderLayout());
        pQueueLcm.add(queueLCMLabelTitle, BorderLayout.WEST);
        pQueueLcm.add(queueLCMLabel, BorderLayout.CENTER);

        pQueue.add(pQueueReq);
        pQueue.add(pQueueRes);
        pQueue.add(pQueueLcm);

        lg.add(pQueue, BorderLayout.CENTER);
    	p.add(lg);

        gbc.gridx = 1;
        gbc.gridy = 1;
        gbc.gridwidth = 2;
        gbc.weighty = 0;
        gbc.fill = GridBagConstraints.HORIZONTAL; //BOTH ?;
        add(p, gbc);

        final JTabbedPane jTabbedPane = new JTabbedPane();
        jTabbedPane.addTab("SMPP", new SmppPanel());
        jTabbedPane.addTab("HTTP", new HttpPanel());
//        jTabbedPane.addTab("MMS", new MmsPanel());
        jTabbedPane.addChangeListener(new ChangeListener() {
           public void stateChanged(ChangeEvent e) {
             CommonPanel selectedTab = (CommonPanel)jTabbedPane.getSelectedComponent();
             for(int i = 0; i<jTabbedPane.getTabCount();i++)
              ((CommonPanel)jTabbedPane.getComponentAt(i)).removeAll();
             selectedTab.init();
             selectedTab.changeStatMode();
           }
        });

        gbc.gridy = 2;
        gbc.gridx = 1;
        gbc.gridwidth = 1;
        gbc.weightx = 1;
        gbc.weighty = 1;
        gbc.fill = GridBagConstraints.BOTH;
        add(jTabbedPane, gbc);
        validate();
    }

    private abstract class CommonPanel extends JPanel {
      public abstract void changeStatMode();

      public void init() {
          setLayout(new GridBagLayout());
          LabelGroup lg = new LabelGroup();
          lg.setLayout(new BorderLayout());
          lg.add(perfbar, BorderLayout.CENTER);
          GridBagConstraints gbc = new GridBagConstraints();

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
      }
    }

    private class SmppPanel extends CommonPanel {
        public SmppPanel() {
           init();
        }
        public void changeStatMode() {
            statMode = smppStatMode;
            contextInfo.setContextInfo(statMode,httpStatMode);
            perfbar.invalidate();
            perfGraph.invalidate();
            if (viewMode == VIEWMODE_SEPARATE) {
                  popupMenu.remove(menuRequest);
                  popupMenu.remove(menuRequestRejected);
                  popupMenu.remove(menuResponse);
                  popupMenu.remove(menuResponseRejected);
                  popupMenu.remove(menuDeliveredHTTP);
                  popupMenu.remove(menuFailedHTTP);
                  popupMenu.add(menuAccepted);
                  popupMenu.add(menuRejected);
                  popupMenu.add(menuDelivered);
                  popupMenu.add(menuGwRejected);
                  popupMenu.add(menuFailed);
             }
        }
    }

    private class HttpPanel extends CommonPanel {
        public void changeStatMode() {
           statMode = httpStatMode;
           contextInfo.setContextInfo(statMode,smppStatMode);
           perfbar.invalidate();
           perfGraph.invalidate();
           if (viewMode == VIEWMODE_SEPARATE) {
              popupMenu.remove(menuAccepted);
              popupMenu.remove(menuRejected);
              popupMenu.remove(menuDelivered);
              popupMenu.remove(menuGwRejected);
              popupMenu.remove(menuFailed);
              popupMenu.add(menuRequest);
              popupMenu.add(menuRequestRejected);
              popupMenu.add(menuResponse);
              popupMenu.add(menuResponseRejected);
              popupMenu.add(menuDeliveredHTTP);
              popupMenu.add(menuFailedHTTP);
          }
        }
    }

    boolean isStopping = false;

    public void run() {
        Socket sock = null;
        DataInputStream is = null;
        isStopping = false;
        try {
            while (!isStopping) {
                try {
                    sock = new Socket(getParameter("host"), Integer.valueOf(getParameter("port")).intValue());
                    //System.out.println( "PerfMon:run():host=" + getParameter("host") + " port=" + Integer.valueOf(getParameter("port")).intValue() );
                    is = new DataInputStream( sock.getInputStream() );
                    PerfSnap snap = new PerfSnap();
                    snap.read(is);
                    snap.calc();
                    gotFirstSnap(snap);
                    while (!isStopping) {
                        snap.read(is);
                        snap.calc();
                        uptimeLabel.setText(snap.strUptime);
                        sctimeLabel.setText(snap.strSctime);
                        sessionCountLabel.setText( snap.strSessionCount );
                        sessionLoadedCountLabel.setText( snap.strSessionLoadedCount );
			            sessionLockedCountLabel.setText( snap.strSessionLockedCount );
                        if( statMode.equals(httpStatMode) ){
                            queueReqLabel.setText(snap.strHttpReqQueueLen);
			                queueResLabel.setText(snap.strHttpResQueueLen);
			                queueLCMLabel.setText(snap.strHttpLCMQueueLen);
                        } else if( statMode.equals(smppStatMode) ) {
                            queueReqLabel.setText(snap.strSmppReqQueueLen);
        		    	    queueResLabel.setText(snap.strSmppResQueueLen);
		        	        queueLCMLabel.setText(snap.strSmppLCMQueueLen);
                        } else {
                            queueReqLabel.setText("");
    			            queueResLabel.setText("");
	            		    queueLCMLabel.setText("");
                        }
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
                        Thread.sleep(10000);
                    } catch (InterruptedException e1) {
                    }
                    ex.printStackTrace(System.out);
                    //System.out.println("I/O error: " + ex.getMessage() + ". Reconnecting...");
                }
            }
        } catch (Exception e) {
            e.printStackTrace();
        } finally {
            if (is != null)
                try {
                    is.close();
                } catch (Exception ee) {
                }
            ;
            if (sock != null)
                try {
                    sock.close();
                } catch (Exception ee) {
                }
            ;
        }
        //System.out.println("Connection thread stopped");
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
        if ((e.getModifiers() & InputEvent.BUTTON3_MASK) != 0) {
            //System.out.println("Modifiers: " + e.getModifiers());
            popupMenu.show(this, e.getX() + e.getComponent().getBounds().x, e.getY() + e.getComponent().getBounds().y);
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
        if (e.getSource() == this.menuSwitch) {
            if (viewMode == VIEWMODE_IO) {
                viewMode = VIEWMODE_SEPARATE;
                popupMenu.remove(menuInput);
                popupMenu.remove(menuOutput);
                popupMenu.remove(menuTransaction);
                if (statMode.equals(smppStatMode)) {
                  popupMenu.add(menuAccepted);
                  popupMenu.add(menuRejected);
                  popupMenu.add(menuDelivered);
                  popupMenu.add(menuGwRejected);
                  popupMenu.add(menuFailed);
                } else {
                  popupMenu.add(menuRequest);
                  popupMenu.add(menuRequestRejected);
                  popupMenu.add(menuResponse);
                  popupMenu.add(menuResponseRejected);
                  popupMenu.add(menuDeliveredHTTP);
                  popupMenu.add(menuFailedHTTP);
                }
            } else {
                viewMode = VIEWMODE_IO;
                if (statMode.equals(smppStatMode)) {
                  popupMenu.remove(menuAccepted);
                  popupMenu.remove(menuRejected);
                  popupMenu.remove(menuDelivered);
                  popupMenu.remove(menuGwRejected);
                  popupMenu.remove(menuFailed);
                } else {
                  popupMenu.remove(menuRequest);
                  popupMenu.remove(menuRequestRejected);
                  popupMenu.remove(menuResponse);
                  popupMenu.remove(menuResponseRejected);
                  popupMenu.remove(menuDeliveredHTTP);
                  popupMenu.remove(menuFailedHTTP);
                }
                popupMenu.add(menuInput);
                popupMenu.add(menuOutput);
                popupMenu.add(menuTransaction);
            }
        } else if (e.getSource() == menuIncrScale) {
            //System.out.println("menuIncrScale");
            scale += 10;
            perfbar.invalidate();
            perfGraph.invalidate();
        } else if (e.getSource() == menuIncrBlock) {
            //System.out.println("menuIncrBlock");
            block++;
            perfbar.invalidate();
            perfGraph.invalidate();
        } else if (e.getSource() == menuIncrPix) {
            //System.out.println("menuIncrPix");
            pixPerSecond++;
            perfbar.invalidate();
            perfGraph.invalidate();
        } else if (e.getSource() == menuDecrScale) {
            //System.out.println("menuDecrScale");
            if (scale >= 20) scale -= 10;
            perfbar.invalidate();
            perfGraph.invalidate();
        } else if (e.getSource() == menuDecrBlock) {
            //System.out.println("menuDecrBlock");
            if (block > 4) {
                //System.out.println("menuDecrBlock: block>4");
                block--;
            }
            perfbar.invalidate();
            perfGraph.invalidate();
        } else if (e.getSource() == menuDecrPix) {
            //System.out.println("menuDecrPix");
            if (pixPerSecond > 2) pixPerSecond--;
            perfbar.invalidate();
            perfGraph.invalidate();
        }
    }

    public void itemStateChanged(ItemEvent e) {
        if (e.getSource() == menuInput) {
            viewInputEnabled = menuInput.getState();
        } else if (e.getSource() == menuOutput) {
            viewOutputEnabled = menuOutput.getState();
        } else if (e.getSource() == menuTransaction) {
            viewTransactionEnabled = menuTransaction.getState();
        } //smpp
        else if (e.getSource() == menuAccepted) {
            viewAcceptedEnabled = menuAccepted.getState();
        } else if (e.getSource() == menuRejected) {
            viewRejectedEnabled = menuRejected.getState();
        } else if (e.getSource() == menuFailed) {
            viewFailedEnabled = menuFailed.getState();
        } else if (e.getSource() == menuDelivered) {
            viewDeliveredEnabled = menuDelivered.getState();
        } else if (e.getSource() == menuGwRejected) {
            viewGwRejectedEnabled = menuGwRejected.getState();
        } //http
        else if (e.getSource() == menuRequest) {
          viewRequestEnabled = menuRequest.getState();
        } else if (e.getSource() == menuRequestRejected) {
          viewRequestRejectedEnabled = menuRequestRejected.getState();
        } else if (e.getSource() == menuResponse) {
          viewResponseEnabled = menuResponse.getState();
        } else if (e.getSource() == menuResponseRejected) {
          viewResponseRejectedEnabled = menuResponseRejected.getState();
        } else if (e.getSource() == menuDeliveredHTTP) {
          viewDeliveredHTTPEnabled = menuDeliveredHTTP.getState();
        } else if (e.getSource() == menuFailedHTTP) {
          viewFailedHTTPEnabled = menuFailedHTTP.getState();
        }
    }

    public void start() {
        System.out.println("Performance monitor starting...");
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