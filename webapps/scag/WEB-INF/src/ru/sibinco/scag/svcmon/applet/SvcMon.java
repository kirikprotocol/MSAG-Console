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
    public SmppTopGraph smppTopGraph;
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

    Calendar smppPauseTime;
    Calendar httpPauseTime;

    String TEST_BUTTON = "TEST";
    boolean start = true;

    public static final String BUTTON_SCALE_Y_INC = "Y <>";
    public static final String BUTTON_SCALE_Y_DEC = "Y ><";
    public static final String BUTTON_SCALE_X_INC = "X <>";
    public static final String BUTTON_SCALE_X_DEC = "X ><";
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

        System.out.println("Initing..." );
        localText = new RemoteResourceBundle(getCodeBase(),getParameter("resource_servlet_uri"));
        System.out.println("SvcMon:getCodeBase()=" + getCodeBase() + ".");
        locale=localText.getLocale();
        maxSpeed = Integer.valueOf(getParameter("max.speed")).intValue();
//        graphScale = Integer.valueOf(getParameter("graph.scale")).intValue();
        graphGrid = Integer.valueOf(getParameter("graph.grid")).intValue();
        graphHiGrid = Integer.valueOf(getParameter("graph.higrid")).intValue();
        graphHead = Integer.valueOf(getParameter("graph.head")).intValue();

        System.out.println("INIT:\nmaxSpeed=" + maxSpeed + "\ngraph.grid=" + graphGrid + "\ngraph.hiGrid=" + graphHiGrid + "\ngraph.head=" + graphHead );
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
            System.out.println("CHECKBOOX '" + checkBox.getText() + "' is " + checkBox.isSelected() );

        }
    }

    boolean isStopping = false;

    protected int getSpeed(){
        int result=10;
        for(int i =0; i<svcSnap.smppCount; i++){
            System.out.println("speed=" + svcSnap.smppSnaps[i].smppSpeed[2]);
            if( result < svcSnap.smppSnaps[i].smppSpeed[2] ){
                result = svcSnap.smppSnaps[i].smppSpeed[2];
                System.out.println("set result to " + result);
            }
        }
        System.out.println("getMaxSpeed()=" + result );
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
            System.out.println("gotFirstSnap:smppCount=" + snap.smppCount + "\tsmppViewList='" + smppViewList + "'" + "\nmaxSpeed=" + maxSpeed);
            smppTopGraph = new SmppTopGraph(snap, maxSpeed, graphScale, graphGrid,
                    graphHiGrid, graphHead, localText, snapSmppHistory, smppViewList, smppViewGraph );

            System.out.println("gotFirstSnap:httpCount=" + snap.httpCount + "\thttpViewList='" + httpViewList + "'");
            httpTopGraph = new HttpTopGraph(snap, maxSpeed, graphScale, graphGrid,
                    graphHiGrid, graphHead, localText, snapHttpHistory, httpViewList, httpViewGraph);
        System.out.println("SvcMon:gotFirstSnap:create TbbedPane");
        JTabbedPane jTabbedPane = new JTabbedPane();
        smppPanel = new SmppPanel(this);
        jTabbedPane.addTab( "SMPP", smppPanel );
        httpPanel = new HttpPanel(this);
        jTabbedPane.addTab("HTTP", httpPanel );
        jTabbedPane.addTab("MMS", new MmsPanel());
        //jTabbedPane.insertTab("SMPP", null, new SmppPanel() , null, 0);

        pause.addObserver( smppPanel.pan );
        pause.addObserver( httpPanel.pan );

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
        System.out.println("SvcMon:run():host='" + getParameter("host") + "' port='" +  Integer.valueOf(getParameter("port")).intValue() + "'");
        try {
            while (!isStopping) {
                try {
                    System.out.println("SvcMon:run():sock");
                    sock = new Socket(getParameter("host"), Integer.valueOf(getParameter("port")).intValue());
                    System.out.println("SvcMon:run():getInputStream");
                    InputStream is1 = sock.getInputStream();
                    System.out.println("SvcMon:run():is1.available() from sock='" + is1.available() + "'");
                    is = new DataInputStream(is1);
                    SvcSnap snap = new SvcSnap();
                    snap.read(is);
                    svcSnap = snap;
                    gotFirstSnap(snap);

                    while (!isStopping) {
//                        System.out.println("SvcMon:run():while:while");
                        if( pause.getState() ){
                            System.out.println("SvcMon:run():while:while:if1:pause");
                            Thread.sleep( 500 );
                            if( (Calendar.getInstance().getTimeInMillis() - smppPauseTime.getTimeInMillis()) > TIMEOUT_MILLISEC ){
                                System.out.println("SvcMon:run():while:while:if2:pause end");
                                pause.setState( false );
                            }
                        }
                        else {
//                            System.out.println("SvcMon:run():while:while:if2");
                            snap.read(is);
                            smppTopGraph.setSnap(snap, smppViewList, graphScale, maxSpeed, smppXScale, smppYScale, smppViewGraph);
                            httpTopGraph.setSnap(snap, httpViewList, graphScale, maxSpeed, httpXScale, httpYScale, httpViewGraph);
                        }
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
                    System.out.println("I/O error: " + ex.getMessage() + ". Reconnecting...");
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
        System.out.println("Connection thread stopped");
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

    ViewButtonPanel pan;

    public SmppPanel( SvcMon mon ) {
//        GridBagConstraints gbc = new GridBagConstraints();
        setLayout( new BorderLayout() );
        ScrollPane sp = new ScrollPane( ScrollPane.SCROLLBARS_AS_NEEDED );
        sp.add( mon.smppTopGraph );
        add(sp, BorderLayout.CENTER );

        setFont( new Font("Dialog", Font.BOLD, 12) );

        pan = new ViewButtonPanel(SvcMon.TYPE_SMPP, mon);
        add( pan, BorderLayout.SOUTH );

        mon.smppTopGraph.requestFocus();
    }
}

class HttpPanel extends JPanel {

    ViewButtonPanel pan;

    public HttpPanel( SvcMon mon ) {
        setLayout( new BorderLayout() );
        ScrollPane sp = new ScrollPane( ScrollPane.SCROLLBARS_AS_NEEDED );
        sp.add( mon.httpTopGraph );
//            setLayout( new BorderLayout() );
        add( sp, BorderLayout.CENTER );

        setFont(new Font("Dialog", Font.BOLD, 12));

        pan = new ViewButtonPanel( SvcMon.TYPE_HTTP, mon );
        add( pan, BorderLayout.SOUTH );
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


    public ViewButtonPanel(final String type, final SvcMon mon){
        final JButton viewPropertiesButton;
        if( type.equals(SvcMon.TYPE_SMPP) ){
            viewPropertiesButton = new JButton(SvcMon.BUTTON_SMPP_PROPERTIES);
        }else if( type.equals(SvcMon.TYPE_HTTP) ){
            viewPropertiesButton = new JButton(SvcMon.BUTTON_HTTP_PROPERTIES);
        }else {
            viewPropertiesButton = new JButton(SvcMon.BUTTON_MMS_PROPERTIES);
        }
//        final JButton incYButton = new JButton(SvcMon.BUTTON_SCALE_Y_INC);
//        final JButton decYButton = new JButton(SvcMon.BUTTON_SCALE_Y_DEC);
//        final JButton incXButton = new JButton(SvcMon.BUTTON_SCALE_X_INC);
//        final JButton decXButton = new JButton(SvcMon.BUTTON_SCALE_X_DEC);
//        final JButton showGraphButton =  new JButton(SvcMon.BUTTON_SHOW_GRAPH);
//        pauseGraphButton = new JButton( SvcMon.BUTTON_PAUSE_GRAPH );

        setButtonsEnabled( false );
        
        setLayout( new GridBagLayout() );
        GridBagConstraints gbc = new GridBagConstraints();
        gbc.gridheight = 1;
        gbc.weightx = 1;
        gbc.fill = GridBagConstraints.BOTH;

        gbc.gridy = 1;
        gbc.gridx = 1;
        gbc.gridheight = 2;
        add(viewPropertiesButton, gbc);

        gbc.gridy = 1;
        gbc.gridx = 2;
        gbc.gridheight = 1;
        add(incYButton, gbc);

        gbc.gridy = 1;
        gbc.gridx = 3;
        gbc.gridheight = 1;
        add(decYButton, gbc);

        gbc.gridy = 1;
        gbc.gridx = 4;
        gbc.gridheight = 2;
        add(showGraphButton, gbc);

        gbc.gridy = 1;
        gbc.gridx = 5;
        gbc.gridheight = 2;
        add(pauseGraphButton, gbc);

        gbc.gridy = 2;
        gbc.gridx = 2;
        gbc.gridheight = 1;
        add(incXButton, gbc);

        gbc.gridy = 2;
        gbc.gridx = 3;
        gbc.gridheight = 1;
        add(decXButton, gbc);

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
                                System.out.println("smpppPauseGraph has set to '" + mon.smppPauseGraph + "'");
                            }
                        }else if( type.equals(SvcMon.TYPE_HTTP) ){
                            if( mon.httpViewGraph ){
                                if( mon.pause.getState() ){
                                    mon.pause.setState( false );
                                }else{
                                    mon.pause.setState( true );
                                    mon.smppPauseTime = Calendar.getInstance();
                                }
                                System.out.println("httpPauseGraph has set to '" + mon.httpPauseGraph + "'");
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
                        System.out.println("SMPP zoomYInListener. new is " + mon.smppYScale + " Before " + (mon.smppYScale - SCALE_STEP_Y) );
                    }else if( type.equals(SvcMon.TYPE_HTTP) ){
                        mon.httpYScale = mon.httpYScale<1? mon.httpYScale*2: mon.httpYScale + SCALE_STEP_Y;
                        System.out.println("HTTP zoomYInListener new is " + mon.httpYScale + " Before " + (mon.httpYScale - SCALE_STEP_Y) );
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
                        System.out.println("SMPP zoomYOutListener");
                        mon.smppYScale = mon.smppYScale>1?mon.smppYScale - SCALE_STEP_Y: mon.smppYScale>0.125? mon.smppYScale/2: mon.smppYScale;
                    }else if( type.equals(SvcMon.TYPE_HTTP) ){
                        System.out.println("HTTP zoomYOutListener");
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
                        System.out.println("SMPP zoomXInListener new is " + mon.smppYScale + " Before " + (mon.smppXScale-SCALE_STEP_X) );
                    }else if( type.equals(SvcMon.TYPE_HTTP) ){
                        mon.httpXScale += SCALE_STEP_X;
                        System.out.println("HTTP zoomXInListener new is " + mon.httpXScale + " Before " + (mon.httpXScale-SCALE_STEP_X) );
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

    }

    public void update( Observable o, Object arg) {

        Pause pause = (Pause)o;
        System.out.println("ViewButtonPanel:update():'" + pause.getState() + "'");
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
        System.out.println("strsHeight=" + totalStrHeight);
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
            System.out.println( "SvcMon:SelectAllListener:BUTTON: " + button.getText() );
                for(int i = 0; i<panel.getComponentCount(); i++){
                    Component comp = panel.getComponent(i);
                    if( comp instanceof JCheckBox ){
                        JCheckBox box = (JCheckBox)comp;
                        box.setSelected( true );
                        System.out.println( "SvcMon:SelectAllListener " +i + " CHECKBOX: " + box.getText() );
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
            System.out.println( "BUTTON: " + button.getText() );
                for(int i = 0; i<panel.getComponentCount(); i++){
                    Component comp = panel.getComponent(i);
                    if( comp instanceof JCheckBox ){
                        JCheckBox box = (JCheckBox)comp;
                        box.setSelected( false );
                        System.out.println(i + " CHECKBOX: " + box.getText() );
                    }
                }
        }
    }
}

//class SelectDeSelectAllListeber implements ActionListener{
//    CheckboxPanel panel;
//
//    public SelectDeSelectAllListeber( CheckboxPanel panel ) {
//        this.panel = panel;
//    }
//
//    public void actionPerformed(ActionEvent e) {
//        if(e.getSource() instanceof JButton ){
//            JButton button = (JButton)e.getSource();
//            System.out.println( "BUTTON: " + button.getText() );
//                for(int i = 0; i<panel.getComponentCount(); i++){
//                    Component comp = panel.getComponent(i);
//                    if( comp instanceof JCheckBox ){
//                        JCheckBox box = (JCheckBox)comp;
//                        if( button.getText().equals(BUTTON_SELECT) ){
//                            box.setSelected( true );
//                        } else if( button.getText().equals(BUTTON_DESELECT) ){
//                            box.setSelected( false );
//                        }
//                        System.out.println(i + " CHECKBOX: " + box.getText() );
//                    }
//                }
//        }
//    }
//}

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
            System.out.println( "BUTTON: " + button.getText() );
            if( button.getText().equals(SvcMon.BUTTON_APPLY) ){
                HashSet temp = new HashSet();
                for( int i = 0; i<panel.getComponentCount(); i++ ){
                    Component comp = panel.getComponent(i);
                    if( comp instanceof JCheckBox ){
                        JCheckBox checkBox = (JCheckBox)comp;
                        String checkName = checkBox.getText();
                        System.out.println( "JCheckBox " + checkName );
                        if( checkBox.isSelected()){
                            temp.add( checkName );
                        }
                    }
                }
                if(type.equals(SvcMon.TYPE_SMPP)){
                    mon.smppViewList = temp;
                    System.out.println("After OK smppViewList='" + mon.smppViewList + "'");
                }else if(type.equals(SvcMon.TYPE_HTTP)){
                    mon.httpViewList = temp;
                    System.out.println("After OK httpViewList='" + mon.httpViewList + "'");
                }
            }
        }

    }
}




// INNER CLASSE

//    class SmppPanel extends JPanel {
//
//        public SmppPanel() {
//            setLayout( new BorderLayout() );
////        GridBagConstraints gbc = new GridBagConstraints();
//            setLayout( new BorderLayout() );
//            ScrollPane sp = new ScrollPane( ScrollPane.SCROLLBARS_AS_NEEDED );
//            sp.add( smppTopGraph );
//            add(sp, BorderLayout.CENTER );
//
//            setFont( new Font("Dialog", Font.BOLD, 12) );
//            ViewButtonPanel pan_pre = new ViewButtonPanel(TYPE_SMPP);
//            ViewButtonPanel pan = new ViewButtonPanel(TYPE_SMPP);
//            add( pan, BorderLayout.SOUTH );
//
//            smppTopGraph.requestFocus();
//        }
//
//        public SmppPanel( SvcMon mon ) {
//            setLayout( new BorderLayout() );
////        GridBagConstraints gbc = new GridBagConstraints();
//            setLayout( new BorderLayout() );
//            ScrollPane sp = new ScrollPane( ScrollPane.SCROLLBARS_AS_NEEDED );
//            sp.add( mon.smppTopGraph );
//            add(sp, BorderLayout.CENTER );
//
//            setFont( new Font("Dialog", Font.BOLD, 12) );
//            ViewButtonPanel pan_pre = new ViewButtonPanel(TYPE_SMPP);
//            ViewButtonPanel pan = new ViewButtonPanel(TYPE_SMPP);
//            add( pan, BorderLayout.SOUTH );
//
//            mon.smppTopGraph.requestFocus();
//        }
//    }

//    class HttpPanel extends JPanel {
//        public HttpPanel() {
//            setLayout( new BorderLayout() );
//            ScrollPane sp = new ScrollPane( ScrollPane.SCROLLBARS_AS_NEEDED );
//            sp.add( httpTopGraph );
////            setLayout( new BorderLayout() );
//            add( sp, BorderLayout.CENTER );
//            setFont(new Font("Dialog", Font.BOLD, 12));
//
//            ViewButtonPanel pan = new ViewButtonPanel(TYPE_HTTP);
//            add( pan, BorderLayout.SOUTH );
//            httpTopGraph.requestFocus();
//        }
//    }

//    class ViewButtonPanel extends JPanel{
//
//        public final String BUTTON_SMPP_PROPERTIES = localText.getString("svcmon.smpp.filter");
//        public final String BUTTON_HTTP_PROPERTIES = localText.getString("svcmon.smpp.filter");
//        public static final String BUTTON_SCALE_Y_INC = "Y scale <>";
//        public static final String BUTTON_SCALE_Y_DEC = "Y scale ><";
//        public static final String BUTTON_SCALE_X_INC = "X scale <>";
//        public static final String BUTTON_SCALE_X_DEC = "X scale ><";
//        public final String BUTTON_SHOW_GRAPH = localText.getString("svcmon.showgraph");
//        public final String BUTTON_HIDE_GRAPH = localText.getString("svcmon.hidegraph");
//        public final String BUTTON_PAUSE_GRAPH = localText.getString("svcmon.pause");
//        public final String BUTTON_CONTINUE_GRAPH = localText.getString("svcmon.continue");
//        public static final int SCALE_STEP_Y = 1;
//        public static final int SCALE_STEP_X = 1;
//
//        public ViewButtonPanel(final String type){
//            final JButton viewPropertiesButton;
//            if( type.equals(TYPE_SMPP) ){
//                viewPropertiesButton = new JButton(BUTTON_SMPP_PROPERTIES);
//            }else if( type.equals(TYPE_HTTP) ){
//                viewPropertiesButton = new JButton(BUTTON_HTTP_PROPERTIES);
//            }else {
//                viewPropertiesButton = new JButton(BUTTON_HTTP_PROPERTIES);
//            }
//            final JButton incYButton = new JButton(BUTTON_SCALE_Y_INC);
//            final JButton decYButton = new JButton(BUTTON_SCALE_Y_DEC);
//            final JButton incXButton = new JButton(BUTTON_SCALE_X_INC);
//            final JButton decXButton = new JButton(BUTTON_SCALE_X_DEC);
//            final JButton showGraphButton =  new JButton(BUTTON_SHOW_GRAPH);
//            final JButton pauseGraphButton = new JButton( (smppPauseGraph? BUTTON_CONTINUE_GRAPH: BUTTON_PAUSE_GRAPH) );
//
////            HardListener hardListener = new HardListener();
////            hardButton.addActionListener(hardListener);
//
//            setLayout( new GridBagLayout() );
//            GridBagConstraints gbc = new GridBagConstraints();
//            gbc.gridheight = 1;
//            gbc.weightx = 1;
//            gbc.fill = GridBagConstraints.BOTH;
//
//            gbc.gridy = 1;
//            gbc.gridx = 1;
//            gbc.gridheight = 2;
//            add(viewPropertiesButton, gbc);
//            gbc.gridheight = 1;
//            gbc.gridy = 1;
//            gbc.gridx = 2;
//            add(incYButton, gbc);
//            gbc.gridy = 1;
//            gbc.gridx = 3;
//            add(decYButton, gbc);
//
//            gbc.gridy = 2;
//            gbc.gridx = 2;
//            add(incXButton, gbc);
//            gbc.gridy = 2;
//            gbc.gridx = 3;
//            add(decXButton, gbc);
//
//            gbc.gridy = 1;
//            gbc.gridx = 4;
//            gbc.gridheight = 2;
//            add(showGraphButton, gbc);
//
//
//            gbc.gridheight = 2;
//            gbc.gridy = 1;
//            gbc.gridx = 5;
//            add(pauseGraphButton, gbc);
//
//            gbc.gridheight = 2;
//            gbc.gridy = 1;
//            gbc.gridx = 6;
//            add(testButton, gbc);
//
//
//           ActionListener pauseGraphListener = new
//                ActionListener(){
//                    public void actionPerformed(ActionEvent event){
////                        viewPause = (viewPause == true)? false: true;
//                        if( type.equals(TYPE_SMPP) ){
//                            if( smppPauseGraph == true ){
//                                smppPauseGraph = false;
//                                pauseGraphButton.setText( BUTTON_PAUSE_GRAPH );
//                            }else{
//                                smppPauseGraph = true;
//                                smppPauseTime = Calendar.getInstance();
//                                pauseGraphButton.setText( BUTTON_CONTINUE_GRAPH );
////                                validate();
////                                try {
////                                    Thread.sleep( 5000 );
////                                } catch (InterruptedException e) {
////                                    e.printStackTrace();  //To change body of catch statement use File | Settings | File Templates.
////                                }
//                            }
//                            if( httpPauseGraph == true ){
//                                httpPauseGraph = false;
//                                pauseGraphButton.setText( BUTTON_PAUSE_GRAPH );
//                            }else{
//                                httpPauseGraph = true;
//                                httpPauseTime = Calendar.getInstance();
//                                pauseGraphButton.setText( BUTTON_CONTINUE_GRAPH );
//
//                            }
//                            System.out.println("smpppPauseGraph has set to '" + smppPauseGraph + "'");
//                        }else if( type.equals(TYPE_HTTP) ){
//                            if( smppPauseGraph == true ){
//                                smppPauseGraph = false;
//                                pauseGraphButton.setText( BUTTON_PAUSE_GRAPH );
//                            }else{
//                                smppPauseGraph = true;
//                                smppPauseTime = Calendar.getInstance();
//                                pauseGraphButton.setText( BUTTON_CONTINUE_GRAPH );
//                            }
//                            if( httpPauseGraph == true ){
//                                httpPauseGraph = false;
//                                pauseGraphButton.setText( BUTTON_PAUSE_GRAPH );
//                            }else{
//                                httpPauseGraph = true;
//                                httpPauseTime = Calendar.getInstance();
//                                pauseGraphButton.setText( BUTTON_CONTINUE_GRAPH );
//                            }
//                            System.out.println("httpPauseGraph has set to '" + httpPauseGraph + "'");
//                        }
//                    }
//                };
//            pauseGraphButton.addActionListener( pauseGraphListener );
//
//            ActionListener viewGraphListener = new
//                ActionListener(){
//                    public void actionPerformed(ActionEvent event){
//                        if( type.equals(TYPE_SMPP) ){
////                            smppViewGraph = (smppViewGraph == true)? false: true;
//                            if( smppViewGraph == true ){
//                                smppViewGraph = false;
//                                showGraphButton.setText( BUTTON_SHOW_GRAPH );
//                            }else{
//                                smppViewGraph = true;
//                                showGraphButton.setText( BUTTON_HIDE_GRAPH );
//                            }
//                        }else if( type.equals(TYPE_HTTP) ){
//                            if( httpViewGraph == true ){
//                                httpViewGraph = false;
//                                showGraphButton.setText( BUTTON_SHOW_GRAPH );
//                            }else{
//                                httpViewGraph = true;
//                                showGraphButton.setText( BUTTON_HIDE_GRAPH );
//                            }
//                        }
//                    }
//                };
//            showGraphButton.addActionListener( viewGraphListener );
//
////            add(hardButton);
//
//            ActionListener viewPropertiesListener = new
//                ActionListener(){
//                    public void actionPerformed(ActionEvent event){
//                        ViewPropertyFrame blank = new ViewPropertyFrame(type);
//                        blank.setVisible(true);
//                    }
//                };
//            viewPropertiesButton.addActionListener( viewPropertiesListener );
//
//            ActionListener zoomYInListener = new
//                ActionListener(){
//                    public void actionPerformed(ActionEvent event){
//                        if( type.equals(TYPE_SMPP) ){
//                            smppYScale = smppYScale<1? smppYScale*2: smppYScale + SCALE_STEP_Y;
//                            System.out.println("SMPP zoomYInListener. new is " + smppYScale + " Before " + (smppYScale - SCALE_STEP_Y) );
//                        }else if( type.equals(TYPE_HTTP) ){
//                            httpYScale = httpYScale<1? httpYScale*2: httpYScale + SCALE_STEP_Y;
//                            System.out.println("HTTP zoomYInListener new is " + httpYScale + " Before " + (httpYScale - SCALE_STEP_Y) );
//                        }
////                        smppTopGraph.maxSpeed = 200;
//                        smppTopGraph.invalidate();
//                    }
//                };
//            incYButton.addActionListener(zoomYInListener);
//
//            ActionListener zoomYOutListener = new
//                ActionListener(){
//                    public void actionPerformed(ActionEvent event){
//                        if( type.equals(TYPE_SMPP) ){
//                            System.out.println("SMPP zoomYOutListener");
//                            smppYScale = smppYScale>1?smppYScale - SCALE_STEP_Y: smppYScale>0.125? smppYScale/2: smppYScale;
//                        }else if( type.equals(TYPE_HTTP) ){
//                            System.out.println("HTTP zoomYOutListener");
//                            httpYScale = httpYScale>1?httpYScale - SCALE_STEP_Y: httpYScale>0.125? httpYScale/2: httpYScale;
//                        }
//                        smppTopGraph.invalidate();
//                    }
//                };
//            decYButton.addActionListener(zoomYOutListener);
//
//            ActionListener zoomXInListener = new
//                ActionListener(){
//                    public void actionPerformed(ActionEvent event){
//                        if( type.equals(TYPE_SMPP) ){
//                            smppXScale += SCALE_STEP_X;
//                            System.out.println("SMPP zoomXInListener new is " + smppYScale + " Before " + (smppXScale-SCALE_STEP_X) );
//                        }else if( type.equals(TYPE_HTTP) ){
//                            httpXScale += SCALE_STEP_X;
//                            System.out.println("HTTP zoomXInListener new is " + httpXScale + " Before " + (httpXScale-SCALE_STEP_X) );
//                        }
//                        smppTopGraph.invalidate();
//                    }
//                };
//            incXButton.addActionListener(zoomXInListener);
//
//            ActionListener zoomXOutListener = new
//                ActionListener(){
//                    public void actionPerformed(ActionEvent event){
//                        if( type.equals(TYPE_SMPP) ){
////                            System.out.println("SMPP zoomXOutListener");
//                            smppXScale = smppXScale>1?smppXScale - SCALE_STEP_X: smppXScale;;
//                        }else if( type.equals(TYPE_HTTP) ){
////                            System.out.println("HTTP zoomXOutListener");
//                            httpXScale = httpXScale>1?httpXScale - SCALE_STEP_X: httpXScale;;
//                        }
//                        smppTopGraph.invalidate();
//                    }
//                };
//            decXButton.addActionListener(zoomXOutListener);
//        }
//        public void delay( int millisec ){
//            try {
//                Thread.sleep( millisec );
//            } catch (InterruptedException e) {
//                e.printStackTrace();  //To change body of catch statement use File | Settings | File Templates.
//            }
//        }
//    }

//    class ViewPropertyFrame extends JFrame{
//        public ViewPropertyFrame(String type){
//
//            setTitle( type + " endpoints filter" );
//            int strHeight;
////            JCheckBox cb = new JCheckBox("A");
////            height = cb.getHeight();
////            System.out.println("CB height=" + height );
//            strHeight = 30;
//            int height = (type.equals(TYPE_SMPP)?svcSnap.smppCount:svcSnap.httpCount)*strHeight;
//            System.out.println("strsHeight=" + height);
//            int frameHeight = height+100;
//            setSize( 230, frameHeight );
//            setLocation( 300, 200 );
//            if( type.equals(TYPE_SMPP) ){
//                SmppPropertyPanel smppPanel = new SmppPropertyPanel(this);
//                getContentPane().add(smppPanel);
//            }else if( type.equals(TYPE_HTTP) ){
//                HttpPropertyPanel htppPanel = new HttpPropertyPanel(this);
//                getContentPane().add(htppPanel);
//            }
//        }
//
//    }


//    class SmppPropertyPanel extends JPanel{
//        public SmppPropertyPanel(JFrame frame){
//            setLayout( new BorderLayout() );
//
//            ScrollPane checkPane = new ScrollPane(ScrollPane.SCROLLBARS_AS_NEEDED);
//            CheckboxPanel checkPanel = new CheckboxPanel(TYPE_SMPP);
//            checkPane.add(checkPanel);
//            SelectPanel selectPanel = new SelectPanel(checkPanel);
//            OkCancelPanel okPanel = new OkCancelPanel( frame, checkPanel, TYPE_SMPP );
//
//            add(selectPanel, BorderLayout.NORTH);
//            add(okPanel, BorderLayout.SOUTH);
//            add(checkPane, BorderLayout.CENTER);
//        }
//
//    }

//    class HttpPropertyPanel extends JPanel{
//        public HttpPropertyPanel(JFrame frame){
//            setLayout( new BorderLayout() );
//
//            ScrollPane checkPane = new ScrollPane(ScrollPane.SCROLLBARS_AS_NEEDED);
//            CheckboxPanel checkPanel = new CheckboxPanel(TYPE_HTTP);
//            checkPane.add(checkPanel);
//            SelectPanel selectPanel = new SelectPanel(checkPanel);
//            OkCancelPanel okPanel = new OkCancelPanel( frame, checkPanel, TYPE_HTTP );
//
//            add(selectPanel, BorderLayout.NORTH);
//            add(okPanel, BorderLayout.SOUTH);
//            add(checkPane, BorderLayout.CENTER);
//        }
//
//    }


//    class OkCancelPanel extends JPanel{
//
//        public static final String BUTTON_CLOSE = "Close";
//
//
//        public OkCancelPanel( final JFrame frame, CheckboxPanel checkPanel, String type ){
//            setLayout( new GridLayout(1,3));
//
//            JButton okButton = new JButton(BUTTON_OK);
//            JButton cancelButton = new JButton(BUTTON_CLOSE);
//
//            add(okButton);
//            add(cancelButton);
////            add(hardButton);
//
////            HardListener hardListener = new HardListener();
////            hardButton.addActionListener(hardListener);
//
//            ActionListener exit = new
//                ActionListener(){
//                    public void actionPerformed( ActionEvent event ){
//                        frame.dispose();
//                    }
//                };
//
//            cancelButton.addActionListener(exit);
//            okButton.addActionListener( new OkListener(checkPanel, type) );
//        }
//    }

//    class OkListener implements ActionListener{
//        CheckboxPanel panel;
//        String type;
//
//        public OkListener( CheckboxPanel panel, String type ) {
//            this.panel = panel;
//            this.type = type;
//        }
//
//        public void actionPerformed(ActionEvent e) {
//            if(e.getSource() instanceof JButton ){
////                SmppSnap[] smppSnaps = svcSnap.smppSnaps;
//                JButton button = (JButton)e.getSource();
//                System.out.println( "BUTTON: " + button.getText() );
//                if( button.getText().equals(BUTTON_OK) ){
//                    HashSet temp = new HashSet();
//                    for( int i = 0; i<panel.getComponentCount(); i++ ){
//                        Component comp = panel.getComponent(i);
//                        if( comp instanceof JCheckBox ){
//                            JCheckBox checkBox = (JCheckBox)comp;
//                            String checkName = checkBox.getText();
//                            System.out.println( "JCheckBox " + checkName );
//                            if( checkBox.isSelected()){
//                                temp.add( checkName );
//                            }
//                        }
//                    }
//                    if(type.equals(TYPE_SMPP)){
//                        smppViewList = temp;
//                        System.out.println("After OK smppViewList='" + smppViewList + "'");
//                    }else if(type.equals(TYPE_HTTP)){
//                        httpViewList = temp;
//                        System.out.println("After OK httpViewList='" + httpViewList + "'");
//                    }
//                }
//            }
//
//        }
//    }

//    class HardListener implements ActionListener{
//        public void actionPerformed(ActionEvent e) {
//            hard = hard?false:true;
//            System.out.println("HARD has been set to '" + hard + "'");
//        }
//    }

//    public static final String BUTTON_SELECT = "Select all";
//    public static final String BUTTON_DESELECT = "Deselect all";

//    class SelectPanel extends JPanel{
//
//        public SelectPanel(CheckboxPanel checkPanel){
//            setLayout( new GridLayout(1,2));
//            JButton selectAllButton = new JButton(BUTTON_SELECT);
//            JButton deSelectAllButton = new JButton(BUTTON_DESELECT);
//            add(selectAllButton);
//            add(deSelectAllButton);
//
//            SelectDeSelectAllListeber selDeselAllListener = new SelectDeSelectAllListeber(checkPanel);
//            selectAllButton.addActionListener(selDeselAllListener);
//            deSelectAllButton.addActionListener(selDeselAllListener);
//        }
//    }
//
//    class CheckboxPanel extends JPanel{
////        public CheckboxPanel( SvcSnap snaps ){
//        public CheckboxPanel( String type ){
//            if( type.equals(TYPE_SMPP)){
//                int smppCount = svcSnap.smppCount;
//                setLayout( new GridLayout(smppCount,1));
//                for (int i = 0; i < smppCount; i++) {
//                    String name = svcSnap.smppSnaps[i].smppId;
//                    JCheckBox checkBox = new JCheckBox( name );
//                    if( smppViewList == null || smppViewList.contains(svcSnap.smppSnaps[i].smppId) ){
//                        checkBox.setSelected(true);
//                    }
//                    add(checkBox);
//    //                checkBox.addActionListener( new CheckBoxListener() );
//                }
//            }else if( type.equals(TYPE_HTTP)){
//                int httpCount = svcSnap.httpCount;
//                setLayout( new GridLayout(httpCount,1));
//                for (int i = 0; i < httpCount; i++) {
//                    String name = svcSnap.httpSnaps[i].httpId;
//                    JCheckBox checkBox = new JCheckBox( name );
//                    if( httpViewList == null || httpViewList.contains(svcSnap.httpSnaps[i].httpId) ){
//                        checkBox.setSelected(true);
//                    }
//                    add(checkBox);
//    //                checkBox.addActionListener( new CheckBoxListener() );
//                }
//
//            }
//        }
//
//    }

//    public SvcSnap initHardSnaps(){
//        SvcSnap snaps = new SvcSnap();
//        snaps.smppCount = SMPP_COUNT;
//        snaps.httpCount = HTTP_COUNT;
//
//        snaps.smppSnaps = new SmppSnap [snaps.smppCount];
//        SmppSnap smppSnap;
//        for( int i= 0; i<snaps.smppCount; i++ ){
//            smppSnap = new SmppSnap();
//            smppSnap.smppId = "smppId"+i;
//            for(int ii=0; ii<SmppSnap.COUNTERS; ii++){
//                smppSnap.smppAvgSpeed[ii]=(short)ii;
//                smppSnap.smppSpeed[ii]=(short)(ii*2);
//            }
//            snaps.smppSnaps[i]= smppSnap;
//        }
//
//        snaps.httpSnaps = new HttpSnap [snaps.httpCount];
//        HttpSnap httpSnap;
//        for( int i=0; i<snaps.httpCount; i++ ){
//            httpSnap = new HttpSnap();
//            httpSnap.httpId = "httpId"+i;
//            for(int ii=0; ii<HttpSnap.COUNTERS; ii++){
//                httpSnap.httpAvgSpeed[ii]=(short)ii;
//                httpSnap.httpSpeed[ii]=(short)(ii*2);
//            }
//            snaps.httpSnaps[i]= httpSnap;
//        }
//
//        return snaps;
//    }

//    class SelectDeSelectAllListeber implements ActionListener{
//        CheckboxPanel panel;
//
//        public SelectDeSelectAllListeber( CheckboxPanel panel ) {
//            this.panel = panel;
//        }
//
//        public void actionPerformed(ActionEvent e) {
//            if(e.getSource() instanceof JButton ){
//                JButton button = (JButton)e.getSource();
//                System.out.println( "BUTTON: " + button.getText() );
//                    for(int i = 0; i<panel.getComponentCount(); i++){
//                        Component comp = panel.getComponent(i);
//                        if( comp instanceof JCheckBox ){
//                            JCheckBox box = (JCheckBox)comp;
//                            if( button.getText().equals(BUTTON_SELECT) ){
//                                box.setSelected( true );
//                            } else if( button.getText().equals(BUTTON_DESELECT) ){
//                                box.setSelected( false );
//                            }
//                            System.out.println(i + " CHECKBOX: " + box.getText() );
//                        }
//                    }
//            }
//        }
//    }
