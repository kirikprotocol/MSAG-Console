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

    private int maxSpeed = 100;
    private int graphScale = 2;
    private int graphGrid = 5;
    private int graphHiGrid = 25;
    private int graphHead = 50;


    ScSnap centerSnap;
    public static final String BUTTON_SELECT = "Select all";
    public static final String BUTTON_DESELECT = "Deselect all";
    public static final String BUTTON_OK = "Ok";
    public static final String BUTTON_CLOSE = "Close";
    public static final String BUTTON_SMPP_PROPERTIES = "SMPP View properties";
    public static final String BUTTON_HTTP_PROPERTIES = "HTTP View properties";
    public static final String BUTTON_ZOOM_IN = "ZOOM IN";
    public static final String BUTTON_ZOOM_OUT = "ZOOM OUT";

    public static final String TYPE_SMPP = "SMPP";
    public static final String TYPE_HTTP = "HTTP";

    public static final int SCALE_STEP = 1;

    public HashSet smppViewList;
    public HashSet httpViewList;


    public void init() {

        System.out.println("Initing...");
        localText = new RemoteResourceBundle(getCodeBase(),getParameter("resource_servlet_uri"));
        locale=localText.getLocale();
        maxSpeed = Integer.valueOf(getParameter("max.speed")).intValue();
        graphScale = Integer.valueOf(getParameter("graph.scale")).intValue();
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
            setLayout(new BorderLayout());
            add(smppTopGraph, BorderLayout.CENTER);
            setFont(new Font("Dialog", Font.BOLD, 12));

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
            final JButton zoomInPropButton = new JButton(BUTTON_ZOOM_IN);
            final JButton zoomOutPropButton = new JButton(BUTTON_ZOOM_OUT);

            setLayout( new GridLayout(1,3) );
            add(viewPropertiesButton);
            add(zoomInPropButton);
            add(zoomOutPropButton);
//            add(hardButton);

            ActionListener viewPropertiesListener = new
                ActionListener(){
                    public void actionPerformed(ActionEvent event){
                        ViewPropertyFrame blank = new ViewPropertyFrame(type);
                        blank.setVisible(true);
                    }
                };
            viewPropertiesButton.addActionListener( viewPropertiesListener );

            ActionListener zoomInListener = new
                ActionListener(){
                    public void actionPerformed(ActionEvent event){
                        if( type.equals(TYPE_SMPP) ){
                            System.out.println("SMPP zoomInListener");
                            graphScale+=SCALE_STEP;
                        }else if( type.equals(TYPE_HTTP) ){
                            System.out.println("HTTP zoomInListener");
                            graphScale+=SCALE_STEP;
                        }
                    }
                };
            zoomInPropButton.addActionListener(zoomInListener);

            ActionListener zoomOutListener = new
                ActionListener(){
                    public void actionPerformed(ActionEvent event){
                        if( type.equals(TYPE_SMPP) ){
                            System.out.println("SMPP zoomOutListener");
                            graphScale-=SCALE_STEP;
                        }else if( type.equals(TYPE_HTTP) ){
                            System.out.println("HTTP zoomOutListener");
                            graphScale-=SCALE_STEP;
                        }

                    }
                };
            zoomOutPropButton.addActionListener(zoomOutListener);
        }
    }

    class ViewPropertyFrame extends JFrame{
        public ViewPropertyFrame(String type){

            setTitle( type + " View proprties" );
            int height;
//            JCheckBox cb = new JCheckBox("A");
//            height = cb.getHeight();
//            System.out.println("CB height=" + height );
            height = 30;
//            setSize( 230, 100 +(type.equals(TYPE_SMPP)?svcSnap.smppCount:svcSnap.httpCount)*height );
            setSize( 230, 300 );
            setLocation( 200, 200 );
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
            add(checkPanel, BorderLayout.CENTER);
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

    class OkCancelPanel extends JPanel{
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
    } //end OkListener

    class HttpPanel extends JPanel {
        public HttpPanel() {
            setLayout(new BorderLayout());
            //add(httpTopGraph, BorderLayout.CENTER);
            setFont(new Font("Dialog", Font.BOLD, 12));
            //httpTopGraph.requestFocus();
        }
    }

    class MmsPanel extends JPanel {
        public MmsPanel() {
            setLayout(new BorderLayout());
            //add(smppTopGraph, BorderLayout.CENTER);
            setFont(new Font("Dialog", Font.BOLD, 12));
        }
    }


    protected void gotFirstSnap(ScSnap snap) {
        remove(connectingLabel);
        GridBagConstraints gbc = new GridBagConstraints();
        gbc.fill = GridBagConstraints.BOTH;

        snapSmppHistory = new SnapSmppHistory();
        snapHttpHistory = new SnapHttpHistory();

        smppTopGraph = new SmppTopGraph(snap, maxSpeed, graphScale, graphGrid,
                graphHiGrid, graphHead, localText, snapSmppHistory, smppViewList);

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
        try {
            while (!isStopping) {
                try {
                    sock = new Socket(getParameter("host"), Integer.valueOf(getParameter("port")).intValue());
                    is = new DataInputStream(sock.getInputStream());
                    ScSnap snap = new ScSnap();
                    centerSnap = snap;
                    snap.read(is);

                    gotFirstSnap(snap);
                    while (!isStopping) {
                        snap.read(is);
                        smppTopGraph.setSnap(snap, smppViewList, graphScale);
                        httpTopGraph.setSnap(snap, httpViewList, graphScale);
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
