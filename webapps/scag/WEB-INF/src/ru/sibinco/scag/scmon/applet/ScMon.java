/*
 * Copyright (c) 2005 SibInco Inc. All Rights Reserved.
 */

package ru.sibinco.scag.scmon.applet;

import ru.sibinco.scag.scmon.ScSnap;
import ru.sibinco.scag.util.RemoteResourceBundle;

import javax.swing.*;
import java.applet.Applet;
import java.awt.*;
import java.awt.event.*;
import java.io.DataInputStream;
import java.io.IOException;
import java.net.Socket;
import java.util.Locale;
import java.util.HashSet;

public class ScMon extends Applet implements Runnable, MouseListener, ActionListener, ItemListener {

    public static RemoteResourceBundle localText;
    public static Locale locale;
    private Label connectingLabel;
    private SnapSmppHistory snapSmppHistory;
    private SnapHttpHistory snapHttpHistory;
    private SmppTopGraph smppTopGraph;
    private HttpTopGraph httpTopGraph;
    private ScreenSplitter screenSplitter;

    private int maxSpeed = 200;
    private int graphScale = 1;
    private int graphGrid = 5;
    private int graphHiGrid = 25;
    private int graphHead = 50;


    ScSnap centerSnap;

    public static final String TYPE_SMPP = "SMPP";
    public static final String TYPE_HTTP = "HTTP";

    public static final int SCALE_STEP = 1;

    public float xScale = 5;
    public float yScale = 1;

    public HashSet smppViewList;
    public HashSet httpViewList;

    public float smppXScale = 5;
    public double smppYScale = 1;
    public float httpXScale = 5;
    public double httpYScale = 1;

    private boolean smppViewGraph = false;
    private boolean httpViewGraph = false;
    private boolean smppPauseGraph = false;
    private boolean httpPauseGraph = false;

//    ScSnap svcSnap;

    public void init() {

        System.out.println("Initing...");
        localText = new RemoteResourceBundle(getCodeBase(),getParameter("resource_servlet_uri"));
        locale=localText.getLocale();
        maxSpeed = Integer.valueOf(getParameter("max.speed")).intValue();
//        graphScale = Integer.valueOf(getParameter("graph.scale")).intValue();
        graphGrid = Integer.valueOf(getParameter("graph.grid")).intValue();
        graphHiGrid = Integer.valueOf(getParameter("graph.higrid")).intValue();
        graphHead = Integer.valueOf(getParameter("graph.head")).intValue();

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
//          GridBagConstraints gbc = new GridBagConstraints();
            setLayout( new BorderLayout() );
            ScrollPane sp = new ScrollPane( ScrollPane.SCROLLBARS_AS_NEEDED );
            sp.add( smppTopGraph );
            add(sp, BorderLayout.CENTER );

            setFont( new Font("Dialog", Font.BOLD, 12) );
            ViewButtonPanel pan = new ViewButtonPanel(TYPE_SMPP);
            add(pan, BorderLayout.SOUTH);

            smppTopGraph.requestFocus();
        }
    }

    class HttpPanel extends JPanel {
        public HttpPanel() {
            setLayout( new BorderLayout() );
            ScrollPane sp = new ScrollPane( ScrollPane.SCROLLBARS_AS_NEEDED );
            sp.add( httpTopGraph );
            add(sp, BorderLayout.CENTER );

            setFont( new Font("Dialog", Font.BOLD, 12) );
            ViewButtonPanel pan = new ViewButtonPanel(TYPE_HTTP);
            add(pan, BorderLayout.SOUTH);

            httpTopGraph.requestFocus();
        }
    }

    class MmsPanel extends JPanel {
        public MmsPanel() {
            setLayout(new BorderLayout());
            //add(mmsTopGraph, BorderLayout.CENTER);
            setFont(new Font("Dialog", Font.BOLD, 12));
        }
    }

    class ViewButtonPanel_ extends JPanel{

        public static final String BUTTON_SMPP_PROPERTIES = "Endpoints filter";
        public static final String BUTTON_HTTP_PROPERTIES = "Endpoints filter";
        public static final String BUTTON_SCALE_Y_IN = "Y scale +";
        public static final String BUTTON_SCALE_Y_OUT = "Y scale -";
        public static final String BUTTON_SCALE_X_IN = "Time scale +";
        public static final String BUTTON_SCALE_X_OUT = "Time scale -";
        public static final int SCALE_STEP_Y = 1;
        public static final int SCALE_STEP_X = 1;
        public static final String BUTTON_VIEW_GRAPH = "View/Hide graph";

        public ViewButtonPanel_(final String type){
            final JButton viewPropertiesButton;
            if( type.equals(TYPE_SMPP) ){
                viewPropertiesButton = new JButton(BUTTON_SMPP_PROPERTIES);
            }else if( type.equals(TYPE_HTTP) ){
                viewPropertiesButton = new JButton(BUTTON_HTTP_PROPERTIES);
            }else {
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
                            System.out.println("SMPP zoomYInListener. new is " + yScale + " Before " + (yScale-SCALE_STEP_Y) );
                        }else if( type.equals(TYPE_HTTP) ){
//                            graphScale+=SCALE_STEP;
//                            maxSpeed = 100;
                            System.out.println("HTTP zoomYInListener new is " + yScale + " Before " + (yScale-SCALE_STEP_Y) );
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
                            System.out.println("SMPP zoomXInListener new is " + yScale + " Before " + (xScale-SCALE_STEP_X) );
                        }else if( type.equals(TYPE_HTTP) ){
                            System.out.println("HTTP zoomXInListener new is " + yScale + " Before " + (xScale-SCALE_STEP_X) );
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

    class ViewButtonPanel extends JPanel{

        public static final String BUTTON_SMPP_PROPERTIES = "Endpoints filter";
        public static final String BUTTON_HTTP_PROPERTIES = "Endpoints filter";
        public static final String BUTTON_SCALE_Y_INC = "Y scale <|>";
        public static final String BUTTON_SCALE_Y_DEC = "Y scale >|<";
        public static final String BUTTON_SCALE_X_INC = "Time scale <|>";
        public static final String BUTTON_SCALE_X_DEC = "Time scale >|< ";
        public static final String BUTTON_SHOW_GRAPH = "Show graph";
        public static final String BUTTON_HIDE_GRAPH = "Hide graph";
        public static final String BUTTON_PAUSE_GRAPH = "Pause graph";
        public static final String BUTTON_CONTINUE_GRAPH = "Continue graph";

        public static final int SCALE_STEP_Y = 1;
        public static final int SCALE_STEP_X = 1;

        public ViewButtonPanel(final String type){
            final JButton viewPropertiesButton;
            if( type.equals(TYPE_SMPP) ){
                viewPropertiesButton = new JButton(BUTTON_SMPP_PROPERTIES);
            }else if( type.equals(TYPE_HTTP) ){
                viewPropertiesButton = new JButton(BUTTON_HTTP_PROPERTIES);
            }else {
                viewPropertiesButton = new JButton(BUTTON_HTTP_PROPERTIES);
            }
            final JButton incYButton = new JButton(BUTTON_SCALE_Y_INC);
            final JButton decYButton = new JButton(BUTTON_SCALE_Y_DEC);
            final JButton incXButton = new JButton(BUTTON_SCALE_X_INC);
            final JButton decXButton = new JButton(BUTTON_SCALE_X_DEC);
            final JButton showGraphButton =  new JButton(BUTTON_SHOW_GRAPH);
            final JButton pauseGraphButton = new JButton(BUTTON_PAUSE_GRAPH);

//            HardListener hardListener = new HardListener();
//            hardButton.addActionListener(hardListener);

//            setLayout( new GridLayout(1,3) );
            setLayout( new GridBagLayout() );
            GridBagConstraints gbc = new GridBagConstraints();
            gbc.gridheight = 1;
            gbc.weightx = 1;
            gbc.fill = GridBagConstraints.BOTH;

            gbc.gridy = 1;
            gbc.gridx = 1;
            gbc.gridheight = 2;
            add(viewPropertiesButton, gbc);
            gbc.gridheight = 1;
            gbc.gridy = 1;
            gbc.gridx = 2;
            add(incYButton, gbc);
            gbc.gridy = 1;
            gbc.gridx = 3;
            add(decYButton, gbc);

            gbc.gridy = 2;
            gbc.gridx = 2;
            add(incXButton, gbc);
            gbc.gridy = 2;
            gbc.gridx = 3;
            add(decXButton, gbc);

            gbc.gridy = 1;
            gbc.gridx = 4;
            gbc.gridheight = 2;
            add(showGraphButton, gbc);


            gbc.gridheight = 2;
            gbc.gridy = 1;
            gbc.gridx = 5;
            add(pauseGraphButton, gbc);

            ActionListener pauseGraphListener = new
                ActionListener(){
                    public void actionPerformed(ActionEvent event){
//                        pauseGraph = (pauseGraph == true)? false: true;
                        if( type.equals(TYPE_SMPP) ){
                            if( smppPauseGraph == true ){
                                smppPauseGraph = false;
                                pauseGraphButton.setText( BUTTON_PAUSE_GRAPH );
                            }else{
                                smppPauseGraph = true;
                                pauseGraphButton.setText( BUTTON_CONTINUE_GRAPH );
                            }
                            System.out.println("smpppPauseGraph has set to '" + smppPauseGraph + "'");
                        }else if( type.equals(TYPE_HTTP) ){
                            if( httpPauseGraph == true ){
                                httpPauseGraph = false;
                                pauseGraphButton.setText( BUTTON_PAUSE_GRAPH );
                            }else{
                                httpPauseGraph = true;
                                pauseGraphButton.setText( BUTTON_CONTINUE_GRAPH );
                            }
                            System.out.println("httpPauseGraph has set to '" + httpPauseGraph + "'");
                        }
                    }
                };
            pauseGraphButton.addActionListener( pauseGraphListener );

//            add(hardButton);

            ActionListener viewPropertiesListener = new
                ActionListener(){
                    public void actionPerformed(ActionEvent event){
                        ViewPropertyFrame blank = new ViewPropertyFrame(type);
                        blank.setVisible(true);
                    }
                };
            viewPropertiesButton.addActionListener( viewPropertiesListener );

            ActionListener viewGraphListener = new
                ActionListener(){
                    public void actionPerformed(ActionEvent event){
                        if( type.equals(TYPE_SMPP) ){
//                            smppViewGraph = (smppViewGraph == true)? false: true;
                            if( smppViewGraph == true ){
                                smppViewGraph = false;
                                showGraphButton.setText( BUTTON_SHOW_GRAPH );
                            }else{
                                smppViewGraph = true;
                                showGraphButton.setText( BUTTON_HIDE_GRAPH );
                            }
                        }else if( type.equals(TYPE_HTTP) ){
                            if( httpViewGraph == true ){
                                httpViewGraph = false;
                                showGraphButton.setText( BUTTON_SHOW_GRAPH );
                            }else{
                                httpViewGraph = true;
                                showGraphButton.setText( BUTTON_HIDE_GRAPH );
                            }

                        }
                    }
                };
            showGraphButton.addActionListener( viewGraphListener );

            ActionListener zoomYInListener = new
                ActionListener(){
                    public void actionPerformed(ActionEvent event){
                        if( type.equals(TYPE_SMPP) ){
                            smppYScale = smppYScale<1? smppYScale*2: smppYScale + SCALE_STEP_Y;
                            System.out.println("SMPP zoomYInListener. new is " + smppYScale + " Before " + (smppYScale - SCALE_STEP_Y) );
                        }else if( type.equals(TYPE_HTTP) ){
                            httpYScale = httpYScale<1? httpYScale*2: httpYScale + SCALE_STEP_Y;
                            System.out.println("HTTP zoomYInListener new is " + httpYScale + " Before " + (httpYScale - SCALE_STEP_Y) );
                        }
//                        smppTopGraph.maxSpeed = 200;
                        smppTopGraph.invalidate();
                    }
                };
            incYButton.addActionListener(zoomYInListener);

            ActionListener zoomYOutListener = new
                ActionListener(){
                    public void actionPerformed(ActionEvent event){
                        if( type.equals(TYPE_SMPP) ){
                            System.out.println("SMPP zoomYOutListener");
                            smppYScale = smppYScale>1?smppYScale - SCALE_STEP_Y: smppYScale>0.125? smppYScale/2: smppYScale;
                        }else if( type.equals(TYPE_HTTP) ){
                            System.out.println("HTTP zoomYOutListener");
                            httpYScale = httpYScale>1?httpYScale - SCALE_STEP_Y: httpYScale>0.125? httpYScale/2: httpYScale;
                        }
                        smppTopGraph.invalidate();
                    }
                };
            decYButton.addActionListener(zoomYOutListener);

            ActionListener zoomXInListener = new
                ActionListener(){
                    public void actionPerformed(ActionEvent event){
                        if( type.equals(TYPE_SMPP) ){
                            smppXScale += SCALE_STEP_X;
                            System.out.println("SMPP zoomXInListener new is " + smppYScale + " Before " + (smppXScale-SCALE_STEP_X) );
                        }else if( type.equals(TYPE_HTTP) ){
                            httpXScale += SCALE_STEP_X;
                            System.out.println("HTTP zoomXInListener new is " + httpXScale + " Before " + (httpXScale-SCALE_STEP_X) );
                        }
                        smppTopGraph.invalidate();
                    }
                };
            incXButton.addActionListener(zoomXInListener);

            ActionListener zoomXOutListener = new
                ActionListener(){
                    public void actionPerformed(ActionEvent event){
                        if( type.equals(TYPE_SMPP) ){
//                            System.out.println("SMPP zoomXOutListener");
                            smppXScale = smppXScale>1?smppXScale - SCALE_STEP_X: smppXScale;;
                        }else if( type.equals(TYPE_HTTP) ){
//                            System.out.println("HTTP zoomXOutListener");
                            httpXScale = httpXScale>1?httpXScale - SCALE_STEP_X: httpXScale;;
                        }
                        smppTopGraph.invalidate();
                    }
                };
            decXButton.addActionListener(zoomXOutListener);
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
            int height = (type.equals(TYPE_SMPP)?centerSnap.smppCount:centerSnap.httpCount)*strHeight;
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

    public static final String BUTTON_SELECT = "Select all";
    public static final String BUTTON_DESELECT = "Deselect all";

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

    class CheckboxPanel extends JPanel{
//        public CheckboxPanel( SvcSnap snaps ){
        public CheckboxPanel( String type ){
            if( type.equals(TYPE_SMPP)){
                int smppCount = centerSnap.smppCount;
                setLayout( new GridLayout(smppCount,1));
                for (int i = 0; i < smppCount; i++) {
                    String name = centerSnap.smppSnaps[i].smppId;
                    JCheckBox checkBox = new JCheckBox( name );
                    if( smppViewList == null || smppViewList.contains(centerSnap.smppSnaps[i].smppId) ){
                        checkBox.setSelected(true);
                    }
                    add(checkBox);
    //                checkBox.addActionListener( new CheckBoxListener() );
                }
            }else if( type.equals(TYPE_HTTP)){
                int httpCount = centerSnap.httpCount;
                setLayout( new GridLayout(httpCount,1));
                for (int i = 0; i < httpCount; i++) {
                    String name = centerSnap.httpSnaps[i].httpId;
                    JCheckBox checkBox = new JCheckBox( name );
                    if( httpViewList == null || httpViewList.contains(centerSnap.httpSnaps[i].httpId) ){
                        checkBox.setSelected(true);
                    }
                    add(checkBox);
    //                checkBox.addActionListener( new CheckBoxListener() );
                }

            }
        }
    }
    public static final String BUTTON_OK = "Ok";

    class OkCancelPanel extends JPanel{

        public static final String BUTTON_CLOSE = "Close";

        public OkCancelPanel( final JFrame frame, CheckboxPanel checkPanel, String type ){
            setLayout( new GridLayout(1,3));

            JButton okButton = new JButton(BUTTON_OK);
            JButton cancelButton = new JButton(BUTTON_CLOSE);

            add(okButton);
            add(cancelButton);

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

    class OkListener implements ActionListener{
        CheckboxPanel panel;
        String type;

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
    }

    protected void gotFirstSnap(ScSnap snap) {
        remove(connectingLabel);
        GridBagConstraints gbc = new GridBagConstraints();
        gbc.fill = GridBagConstraints.BOTH;

        snapSmppHistory = new SnapSmppHistory();
        snapHttpHistory = new SnapHttpHistory();

//        smppTopGraph = new SmppTopGraph(snap, maxSpeed, graphScale, graphGrid,
//                graphHiGrid, graphHead, localText, snapSmppHistory, smppViewList);

        System.out.println("gotFirstSnap:smppCount=" + snap.smppCount + "\tsmppViewList='" + smppViewList + "'" + "\nmaxSpeed=" + maxSpeed);
        smppTopGraph = new SmppTopGraph(snap, maxSpeed, graphScale, graphGrid,
                graphHiGrid, graphHead, localText, snapSmppHistory, smppViewList );

        httpTopGraph = new HttpTopGraph(snap, maxSpeed, graphScale, graphGrid,
                graphHiGrid, graphHead, localText, snapHttpHistory, httpViewList);

        JTabbedPane jTabbedPane = new JTabbedPane();
        jTabbedPane.addTab("SMPP", new SmppPanel());
        jTabbedPane.addTab("HTTP", new HttpPanel());
        jTabbedPane.addTab("MMS", new MmsPanel());
        //jTabbedPane.insertTab("SMPP", null, new SmppPanel(), null, 0);
        gbc.gridy = 1;
        gbc.gridx = 1;
        gbc.gridwidth = 1;
        gbc.weightx = 1;
        gbc.weighty = 1;
        gbc.fill = GridBagConstraints.BOTH;
        add(jTabbedPane, gbc);
        validate();
    }

    boolean isStopping = false;

    public void run() {
        Socket sock = null;
        DataInputStream is = null;
        isStopping = false;
        System.out.println("SccMon:run():host='" + getParameter("host") + "' port='" +  Integer.valueOf(getParameter("port")).intValue() + "'");
        try {
            while (!isStopping) {
                try {
                    System.out.println("ScMon:run():sock");
                    sock = new Socket(getParameter("host"), Integer.valueOf(getParameter("port")).intValue());
                    System.out.println("ScMon:run():getInputStream");
                    is = new DataInputStream(sock.getInputStream());
                    ScSnap snap = new ScSnap();
                    centerSnap = snap;
                    snap.read(is);

                    gotFirstSnap(snap);
                    while (!isStopping) {
                        System.out.println("ScMon:run():wihile2");
                        snap.read(is);
                        if( !smppPauseGraph ){
                            smppTopGraph.setSnap(snap, smppViewList, graphScale, maxSpeed, smppXScale, smppYScale, smppViewGraph);
                        }
                        if( !httpPauseGraph ){
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
