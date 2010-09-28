/*
 * Copyright (c) 2005 SibInco Inc. All Rights Reserved.
 */
package ru.sibinco.scag.svcmon.applet;

//import static ru.sibinco.scag.svcmon.applet.SvcMon.TYPE_SMPP;
//import static java.lang.Thread.currentThread;
import ru.sibinco.scag.svcmon.SvcSnap;
import ru.sibinco.scag.util.RemoteResourceBundle;
//import ru.sibinco.scag.scmon.snap.SmppSnap;
import ru.sibinco.scag.svcmon.snap.SmppSnap;
import ru.sibinco.scag.svcmon.snap.HttpSnap;

import javax.swing.*;
import java.applet.Applet;
import java.awt.*;
import java.awt.event.*;
import java.io.DataInputStream;
import java.io.IOException;
import java.net.Socket;
import java.util.Locale;
import java.util.HashSet;
import java.io.InputStream;
import java.util.Calendar;
import java.util.Observable;
import java.util.Observer;
/**
 * The <code>SvcMon</code> class represents
 * <p><p/>
 * Date: 08.12.2005
 * Time: 17:14:20
 *
 * @author &lt;a href="mailto:igor@sibinco.ru"&gt;Igor Klimenko&lt;/a&gt;
 */

class Pause extends Observable{

    private boolean state = false;

    public void setState( boolean state ){
        this.state = state;
        setChanged();
        notifyObservers();
    }

    public boolean getState(){
        return state;
    }
}

public class SvcMon extends Applet implements Runnable, MouseListener, ActionListener, ItemListener {

    public static RemoteResourceBundle localText;
    public static Locale locale;
    private Label connectingLabel;
    private SnapSmppHistory snapSmppHistory;
    private SnapHttpHistory snapHttpHistory;
//    private SmppTopGraph smppTopGraph;
    SmppTopGraph smppTopGraph;
    HttpTopGraph httpTopGraph;
    private ScreenSplitter screenSplitter;

    private int maxSpeed = 100;
    private int graphScale = 1;
    private int graphGrid = 5;
    private int graphHiGrid = 25;
    private int graphHead = 50;

    SvcSnap svcSnap;

    public static final String TYPE_SMPP = "SMPP";
    public static final String TYPE_HTTP = "HTTP";
    public static final String TYPE_MMS = "MMS";

    public static final int SCALE_STEP = 1;

    public HashSet smppViewList;
    public HashSet httpViewList;

    public float smppXScale = 5;
    public double smppYScale = 1;
    public float httpXScale = 5;
    public double httpYScale = 1;


//    private boolean hard = false;
    boolean smppViewGraph = false;
    boolean httpViewGraph = false;
    boolean smppPauseGraph = false;
    boolean httpPauseGraph = false;

    Pause pause = new Pause();

    int smppShiftV = 0;
    int httpShiftV = 0;

    Calendar smppPauseTime;
    Calendar httpPauseTime;

    String TEST_BUTTON = "TEST";
    boolean start = true;

    public static final String BUTTON_SCALE_Y_INC = "Y <>";
    public static final String BUTTON_SCALE_Y_DEC = "Y ><";
    public static final String BUTTON_SCALE_X_INC = "X <>";
    public static final String BUTTON_SCALE_X_DEC = "X ><";

    public static final String BUTTON_SHIFTV_UP   = "A";
    public static final String BUTTON_SHIFTV_DOWN = "V";

    static String BUTTON_SMPP_PROPERTIES;
    static String BUTTON_HTTP_PROPERTIES;
    static String BUTTON_MMS_PROPERTIES;
    static String BUTTON_SHOW_GRAPH;
    static String BUTTON_HIDE_GRAPH;
    static String BUTTON_PAUSE_GRAPH;
    static String BUTTON_CONTINUE_GRAPH;

    static String BUTTON_SELECT_ALL;
    static String BUTTON_DESELECT_ALL;
    static String BUTTON_APPLY;
//    static String BUTTON_OK;
    static String BUTTON_CLOSE;

    public static final int TIMEOUT_MILLISEC = 10000;
//    public int SMPP_COUNT = 15;
//    public int HTTP_COUNT = 10;

    public void init() {

        //System.out.println("Initing..." );
        localText = new RemoteResourceBundle(getCodeBase(),getParameter("resource_servlet_uri"));
        //System.out.println("SvcMon:getCodeBase()=" + getCodeBase() + ".");
        locale=localText.getLocale();

        maxSpeed = Integer.valueOf(getParameter("max.speed")).intValue();
//        graphScale = Integer.valueOf(getParameter("graph.scale")).intValue();
        graphGrid = Integer.valueOf(getParameter("graph.grid")).intValue();
        graphHiGrid = Integer.valueOf(getParameter("graph.higrid")).intValue();
        graphHead = Integer.valueOf(getParameter("graph.head")).intValue();
        //System.out.println("INIT:\nmaxSpeed=" + maxSpeed + "\ngraph.grid=" + graphGrid + "\ngraph.hiGrid=" + graphHiGrid + "\ngraph.head=" + graphHead +"\n" );

        setFont(new Font("Dialog", Font.BOLD, 14));
        setLayout(new GridBagLayout());
        setBackground(SystemColor.control);
        GridBagConstraints gbc = new GridBagConstraints();
        gbc.fill = GridBagConstraints.BOTH;

        connectingLabel = new Label(localText.getString("connecting"));
        add(connectingLabel, gbc);

        BUTTON_SMPP_PROPERTIES = localText.getString("svcmon.button.smpp.filter");
        BUTTON_HTTP_PROPERTIES = localText.getString("svcmon.button.smpp.filter");
        BUTTON_SHOW_GRAPH = localText.getString("svcmon.button.showgraph");
        BUTTON_HIDE_GRAPH = localText.getString("svcmon.button.hidegraph");
        BUTTON_PAUSE_GRAPH = localText.getString("svcmon.button.pause");
        BUTTON_CONTINUE_GRAPH = localText.getString("svcmon.button.continue");

        BUTTON_SELECT_ALL = localText.getString("svcmon.selectframe.button.selectall");
        BUTTON_DESELECT_ALL= localText.getString("svcmon.selectframe.button.deselectall");
        BUTTON_APPLY = localText.getString("svcmon.selectframe.button.apply");
        BUTTON_CLOSE = localText.getString("svcmon.selectframe.button.close");
        validate();
    }

//INNER CLASSES

    class CheckBoxListener implements ActionListener{
        public CheckBoxListener() {

        }

        public void actionPerformed(ActionEvent e) {
            JCheckBox checkBox = (JCheckBox)e.getSource();
            //System.out.println("CHECKBOOX '" + checkBox.getText() + "' is " + checkBox.isSelected() );

        }
    }

    boolean isStopping = false;

    protected int getSpeed(){
        int result=10;
        for(int i =0; i<svcSnap.smppCount; i++){
            //System.out.println("speed=" + svcSnap.smppSnaps[i].smppSpeed[2]);
            if( result < svcSnap.smppSnaps[i].smppSpeed[2] ){
                result = svcSnap.smppSnaps[i].smppSpeed[2];
                //System.out.println("set result to " + result);
            }
        }
        //System.out.println("getMaxSpeed()=" + result );
        return result;
    }

    SmppPanel smppPanel;
    HttpPanel httpPanel;

    protected void gotFirstSnap(SvcSnap snap) {
            start = false;
            remove(connectingLabel);
            GridBagConstraints gbc = new GridBagConstraints();
            gbc.fill = GridBagConstraints.BOTH;
            snapSmppHistory = new SnapSmppHistory();
            snapHttpHistory = new SnapHttpHistory();
//            maxSpeed = ( maxSpeed<getSpeed() )? getSpeed(): maxSpeed;
            //System.out.println("gotFirstSnap:smppCount=" + snap.smppCount + "\tsmppViewList='" + smppViewList + "'" + "\nmaxSpeed=" + maxSpeed);
            smppTopGraph = new SmppTopGraph(snap, maxSpeed, graphScale, graphGrid,
                    graphHiGrid, graphHead, localText, snapSmppHistory, smppViewList, smppViewGraph );

            //System.out.println("gotFirstSnap:httpCount=" + snap.httpCount + "\thttpViewList='" + httpViewList + "'");
            httpTopGraph = new HttpTopGraph(snap, maxSpeed, graphScale, graphGrid,
                    graphHiGrid, graphHead, localText, snapHttpHistory, httpViewList, httpViewGraph);
        //System.out.println("SvcMon:gotFirstSnap:create TbbedPane");
        JTabbedPane jTabbedPane = new JTabbedPane();
        smppPanel = new SmppPanel(this);
        jTabbedPane.addTab( "SMPP", smppPanel );
        httpPanel = new HttpPanel(this);
        jTabbedPane.addTab("HTTP", httpPanel );
        /* Hide MMS
        jTabbedPane.addTab("MMS", new MmsPanel());*/

        //jTabbedPane.insertTab("SMPP", null, new SmppPanel() , null, 0);

        pause.addObserver( smppPanel.viewButtonPanel );
        pause.addObserver( httpPanel.viewButtonPanel );

        gbc.gridy = 1;
        gbc.gridx = 1;
        gbc.gridwidth = 1;
        gbc.weightx = 1;
        gbc.weighty = 1;
        gbc.fill = GridBagConstraints.BOTH;
        add(jTabbedPane, gbc);
        validate();
    }

    public void run() {
        Socket sock = null;
        DataInputStream is = null;
        isStopping = false;
        //System.out.println("SvcMon.run() host='" + getParameter("host") + "' port='" +  Integer.valueOf(getParameter("port")).intValue() + "'");
        try {
            SvcSnap snap = new SvcSnap(); // moved
            int counter = TIMEOUT_MILLISEC/1000;
            while (!isStopping) {
                try {
                    //System.out.println("SvcMon.run() new Socket()");
                    sock = new Socket(getParameter("host"), Integer.valueOf(getParameter("port")).intValue());
                    //System.out.println("SvcMon.run() getInputStream");
                    InputStream is1 = sock.getInputStream();
                    //System.out.println("SvcMon.run() is1.available() from sock='" + is1.available() + "'");
                    is = new DataInputStream(is1);
//                    SvcSnap snap = new SvcSnap(); moved above
                    snap.read(is);
                    svcSnap = snap;
                    gotFirstSnap(snap);
                    String text = this.smppPanel.viewButtonPanel.pauseGraphButton.getText();
                    while (!isStopping) {
//                        System.out.println("SvcMon:run():while:while");
                        if( pause.getState() ){
                            //System.out.println("SvcMon:run():while:while:if1:pause");

                            this.smppPanel.viewButtonPanel.pauseGraphButton.setText( (counter--) + " " + BUTTON_CONTINUE_GRAPH );
                            Thread.sleep( 1000 );
                            if( (Calendar.getInstance().getTimeInMillis() - smppPauseTime.getTimeInMillis()) > TIMEOUT_MILLISEC ){
                                //System.out.println("SvcMon:run():while:while:if2:pause end");
                                pause.setState( false );
                            }
                        } else {
                            counter = TIMEOUT_MILLISEC/1000;;
//                            System.out.println("SvcMon:run():while:while:if2");
                            snap.read(is);
                            smppTopGraph.setSnap(snap, smppViewList, graphScale, maxSpeed, smppXScale, smppYScale, smppViewGraph, smppShiftV);
                            httpTopGraph.setSnap(snap, httpViewList, graphScale, maxSpeed, httpXScale, httpYScale, httpViewGraph, httpShiftV);
                        }
//                        Thread.sleep( 1000 );
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
        offscreen = null;
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
            if (screen == null) {
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

    public void mouseEntered(MouseEvent e) {

    }

    public void mouseExited(MouseEvent e) {

    }

    public void mousePressed(MouseEvent e) {

    }

    public void mouseReleased(MouseEvent e) {

    }

    public void actionPerformed(ActionEvent e) {

    }

    public void itemStateChanged(ItemEvent e) {

    }

    public void start() {
        System.out.println("SvcMon starting...");
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
class GraphParam{
    private int graphScale = 2;
    private int graphGrid = 5;
    private int graphHiGrid = 25;
    private int graphHead = 50;
}

class MmsPanel extends JPanel {
    public MmsPanel() {
        setLayout(new BorderLayout());
        //add(smppTopGraph, BorderLayout.CENTER);
        setFont(new Font("Dialog", Font.BOLD, 22));

        setLayout(new GridBagLayout());
        setBackground(SystemColor.control);
        GridBagConstraints gbc = new GridBagConstraints();
        gbc.fill = GridBagConstraints.BOTH;

        Label label = new Label("MMS \n under constract");
        add( label );
    }
}

class SmppPanel extends JPanel {

    ViewButtonPanel viewButtonPanel;

    public SmppPanel( SvcMon mon ) {
//        GridBagConstraints gbc = new GridBagConstraints();
        setLayout( new BorderLayout() );
        ScrollPane sp = new ScrollPane( ScrollPane.SCROLLBARS_AS_NEEDED );
        sp.add( mon.smppTopGraph );
        add(sp, BorderLayout.CENTER );

        setFont( new Font("Dialog", Font.BOLD, 12) );

        viewButtonPanel = new ViewButtonPanel(SvcMon.TYPE_SMPP, mon);
        add( viewButtonPanel, BorderLayout.SOUTH );

        mon.smppTopGraph.requestFocus();
    }
}

class HttpPanel extends JPanel {

    ViewButtonPanel viewButtonPanel;

    public HttpPanel( SvcMon mon ) {
        setLayout( new BorderLayout() );
        ScrollPane sp = new ScrollPane( ScrollPane.SCROLLBARS_AS_NEEDED );
        sp.add( mon.httpTopGraph );
//            setLayout( new BorderLayout() );
        add( sp, BorderLayout.CENTER );

        setFont(new Font("Dialog", Font.BOLD, 12));

        viewButtonPanel = new ViewButtonPanel( SvcMon.TYPE_HTTP, mon );
        add( viewButtonPanel, BorderLayout.SOUTH );
//        mon.httpTopGraph.requestFocus();
    }
}

class ViewButtonPanel extends JPanel implements Observer{

    static final int SCALE_STEP_Y = 1;
    static final int SCALE_STEP_X = 1;

    final JButton pauseGraphButton = new JButton( SvcMon.BUTTON_PAUSE_GRAPH );;
    final JButton incYButton = new JButton(SvcMon.BUTTON_SCALE_Y_INC);
    final JButton decYButton = new JButton(SvcMon.BUTTON_SCALE_Y_DEC);
    final JButton incXButton = new JButton(SvcMon.BUTTON_SCALE_X_INC);
    final JButton decXButton = new JButton(SvcMon.BUTTON_SCALE_X_DEC);
    final JButton showGraphButton =  new JButton(SvcMon.BUTTON_SHOW_GRAPH);

    static final int SHIFT_STEP = 100;
    final JButton shiftUpButton   = new JButton(SvcMon.BUTTON_SHIFTV_UP);
    final JButton shiftDownButton = new JButton(SvcMon.BUTTON_SHIFTV_DOWN);

    public ViewButtonPanel(final String type, final SvcMon mon){
        final JButton viewPropertiesButton;
        if( type.equals(SvcMon.TYPE_SMPP) ){
            viewPropertiesButton = new JButton(SvcMon.BUTTON_SMPP_PROPERTIES);
        }else if( type.equals(SvcMon.TYPE_HTTP) ){
            viewPropertiesButton = new JButton(SvcMon.BUTTON_HTTP_PROPERTIES);
        }else {
            viewPropertiesButton = new JButton(SvcMon.BUTTON_MMS_PROPERTIES);
        }

        setButtonsEnabled( false );
        
        setLayout( new GridBagLayout() );
        GridBagConstraints gbc = new GridBagConstraints();
        gbc.gridheight = 1;
        gbc.weightx = 1;
        gbc.fill = GridBagConstraints.BOTH;

//        gbc.gridy = 1;
//        gbc.gridx = 2;
        gbc.gridy = 1;
        gbc.gridx = 0;
        gbc.gridheight = 1;
        add(incYButton, gbc);

//        gbc.gridy = 1;
//        gbc.gridx = 3;
        gbc.gridy = 2;
        gbc.gridx = 0;
        gbc.gridheight = 1;
        add(decYButton, gbc);

        gbc.gridy = 2;
        gbc.gridx = 1;
        gbc.gridheight = 1;
        gbc.gridwidth = 2;
        add(viewPropertiesButton, gbc);

        gbc.gridy = 1;
        gbc.gridx = 1;
        gbc.gridheight = 1;
        gbc.gridwidth = 1;
        add(incXButton, gbc);

        gbc.gridy = 1;
        gbc.gridx = 2;
        gbc.gridheight = 1;
        gbc.gridwidth = 1;
        add(decXButton, gbc);

        gbc.gridy = 1;
        gbc.gridx = 3;
        gbc.gridheight = 2;
        gbc.gridwidth = 1;
        add(showGraphButton, gbc);

        gbc.gridy = 1;
        gbc.gridx = 4;
        gbc.gridheight = 2;
        add(pauseGraphButton, gbc);

        gbc.gridy = 1;
        gbc.gridx = 5;
        gbc.gridheight = 1;
        add(shiftUpButton, gbc);

        gbc.gridy = 2;
        gbc.gridx = 5;
        gbc.gridheight = 1;
        add(shiftDownButton, gbc);


        ActionListener shiftUpListener = new
            ActionListener(){
                public void actionPerformed(ActionEvent event){
                    if( type.equals(SvcMon.TYPE_SMPP) ){
                        mon.smppShiftV = mon.smppShiftV<2000? mon.smppShiftV + SHIFT_STEP: 2000;
                        //System.out.println("SvcMon SMPP shiftUpListener shiftV='" + mon.smppShiftV + "' prev='" + (mon.smppShiftV - SHIFT_STEP) + "'" );
                    }else if( type.equals(SvcMon.TYPE_HTTP) ){
                        mon.httpShiftV = mon.httpShiftV<2000? mon.httpShiftV + SHIFT_STEP: 2000;
                        //System.out.println("SvcMon HTTP zoomYInListener shiftV=' " + mon.httpShiftV + " prev " + (mon.httpShiftV - SHIFT_STEP) );
                    }
//                        smppTopGraph.maxSpeed = 200;
                    mon.smppTopGraph.invalidate();
                }
            };
        shiftUpButton.addActionListener(shiftUpListener);

        ActionListener shiftDownListener = new
            ActionListener(){
                public void actionPerformed(ActionEvent event){
                    if( type.equals(SvcMon.TYPE_SMPP) ){
                        mon.smppShiftV = mon.smppShiftV>=SHIFT_STEP? mon.smppShiftV - SHIFT_STEP: 0;
                        //System.out.println("SMPP shiftDownListener. smppShiftV='" + mon.smppShiftV + "' prev='" + (mon.smppShiftV + SHIFT_STEP) + "'" );
                    }else if( type.equals(SvcMon.TYPE_HTTP) ){
                        mon.httpShiftV = mon.httpShiftV>=SHIFT_STEP? mon.httpShiftV - SHIFT_STEP: 0;
                        //System.out.println("HTTP shiftDownListener. httpShiftV='" + mon.httpShiftV + "' prev='" + (mon.httpShiftV + SHIFT_STEP) + "'" );
                    }
//                        smppTopGraph.maxSpeed = 200;
                    mon.smppTopGraph.invalidate();
                }
            };
        shiftDownButton.addActionListener(shiftDownListener);



        ActionListener pauseGraphListener = new
            ActionListener(){
                    public void actionPerformed(ActionEvent event){
//                        pauseGraph = (pauseGraph == true)? false: true;
                        if( type.equals(SvcMon.TYPE_SMPP) ){
                            if( mon.smppViewGraph ){
                                if( mon.pause.getState() ){
                                    mon.pause.setState( false );
                                }else{
                                    mon.pause.setState( true );
                                    mon.smppPauseTime = Calendar.getInstance();
                                }
                                //System.out.println("smpppPauseGraph has set to '" + mon.smppPauseGraph + "'");
                            }
                        }else if( type.equals(SvcMon.TYPE_HTTP) ){
                            if( mon.httpViewGraph ){
                                if( mon.pause.getState() ){
                                    mon.pause.setState( false );
                                }else{
                                    mon.pause.setState( true );
                                    mon.smppPauseTime = Calendar.getInstance();
                                }
                                //System.out.println("httpPauseGraph has set to '" + mon.httpPauseGraph + "'");
                            }
                        }
                    }
                public void actionPerformed_(ActionEvent event){
                        if( mon.pause.getState() ){
                            mon.pause.setState( false );
                        }else{
                            mon.smppPauseTime = Calendar.getInstance();
                            mon.pause.setState( true );
                        }
                }
            };
        pauseGraphButton.addActionListener( pauseGraphListener );

        ActionListener viewGraphListener = new
            ActionListener(){
                public void actionPerformed(ActionEvent event){
                    if( type.equals(SvcMon.TYPE_SMPP) ){
//                            smppViewGraph = (smppViewGraph == true)? false: true;
                        if( mon.smppViewGraph == true ){
                            mon.smppViewGraph = false;
                            showGraphButton.setText( SvcMon.BUTTON_SHOW_GRAPH );
                            setButtonsEnabled( false );
                        }else{
                            mon.smppViewGraph = true;
                            showGraphButton.setText( SvcMon.BUTTON_HIDE_GRAPH );
                            setButtonsEnabled( true );
                        }
                    }else if( type.equals(SvcMon.TYPE_HTTP) ){
                        if( mon.httpViewGraph == true ){
                            mon.httpViewGraph = false;
                            showGraphButton.setText( SvcMon.BUTTON_SHOW_GRAPH );
                            setButtonsEnabled( false );
                        }else{
                            mon.httpViewGraph = true;
                            showGraphButton.setText( SvcMon.BUTTON_HIDE_GRAPH );
                            setButtonsEnabled( true );
                        }
                    }
                }
            };
        showGraphButton.addActionListener( viewGraphListener );

//            add(hardButton);

        ActionListener viewPropertiesListener = new
            ActionListener(){
                public void actionPerformed(ActionEvent event){
                    ViewPropertyFrame blank = new ViewPropertyFrame(type, mon);
                    blank.setVisible(true);
                }
            };
        viewPropertiesButton.addActionListener( viewPropertiesListener );

        ActionListener zoomYInListener = new
            ActionListener(){
                public void actionPerformed(ActionEvent event){
                    if( type.equals(SvcMon.TYPE_SMPP) ){
                        mon.smppYScale = mon.smppYScale<1? mon.smppYScale*2: mon.smppYScale + SCALE_STEP_Y;
                        //System.out.println("SMPP zoomYInListener. new is " + mon.smppYScale + " Before " + (mon.smppYScale - SCALE_STEP_Y) );
                    }else if( type.equals(SvcMon.TYPE_HTTP) ){
                        mon.httpYScale = mon.httpYScale<1? mon.httpYScale*2: mon.httpYScale + SCALE_STEP_Y;
                        //System.out.println("HTTP zoomYInListener new is " + mon.httpYScale + " Before " + (mon.httpYScale - SCALE_STEP_Y) );
                    }
//                        smppTopGraph.maxSpeed = 200;
                    mon.smppTopGraph.invalidate();
                }
            };
        incYButton.addActionListener(zoomYInListener);

        ActionListener zoomYOutListener = new
            ActionListener(){
                public void actionPerformed(ActionEvent event){
                    if( type.equals(SvcMon.TYPE_SMPP) ){
                        //System.out.println("SMPP zoomYOutListener");
                        mon.smppYScale = mon.smppYScale>1?mon.smppYScale - SCALE_STEP_Y: mon.smppYScale>0.125? mon.smppYScale/2: mon.smppYScale;
                    }else if( type.equals(SvcMon.TYPE_HTTP) ){
                        //System.out.println("HTTP zoomYOutListener");
                        mon.httpYScale = mon.httpYScale>1?mon.httpYScale - SCALE_STEP_Y: mon.httpYScale>0.125? mon.httpYScale/2: mon.httpYScale;
                    }
                    mon.smppTopGraph.invalidate();
                }
            };
        decYButton.addActionListener(zoomYOutListener);

        ActionListener zoomXInListener = new
            ActionListener(){
                public void actionPerformed(ActionEvent event){
                    if( type.equals(SvcMon.TYPE_SMPP) ){
                        mon.smppXScale += SCALE_STEP_X;
                        //System.out.println("SMPP zoomXInListener new is " + mon.smppYScale + " Before " + (mon.smppXScale-SCALE_STEP_X) );
                    }else if( type.equals(SvcMon.TYPE_HTTP) ){
                        mon.httpXScale += SCALE_STEP_X;
                        //System.out.println("HTTP zoomXInListener new is " + mon.httpXScale + " Before " + (mon.httpXScale-SCALE_STEP_X) );
                    }
                    mon.smppTopGraph.invalidate();
                }
            };
        incXButton.addActionListener(zoomXInListener);

        ActionListener zoomXOutListener = new
            ActionListener(){
                public void actionPerformed(ActionEvent event){
                    if( type.equals(SvcMon.TYPE_SMPP) ){
//                            System.out.println("SMPP zoomXOutListener");
                        mon.smppXScale = mon.smppXScale>1?mon.smppXScale - SCALE_STEP_X: mon.smppXScale;;
                    }else if( type.equals(SvcMon.TYPE_HTTP) ){
//                            System.out.println("HTTP zoomXOutListener");
                        mon.httpXScale = mon.httpXScale>1?mon.httpXScale - SCALE_STEP_X: mon.httpXScale;;
                    }
                    mon.smppTopGraph.invalidate();
                }
            };
        decXButton.addActionListener(zoomXOutListener);
    }

    void setButtonsEnabled( boolean value ){

        pauseGraphButton.setEnabled( value );
        incYButton.setEnabled( value );
        decYButton.setEnabled( value );
        incXButton.setEnabled( value );
        decXButton.setEnabled( value );
        shiftUpButton.setEnabled( value );
        shiftDownButton.setEnabled( value );

    }

    public void update( Observable o, Object arg) {

        Pause pause = (Pause)o;
        //System.out.println("ViewButtonPanel:update():'" + pause.getState() + "'");
        if( pause.getState() ){
            pauseGraphButton.setText( SvcMon.BUTTON_CONTINUE_GRAPH );
        }else{
            pauseGraphButton.setText( SvcMon.BUTTON_PAUSE_GRAPH );
        }

    }

}

class ViewPropertyFrame extends JFrame{
    public ViewPropertyFrame(String type, SvcMon mon){

        setTitle( type + " endpoints filter" );
//        int strHeight;
//            JCheckBox cb = new JCheckBox("A");
//            height = cb.getHeight();
//            System.out.println("CB height=" + height );
        final int strHeight = 30;
        int totalStrHeight = ( type.equals(SvcMon.TYPE_SMPP)? mon.svcSnap.smppCount: mon.svcSnap.httpCount ) * strHeight;
        //System.out.println("strsHeight=" + totalStrHeight);
        final int buttonsHeight = 100;
        int frameHeight = totalStrHeight + buttonsHeight;
        int frameWidth = 300;
        setSize( frameWidth, frameHeight );
        setLocation( 300, 200 );
        if( type.equals(SvcMon.TYPE_SMPP) ){
            PropertyPanel smppPanel = new PropertyPanel(this, mon, SvcMon.TYPE_SMPP);
            getContentPane().add(smppPanel);
        }else if( type.equals(SvcMon.TYPE_HTTP) ){
            PropertyPanel htppPanel = new PropertyPanel(this, mon, SvcMon.TYPE_HTTP);
            getContentPane().add(htppPanel);
        }
    }

}

class PropertyPanel extends JPanel{
    public PropertyPanel(JFrame frame, SvcMon mon, String type){
        setLayout( new BorderLayout() );

        ScrollPane checkPane = new ScrollPane(ScrollPane.SCROLLBARS_AS_NEEDED);
        CheckboxPanel checkPanel = new CheckboxPanel(type, mon);
        checkPane.add(checkPanel);
        SelectPanel selectPanel = new SelectPanel(checkPanel);
        OkCancelPanel okPanel = new OkCancelPanel( frame, checkPanel, type, mon );

        add(selectPanel, BorderLayout.NORTH);
        add(okPanel, BorderLayout.SOUTH);
        add(checkPane, BorderLayout.CENTER);
    }

}

class SmppPropertyPanel extends JPanel{
    public SmppPropertyPanel(JFrame frame, SvcMon mon){
        setLayout( new BorderLayout() );

        ScrollPane checkPane = new ScrollPane(ScrollPane.SCROLLBARS_AS_NEEDED);
        CheckboxPanel checkPanel = new CheckboxPanel(SvcMon.TYPE_SMPP, mon);
        checkPane.add(checkPanel);
        SelectPanel selectPanel = new SelectPanel(checkPanel);
        OkCancelPanel okPanel = new OkCancelPanel( frame, checkPanel, SvcMon.TYPE_SMPP, mon );

        add(selectPanel, BorderLayout.NORTH);
        add(okPanel, BorderLayout.SOUTH);
        add(checkPane, BorderLayout.CENTER);
    }

}

class HttpPropertyPanel extends JPanel{
    public HttpPropertyPanel(JFrame frame, SvcMon mon){
        setLayout( new BorderLayout() );

        ScrollPane checkPane = new ScrollPane(ScrollPane.SCROLLBARS_AS_NEEDED);
        CheckboxPanel checkPanel = new CheckboxPanel(SvcMon.TYPE_HTTP, mon);
        checkPane.add(checkPanel);
        SelectPanel selectPanel = new SelectPanel(checkPanel);
        OkCancelPanel okPanel = new OkCancelPanel( frame, checkPanel, SvcMon.TYPE_SMPP, mon );

        add(selectPanel, BorderLayout.NORTH);
        add(okPanel, BorderLayout.SOUTH);
        add(checkPane, BorderLayout.CENTER);
    }

}

class SelectPanel extends JPanel{

//    public static final String BUTTON_SELECT = SvcMon.BUTTON_SELECT_ALL;
//    public static final String BUTTON_DESELECT = SvcMon.BUTTON_DESELECT_ALL;

    public SelectPanel(CheckboxPanel checkPanel){
        setLayout( new GridLayout(1,2));
        JButton selectAllButton = new JButton(SvcMon.BUTTON_SELECT_ALL);
        JButton deSelectAllButton = new JButton(SvcMon.BUTTON_DESELECT_ALL);
        add(selectAllButton);
        add(deSelectAllButton);

//        SelectDeSelectAllListeber selDeselAllListener = new SelectDeSelectAllListeber(checkPanel);
        SelectAllListener selectAllListener = new SelectAllListener(checkPanel);
        DeSelectAllListener deselectAllListener = new DeSelectAllListener(checkPanel);
        selectAllButton.addActionListener(selectAllListener);
        deSelectAllButton.addActionListener(deselectAllListener);
    }
}

class SelectAllListener implements ActionListener{
    CheckboxPanel panel;

    public SelectAllListener( CheckboxPanel panel ) {
        this.panel = panel;
    }

    public void actionPerformed(ActionEvent e) {
        if(e.getSource() instanceof JButton ){
            JButton button = (JButton)e.getSource();
            //System.out.println( "SvcMon:SelectAllListener:BUTTON: " + button.getText() );
                for(int i = 0; i<panel.getComponentCount(); i++){
                    Component comp = panel.getComponent(i);
                    if( comp instanceof JCheckBox ){
                        JCheckBox box = (JCheckBox)comp;
                        box.setSelected( true );
                        //System.out.println( "SvcMon:SelectAllListener " +i + " CHECKBOX: " + box.getText() );
                    }
                }
        }
    }
}

class DeSelectAllListener implements ActionListener{
    CheckboxPanel panel;

    public DeSelectAllListener( CheckboxPanel panel ) {
        this.panel = panel;
    }

    public void actionPerformed(ActionEvent e) {
        if(e.getSource() instanceof JButton ){
            JButton button = (JButton)e.getSource();
            //System.out.println( "BUTTON: " + button.getText() );
                for(int i = 0; i<panel.getComponentCount(); i++){
                    Component comp = panel.getComponent(i);
                    if( comp instanceof JCheckBox ){
                        JCheckBox box = (JCheckBox)comp;
                        box.setSelected( false );
                        //System.out.println(i + " CHECKBOX: " + box.getText() );
                    }
                }
        }
    }
}

class CheckboxPanel extends JPanel{
//        public CheckboxPanel( SvcSnap snaps ){
    public CheckboxPanel( String type, SvcMon mon ){
        if( type.equals(SvcMon.TYPE_SMPP)){
            int smppCount = mon.svcSnap.smppCount;
            setLayout( new GridLayout(smppCount,1));
            for (int i = 0; i < smppCount; i++) {
                String name = mon.svcSnap.smppSnaps[i].smppId;
                JCheckBox checkBox = new JCheckBox( name );
                if( mon.smppViewList == null || mon.smppViewList.contains(mon.svcSnap.smppSnaps[i].smppId) ){
                    checkBox.setSelected(true);
                }
                add(checkBox);
//                checkBox.addActionListener( new CheckBoxListener() );
            }
        }else if( type.equals(SvcMon.TYPE_HTTP)){
            int httpCount = mon.svcSnap.httpCount;
            setLayout( new GridLayout(httpCount,1));
            for (int i = 0; i < httpCount; i++) {
                String name = mon.svcSnap.httpSnaps[i].httpId;
                JCheckBox checkBox = new JCheckBox( name );
                if( mon.httpViewList == null || mon.httpViewList.contains(mon.svcSnap.httpSnaps[i].httpId) ){
                    checkBox.setSelected(true);
                }
                add(checkBox);
//                checkBox.addActionListener( new CheckBoxListener() );
            }

        }
    }

}

class OkCancelPanel extends JPanel{
    public OkCancelPanel( final JFrame frame, CheckboxPanel checkPanel, String type, SvcMon mon ){
        setLayout( new GridLayout(1,3));

        JButton okButton = new JButton(SvcMon.BUTTON_APPLY);
        JButton cancelButton = new JButton(SvcMon.BUTTON_CLOSE);

        add(okButton);
        add(cancelButton);
    //            add(hardButton);
    //            HardListener hardListener = new HardListener();
    //            hardButton.addActionListener(hardListener);

        ActionListener exit = new
            ActionListener(){
                public void actionPerformed( ActionEvent event ){
                    frame.dispose();
                }
            };

        cancelButton.addActionListener(exit);
        okButton.addActionListener( new OkListener(checkPanel, type, mon ) );
    }
}

class OkListener implements ActionListener{
    CheckboxPanel panel;
    String type;
    SvcMon mon;

    public OkListener( CheckboxPanel panel, String type, SvcMon mon ) {
        this.panel = panel;
        this.type = type;
        this.mon = mon;
    }

    public void actionPerformed(ActionEvent e) {
        if(e.getSource() instanceof JButton ){
            JButton button = (JButton)e.getSource();
            //System.out.println( "BUTTON: " + button.getText() );
            if( button.getText().equals(SvcMon.BUTTON_APPLY) ){
                HashSet temp = new HashSet();
                for( int i = 0; i<panel.getComponentCount(); i++ ){
                    Component comp = panel.getComponent(i);
                    if( comp instanceof JCheckBox ){
                        JCheckBox checkBox = (JCheckBox)comp;
                        String checkName = checkBox.getText();
                        //System.out.println( "JCheckBox " + checkName );
                        if( checkBox.isSelected()){
                            temp.add( checkName );
                        }
                    }
                }
                if(type.equals(SvcMon.TYPE_SMPP)){
                    mon.smppViewList = temp;
                    //System.out.println("After OK smppViewList='" + mon.smppViewList + "'");
                }else if(type.equals(SvcMon.TYPE_HTTP)){
                    mon.httpViewList = temp;
                    //System.out.println("After OK httpViewList='" + mon.httpViewList + "'");
                }
            }
        }

    }
}