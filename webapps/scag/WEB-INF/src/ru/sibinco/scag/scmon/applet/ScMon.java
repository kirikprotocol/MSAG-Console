/*
 * Copyright (c) 2005 SibInco Inc. All Rights Reserved.
 */

package ru.sibinco.scag.scmon.applet;

import ru.sibinco.scag.scmon.ScSnap;

import javax.swing.*;
import java.awt.event.MouseListener;
import java.awt.event.ActionListener;
import java.awt.event.ItemListener;
import java.awt.event.MouseEvent;
import java.awt.event.ActionEvent;
import java.awt.event.ItemEvent;
import java.awt.*;
import java.util.ResourceBundle;
import java.util.Locale;
import java.net.Socket;
import java.io.DataInputStream;
import java.io.IOException;

public class ScMon extends JApplet implements Runnable, MouseListener, ActionListener, ItemListener {

    public static ResourceBundle localText;
    public static ResourceBundle messagesText;
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

    public void init() {

        System.out.println("Initing...");
        locale = new Locale(getParameter("locale.language").toLowerCase(), getParameter("locale.country").toLowerCase());
        localText = ResourceBundle.getBundle("ru.sibinco.scag.scmon.applet.text", locale);
        messagesText = ResourceBundle.getBundle("ru.sibinco.scag.scmon.applet.text", locale);
        maxSpeed = Integer.valueOf(getParameter("max.speed")).intValue();
        graphScale = Integer.valueOf(getParameter("graph.scale")).intValue();
        graphGrid = Integer.valueOf(getParameter("graph.grid")).intValue();
        graphHiGrid = Integer.valueOf(getParameter("graph.higrid")).intValue();
        graphHead = Integer.valueOf(getParameter("graph.head")).intValue();

        setFont(new Font("Dialog", Font.BOLD, 14));
        setLayout(new GridLayout());
        setBackground(SystemColor.control);
        GridBagConstraints gbc = new GridBagConstraints();
        gbc.fill = GridBagConstraints.BOTH;

        connectingLabel = new Label(localText.getString("connecting"));
        add(connectingLabel, gbc);
        validate();
    }

    protected void gotFirstSnap(ScSnap snap) {
        remove(connectingLabel);
        GridBagConstraints gbc = new GridBagConstraints();
        gbc.fill = GridBagConstraints.BOTH;

        snapSmppHistory = new SnapSmppHistory();
        snapHttpHistory = new SnapHttpHistory();

        smppTopGraph = new SmppTopGraph(snap, maxSpeed, graphScale, graphGrid,
                graphHiGrid, graphHead, localText, messagesText, snapSmppHistory);

        httpTopGraph = new HttpTopGraph(snap, maxSpeed, graphScale, graphGrid,
                graphHiGrid, graphHead, localText, messagesText, snapHttpHistory);

        JTabbedPane jTabbedPane = new JTabbedPane();
        jTabbedPane.addTab("SMPP", new SmppPanel());
        jTabbedPane.addTab("HTTP", new HttpPanel());
        jTabbedPane.addTab("MMS", new MmsPanel());

        gbc.gridy = 1;
        gbc.gridx = 1;
        gbc.gridwidth = 1;
        gbc.weightx = 1;
        gbc.weighty = 1;
        gbc.fill = GridBagConstraints.BOTH;
        add(jTabbedPane, gbc);
        validate();
    }

    class SmppPanel extends JPanel {
        public SmppPanel() {
            setLayout(new BorderLayout());
            add(smppTopGraph, BorderLayout.CENTER);
            setFont(new Font("Dialog", Font.BOLD, 12));
            smppTopGraph.requestFocus();
        }
    }

    class HttpPanel extends JPanel {
        public HttpPanel() {
            setLayout(new BorderLayout());
            add(httpTopGraph, BorderLayout.CENTER);
            setFont(new Font("Dialog", Font.BOLD, 12));
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
                    snap.read(is);

                    gotFirstSnap(snap);
                    while (!isStopping) {
                        snap.read(is);
                        smppTopGraph.setSnap(snap);
                        httpTopGraph.setSnap(snap);
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
