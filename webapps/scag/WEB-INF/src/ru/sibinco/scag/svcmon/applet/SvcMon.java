/*
 * Copyright (c) 2005 SibInco Inc. All Rights Reserved.
 */
package ru.sibinco.scag.svcmon.applet;

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

/**
 * The <code>SvcMon</code> class represents
 * <p><p/>
 * Date: 08.12.2005
 * Time: 17:14:20
 *
 * @author &lt;a href="mailto:igor@sibinco.ru"&gt;Igor Klimenko&lt;/a&gt;
 */
public class SvcMon extends Applet implements Runnable, MouseListener, ActionListener, ItemListener {

    public static RemoteResourceBundle localText;
    public static Locale locale;
    private Label connectingLabel;
    private SnapSmppHistory snapSmppHistory;
    private SnapHttpHistory snapHttpHistory;
    private SmppTopGraph smppTopGraph;
    private HttpTopGraph httpTopGraph;
    private ScreenSplitter screenSplitter;

    private int maxSpeed = 100;
    private int graphScale = 200;
    private int graphGrid = 5;
    private int graphHiGrid = 25;
    private int graphHead = 50;

    SvcSnap svcSnap;

    public static final String BUTTON_SELECT = "Select all";
    public static final String BUTTON_DESELECT = "Deselect all";
    public static final String BUTTON_OK = "Ok";
    public static final String BUTTON_CLOSE = "Close";
    public static final String BUTTON_SMPP_PROPERTIES = "Endpoints filter";
    public static final String BUTTON_HTTP_PROPERTIES = "Endpoints filter";
    public static final String BUTTON_SCALE_Y_IN = "Y scale +";
    public static final String BUTTON_SCALE_Y_OUT = "Y scale -";
    public static final String BUTTON_SCALE_X_IN = "X scale +";
    public static final String BUTTON_SCALE_X_OUT = "X scale -";

    public static final String TYPE_SMPP = "SMPP";
    public static final String TYPE_HTTP = "HTTP";

    public static final int SCALE_STEP = 1;

    public HashSet smppViewList;
    public HashSet httpViewList;

    public float xScale = 5;
    public float yScale = 1;

    public static final int SCALE_STEP_Y = 1;
    public static final int SCALE_STEP_X = 1;

    public int SMPP_COUNT = 15;
    public int HTTP_COUNT = 10;

    private boolean hard = false;

    public void init() {

        System.out.println("Initing..." );
        localText = new RemoteResourceBundle(getCodeBase(),getParameter("resource_servlet_uri"));
        locale=localText.getLocale();
        maxSpeed = Integer.valueOf(getParameter("max.speed")).intValue();
        graphScale = Integer.valueOf(getParameter("graph.scale")).intValue();
        graphGrid = Integer.valueOf(getParameter("graph.grid")).intValue();
        graphHiGrid = Integer.valueOf(getParameter("graph.higrid")).intValue();
        graphHead = Integer.valueOf(getParameter("graph.head")).intValue();

        System.out.println("INIT:maxSpeed=" + maxSpeed);
        setFont(new Font("Dialog", Font.BOLD, 14));
        setLayout(new GridBagLayout());
        setBackground(SystemColor.control);
        GridBagConstraints gbc = new GridBagConstraints();
        gbc.fill = GridBagConstraints.BOTH;

        connectingLabel = new Label(localText.getString("connecting"));
        add(connectingLabel, gbc);
        validate();
    }

    class SmppPanel extends JPanel {

        public SmppPanel() {
            setLayout(new BorderLayout());
//        GridBagConstraints gbc = new GridBagConstraints();
//        gbc.gridy = 1;
//        gbc.gridx = 1;
//        gbc.gridwidth = 1;
//        gbc.gridheight = 4;
//        gbc.weightx = 1;
//        gbc.weighty = 1;
            System.out.println("add smppTopGraph");
            add(smppTopGraph, BorderLayout.CENTER );
            setFont( new Font("Dialog", Font.BOLD, 12) );

            ViewButtonPanel pan = new ViewButtonPanel(TYPE_SMPP);
            add(pan, BorderLayout.SOUTH);

            smppTopGraph.requestFocus();
        }
    }

    class ViewButtonPanel extends JPanel{
        public ViewButtonPanel(final String type){
            final JButton viewPropertiesButton;
            if( type.equals(TYPE_SMPP) ){
                viewPropertiesButton = new JButton(BUTTON_SMPP_PROPERTIES);
            }else{
                viewPropertiesButton = new JButton(BUTTON_HTTP_PROPERTIES);
            }
            final JButton zoomYInPropButton = new JButton(BUTTON_SCALE_Y_IN);
            final JButton zoomYOutPropButton = new JButton(BUTTON_SCALE_Y_OUT);
            final JButton zoomXInPropButton = new JButton(BUTTON_SCALE_X_IN);
            final JButton zoomXOutPropButton = new JButton(BUTTON_SCALE_X_OUT);
//            JButton hardButton = new JButton("HARD");

//            HardListener hardListener = new HardListener();
//            hardButton.addActionListener(hardListener);

//            setLayout( new GridLayout(1,3) );
            setLayout( new GridBagLayout() );
            GridBagConstraints gbc = new GridBagConstraints();
            gbc.gridy = 1;
            gbc.gridx = 1;
            gbc.gridheight = 2;
            gbc.weightx =1;
            gbc.fill = GridBagConstraints.BOTH;
            add(viewPropertiesButton, gbc);

            gbc.gridheight = 1;

            gbc.gridy = 1;
            gbc.gridx = 2;
            add(zoomYInPropButton, gbc);
            gbc.gridy = 1;
            gbc.gridx = 3;
            add(zoomYOutPropButton, gbc);

            gbc.gridy = 2;
            gbc.gridx = 2;
            add(zoomXInPropButton, gbc);
            gbc.gridy = 2;
            gbc.gridx = 3;
            add(zoomXOutPropButton, gbc);
//            add(hardButton);

            ActionListener viewPropertiesListener = new
                ActionListener(){
                    public void actionPerformed(ActionEvent event){
                        ViewPropertyFrame blank = new ViewPropertyFrame(type);
                        blank.setVisible(true);
                    }
                };
            viewPropertiesButton.addActionListener( viewPropertiesListener );

            ActionListener zoomYInListener = new
                ActionListener(){
                    public void actionPerformed(ActionEvent event){
                        if( type.equals(TYPE_SMPP) ){
                            yScale += SCALE_STEP_Y;
                            System.out.println("SMPP zoomYInListener " + graphScale);
                        }else if( type.equals(TYPE_HTTP) ){
//                            graphScale+=SCALE_STEP;
//                            maxSpeed = 100;
                            System.out.println("HTTP zoomYInListener " + graphScale);
                        }
//                        smppTopGraph.maxSpeed = 200;
                        smppTopGraph.invalidate();
                    }
                };
            zoomYInPropButton.addActionListener(zoomYInListener);

            ActionListener zoomYOutListener = new
                ActionListener(){
                    public void actionPerformed(ActionEvent event){
                        if( type.equals(TYPE_SMPP) ){
                            System.out.println("SMPP zoomYOutListener");
                            yScale = yScale>1?yScale-SCALE_STEP_Y:yScale;
                        }else if( type.equals(TYPE_HTTP) ){
                            System.out.println("HTTP zoomYOutListener");
//                            if ( graphScale>1) {
//                                graphScale-=SCALE_STEP;
//                                maxSpeed = 200;
//                            }
                        }
//                        smppTopGraph.maxSpeed = 200;
                        smppTopGraph.invalidate();
                    }
                };
            zoomYOutPropButton.addActionListener(zoomYOutListener);

            ActionListener zoomXInListener = new
                ActionListener(){
                    public void actionPerformed(ActionEvent event){
                        if( type.equals(TYPE_SMPP) ){
                            xScale += SCALE_STEP_X;
                            System.out.println("SMPP zoomXInListener " + graphScale);
                        }else if( type.equals(TYPE_HTTP) ){
                            System.out.println("HTTP zoomXInListener " + graphScale);
                        }
                        smppTopGraph.invalidate();
                    }
                };
            zoomXInPropButton.addActionListener(zoomXInListener);

            ActionListener zoomXOutListener = new
                ActionListener(){
                    public void actionPerformed(ActionEvent event){
                        if( type.equals(TYPE_SMPP) ){
                            xScale = xScale>1?xScale-SCALE_STEP_X:xScale;;
                        }else if( type.equals(TYPE_HTTP) ){
                            System.out.println("HTTP zoomXOutListener");
//                            if ( graphScale>1) {
//                                graphScale-=SCALE_STEP;
//                                maxSpeed = 200;
//                            }
                        }
                        smppTopGraph.invalidate();
                    }
                };
            zoomXOutPropButton.addActionListener(zoomXOutListener);
        }
    }

    class ViewPropertyFrame extends JFrame{
        public ViewPropertyFrame(String type){

            setTitle( type + " endpoints filter" );
            int strHeight;
//            JCheckBox cb = new JCheckBox("A");
//            height = cb.getHeight();
//            System.out.println("CB height=" + height );
            strHeight = 30;
            int height = (type.equals(TYPE_SMPP)?svcSnap.smppCount:svcSnap.httpCount)*strHeight;
            System.out.println("strsHeight=" + height);
            int frameHeight = height+100;
            setSize( 230, frameHeight );
            setLocation( 300, 200 );
            if( type.equals(TYPE_SMPP) ){
                SmppPropertyPanel smppPanel = new SmppPropertyPanel(this);
                getContentPane().add(smppPanel);
            }else if( type.equals(TYPE_HTTP) ){
                HttpPropertyPanel htppPanel = new HttpPropertyPanel(this);
                getContentPane().add(htppPanel);
            }
        }

    }


    class SmppPropertyPanel extends JPanel{
        public SmppPropertyPanel(JFrame frame){
            setLayout( new BorderLayout() );

            ScrollPane checkPane = new ScrollPane(ScrollPane.SCROLLBARS_AS_NEEDED);
            CheckboxPanel checkPanel = new CheckboxPanel(TYPE_SMPP);
            checkPane.add(checkPanel);
            SelectPanel selectPanel = new SelectPanel(checkPanel);
            OkCancelPanel okPanel = new OkCancelPanel( frame, checkPanel, TYPE_SMPP );

            add(selectPanel, BorderLayout.NORTH);
            add(okPanel, BorderLayout.SOUTH);
            add(checkPane, BorderLayout.CENTER);
        }

    }

    class HttpPropertyPanel extends JPanel{
        public HttpPropertyPanel(JFrame frame){
            setLayout( new BorderLayout() );

            ScrollPane checkPane = new ScrollPane(ScrollPane.SCROLLBARS_AS_NEEDED);
            CheckboxPanel checkPanel = new CheckboxPanel(TYPE_HTTP);
            checkPane.add(checkPanel);
            SelectPanel selectPanel = new SelectPanel(checkPanel);
            OkCancelPanel okPanel = new OkCancelPanel( frame, checkPanel, TYPE_HTTP );

            add(selectPanel, BorderLayout.NORTH);
            add(okPanel, BorderLayout.SOUTH);
            add(checkPane, BorderLayout.CENTER);
        }

    }

    class OkCancelPanel extends JPanel{
        public OkCancelPanel( final JFrame frame, CheckboxPanel checkPanel, String type ){
            setLayout( new GridLayout(1,3));

            JButton okButton = new JButton(BUTTON_OK);
            JButton cancelButton = new JButton(BUTTON_CLOSE);

            add(okButton);
            add(cancelButton);
//            add(hardButton);

            HardListener hardListener = new HardListener();
//            hardButton.addActionListener(hardListener);

            ActionListener exit = new
                ActionListener(){
                    public void actionPerformed( ActionEvent event ){
                        frame.dispose();
                    }
                };

            cancelButton.addActionListener(exit);
            okButton.addActionListener( new OkListener(checkPanel, type) );
        }
    }

    class HardListener implements ActionListener{
        public void actionPerformed(ActionEvent e) {
            hard = hard?false:true;
            System.out.println("HARD has been set to '" + hard + "'");
        }
    }

    class SelectPanel extends JPanel{
        public SelectPanel(CheckboxPanel checkPanel){
            setLayout( new GridLayout(1,2));
            JButton selectAllButton = new JButton(BUTTON_SELECT);
            JButton deSelectAllButton = new JButton(BUTTON_DESELECT);
            add(selectAllButton);
            add(deSelectAllButton);

            SelectDeSelectAllListeber selDeselAllListener = new SelectDeSelectAllListeber(checkPanel);
            selectAllButton.addActionListener(selDeselAllListener);
            deSelectAllButton.addActionListener(selDeselAllListener);
        }
    }

    class CheckboxPanel extends JPanel{
//        public CheckboxPanel( SvcSnap snaps ){
        public CheckboxPanel( String type ){
            if( type.equals(TYPE_SMPP)){
                int smppCount = svcSnap.smppCount;
                setLayout( new GridLayout(smppCount,1));
                for (int i = 0; i < smppCount; i++) {
                    String name = svcSnap.smppSnaps[i].smppId;
                    JCheckBox checkBox = new JCheckBox( name );
                    if( smppViewList == null || smppViewList.contains(svcSnap.smppSnaps[i].smppId) ){
                        checkBox.setSelected(true);
                    }
                    add(checkBox);
    //                checkBox.addActionListener( new CheckBoxListener() );
                }
            }else if( type.equals(TYPE_HTTP)){
                int httpCount = svcSnap.httpCount;
                setLayout( new GridLayout(httpCount,1));
                for (int i = 0; i < httpCount; i++) {
                    String name = svcSnap.httpSnaps[i].httpId;
                    JCheckBox checkBox = new JCheckBox( name );
                    if( httpViewList == null || httpViewList.contains(svcSnap.httpSnaps[i].httpId) ){
                        checkBox.setSelected(true);
                    }
                    add(checkBox);
    //                checkBox.addActionListener( new CheckBoxListener() );
                }

            }
        }

    }

    public SvcSnap initHardSnaps(){
        SvcSnap snaps = new SvcSnap();
        snaps.smppCount = SMPP_COUNT;
        snaps.httpCount = HTTP_COUNT;

        snaps.smppSnaps = new SmppSnap [snaps.smppCount];
        SmppSnap smppSnap;
        for( int i= 0; i<snaps.smppCount; i++ ){
            smppSnap = new SmppSnap();
            smppSnap.smppId = "smppId"+i;
            for(int ii=0; ii<SmppSnap.COUNTERS; ii++){
                smppSnap.smppAvgSpeed[ii]=(short)ii;
                smppSnap.smppSpeed[ii]=(short)(ii*2);
            }
            snaps.smppSnaps[i]= smppSnap;
        }

        snaps.httpSnaps = new HttpSnap [snaps.httpCount];
        HttpSnap httpSnap;
        for( int i=0; i<snaps.httpCount; i++ ){
            httpSnap = new HttpSnap();
            httpSnap.httpId = "httpId"+i;
            for(int ii=0; ii<HttpSnap.COUNTERS; ii++){
                httpSnap.httpAvgSpeed[ii]=(short)ii;
                httpSnap.httpSpeed[ii]=(short)(ii*2);
            }
            snaps.httpSnaps[i]= httpSnap;
        }

        return snaps;
    }

    class SelectDeSelectAllListeber implements ActionListener{
        CheckboxPanel panel;

        public SelectDeSelectAllListeber( CheckboxPanel panel ) {
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
                            if( button.getText().equals(BUTTON_SELECT) ){
                                box.setSelected( true );
                            } else if( button.getText().equals(BUTTON_DESELECT) ){
                                box.setSelected( false );
                            }
                            System.out.println(i + " CHECKBOX: " + box.getText() );
                        }
                    }
            }
        }
    }


    class CheckBoxListener implements ActionListener{
        public CheckBoxListener() {

        }

        public void actionPerformed(ActionEvent e) {
            JCheckBox checkBox = (JCheckBox)e.getSource();
            System.out.println("CHECKBOOX '" + checkBox.getText() + "' is " + checkBox.isSelected() );

        }
    }

    class OkListener implements ActionListener{
        CheckboxPanel panel;
        String type;
//        SvcSnap svcSnap;
//        HttpSnap[] httpSnaps;

//        public OkListener( CheckboxPanel panel, SvcSnap svcSnap ) {
//            this.panel = panel;
//            this.svcSnap = svcSnap;
//        }

        public OkListener( CheckboxPanel panel, String type ) {
            this.panel = panel;
            this.type = type;
        }

        public void actionPerformed(ActionEvent e) {
            if(e.getSource() instanceof JButton ){
//                SmppSnap[] smppSnaps = svcSnap.smppSnaps;
                JButton button = (JButton)e.getSource();
                System.out.println( "BUTTON: " + button.getText() );
                if( button.getText().equals(BUTTON_OK) ){
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
                    if(type.equals(TYPE_SMPP)){
                        smppViewList = temp;
                        System.out.println("After OK smppViewList='" + smppViewList + "'");
                    }else if(type.equals(TYPE_HTTP)){
                        httpViewList = temp;
                        System.out.println("After OK httpViewList='" + httpViewList + "'");
                    }
                }
            }

        }
    } //end OkListener

    class HttpPanel extends JPanel {
        public HttpPanel() {
//            setLayout(new BorderLayout());
//            add(httpTopGraph, BorderLayout.CENTER);
//            setFont(new Font("Dialog", Font.BOLD, 12));
//
//            final JButton smppPropButton = new JButton( BUTTON_HTTP_PROPERTIES );
//            add( smppPropButton, BorderLayout.SOUTH );
//            ActionListener newListener = new
//                ActionListener(){
//                    public void actionPerformed( ActionEvent event ){
//                        ViewPropertyFrame blank = new ViewPropertyFrame(TYPE_HTTP);
//                        blank.show();
//                    }
//                };
//            smppPropButton.addActionListener( newListener );
//            httpTopGraph.requestFocus();
            System.out.println("HttpPanel create");
            setLayout(new BorderLayout());
            add(httpTopGraph, BorderLayout.CENTER );
            setFont(new Font("Dialog", Font.BOLD, 12));

            ViewButtonPanel pan = new ViewButtonPanel(TYPE_HTTP);
            add( pan, BorderLayout.SOUTH );

            httpTopGraph.requestFocus();
        }
    }

    class MmsPanel extends JPanel {
        public MmsPanel() {
            setLayout(new BorderLayout());
            //add(smppTopGraph, BorderLayout.CENTER);
            setFont(new Font("Dialog", Font.BOLD, 12));
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

    protected void gotFirstSnap(SvcSnap snap) {
//        svcSnap = snap;
//        snap = initHardSnaps();
        remove(connectingLabel);
        GridBagConstraints gbc = new GridBagConstraints();
        gbc.fill = GridBagConstraints.BOTH;

        snapSmppHistory = new SnapSmppHistory();
        snapHttpHistory = new SnapHttpHistory();
        maxSpeed = ( maxSpeed<getSpeed() )? getSpeed(): maxSpeed;
        System.out.println("gotFirstSnap:smppCount=" + snap.smppCount + "\tsmppViewList='" + smppViewList + "'" +
                "\nmaxSpeed=" + maxSpeed);
        smppTopGraph = new SmppTopGraph(snap, maxSpeed, graphScale, graphGrid,
                graphHiGrid, graphHead, localText, snapSmppHistory, smppViewList );

        System.out.println("gotFirstSnap:httpCount=" + snap.httpCount + "\thttpViewList='" + httpViewList + "'");
        httpTopGraph = new HttpTopGraph(snap, maxSpeed, graphScale, graphGrid,
                graphHiGrid, graphHead, localText, snapHttpHistory, httpViewList);

        JTabbedPane jTabbedPane = new JTabbedPane();
        jTabbedPane.addTab("SMPP", new SmppPanel());
        jTabbedPane.addTab("HTTP", new HttpPanel());
        jTabbedPane.addTab("MMS", new MmsPanel());
        //jTabbedPane.insertTab("SMPP", null, new SmppPanel() , null, 0);
        gbc.gridy = 1;
        gbc.gridx = 1;
        gbc.gridwidth = 1;
        gbc.weightx = 1;
        gbc.weighty = 1;
        gbc.fill = GridBagConstraints.BOTH;
        add(jTabbedPane, gbc);
//        gbc.gridy = 2;
//        gbc.gridx = 2;
//        gbc.gridwidth = 1;
//        gbc.fill = GridBagConstraints.VERTICAL;
//        JButton propButton = new JButton( "Properties");
//        add( propButton, gbc );
        validate();
    }

    public void run() {
        Socket sock = null;
        DataInputStream is = null;
        isStopping = false;
        try {
            while (!isStopping) {
                try {
                    sock = new Socket(getParameter("host"), Integer.valueOf(getParameter("port")).intValue());
                    is = new DataInputStream(sock.getInputStream());
                    SvcSnap snap = new SvcSnap();
                    snap.read(is);
                    if(hard){
                        snap = initHardSnaps();
                    }
                    svcSnap = snap;
                    gotFirstSnap(snap);
                    while (!isStopping) {
                        snap.read(is);
                        if(hard){
                            snap = initHardSnaps();
                        }
                        svcSnap = snap;
//                        System.out.println("run():graphScale=" + graphScale);
                        smppTopGraph.setSnap(snap, smppViewList, graphScale, maxSpeed, xScale, yScale);
                        httpTopGraph.setSnap(snap, httpViewList, graphScale );
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
