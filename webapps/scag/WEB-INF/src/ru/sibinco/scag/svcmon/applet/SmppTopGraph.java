/*
 * Copyright (c) 2005 SibInco Inc. All Rights Reserved.
 */
package ru.sibinco.scag.svcmon.applet;

import ru.sibinco.scag.svcmon.SvcSnap;
import ru.sibinco.scag.svcmon.snap.SmppSnap;
import ru.sibinco.scag.util.RemoteResourceBundle;

import javax.swing.*;
import java.awt.*;
import java.awt.event.KeyEvent;
import java.awt.event.KeyListener;
import java.awt.event.MouseEvent;
import java.awt.event.MouseListener;
import java.awt.event.MouseMotionListener;
import java.util.Comparator;
import java.util.HashSet;
import java.util.Date;
import java.util.Calendar;
import java.text.SimpleDateFormat;
import java.util.Vector;
import java.util.ArrayList;

/**
 * The <code>SmppTopGraph</code> class represents
 * <p><p/>
 * Date: 09.12.2005
 * Time: 11:20:32
 *
 * @author &lt;a href="mailto:igor@sibinco.ru"&gt;Igor Klimenko&lt;/a&gt;
 */
public class SmppTopGraph extends Canvas implements MouseListener, MouseMotionListener, KeyListener {
    static final int pad = 1;
    private static final Color tipBackground = SystemColor.control;
    private static final Color tipTextColor = SystemColor.controlText;
    private static final Color colorBackground = Color.black;
    private static Color colorHiBackground;
    static{
        try{
            colorHiBackground = new Color(colorBackground.getRed(), colorBackground.getGreen() + 64, colorBackground.getBlue());
        }catch( Exception e ){
            colorHiBackground = colorBackground;
        }
    }
    private static final Color headColor = Color.lightGray;
    private static final Color graphColor = Color.green;
    private static Color graphHiGridColor;
    static{
        try{
            graphHiGridColor  = new Color(colorBackground.getRed(), colorBackground.getGreen() + 128, colorBackground.getBlue());
        }catch( Exception e ){
            graphHiGridColor = colorBackground;
        }
    }
    private static Color graphGridColor;
    static{
        try{
            graphGridColor = new Color(colorBackground.getRed(), colorBackground.getGreen() + 80, colorBackground.getBlue());
        }catch( Exception e ){
            graphGridColor = colorBackground;
        }
    }
    private static Color graphBarBGColor;
    static{
        try{
            graphBarBGColor = new Color(colorBackground.getRed(), colorBackground.getGreen() + 96, colorBackground.getBlue());
        }catch( Exception e ){
            graphBarBGColor = colorBackground;
        }
    }
    private static final Color smeHiColor = Color.white;
    private static final Color smeColor = Color.lightGray;

    private static final Color columnsColor[] = {
            new Color(128, 128, 224), // accepted
            new Color(224, 224, 224), // rejected
            new Color(128, 224, 128), // delivered
            new Color(128, 224, 224), // Gw rejected
            new Color(224, 128, 128) // Failed
    };

//    private static final Color columnsColor[] = {
//            Color.blue, // accepted 0
//            Color.white, // rejected 1
//            Color.green, // delivered 2
//            Color.cyan, // Gw rejected 3
//            Color.red // Failed 4
//    };

    private static final Color colorGraphAccepted = Color.blue;
    private static final Color colorGraphRejected = Color.white;
    private static final Color colorGraphDeliver = Color.green;
    private static final Color colorGraphGwRejected = Color.cyan;
    private static final Color colorGraphFailed = Color.red;

    Image offscreen;
    int bottomSpace = pad;
    int topSpace = pad;
    int counterWidth = 0;
    int smeNameWidth = 0;
    int maxSpeed = 200;
    int rowHeight = SmppSnap.COUNTERS * 2;
    RemoteResourceBundle localeText;
//    int graphScale;
    float graphScale;
    int graphGrid;
    int graphHiGrid;
    int graphHead;
    int headerHeight;
    int smppListStart;
    int smppGraphStart;
    int separatorWidth = 0; //igor

    int mouseX;
    int mouseY;

    private static final int SORT_DESC = 1;
    private static final int SORT_ASC = 2;
    int smeSortId = -1;
    int smeSortState;
    Comparator smppComparator = null;

    int split = -1;
    int shiftStep = 10;

    SvcSnap snap;
    SnapSmppHistory snapSmppHistory;

    Font graphFont;
    int graphTextWidth;
    int barSeparator = 2;
    java.util.HashSet smppViewList;

    boolean swich = false;
    double yScale = 1;
    int xScale = 10;
    int rest = 0;
//    float yParam = 1;
    public static final int DELIMITER = 2;
    boolean viewGraph = false;

    int shiftV = 0;

    public SmppTopGraph(SvcSnap snap, int maxSpeed, int graphScale,
                        int graphGrid, int graphHiGrid,
                        int graphHead, RemoteResourceBundle localeText,
                        SnapSmppHistory snapSmppHistory) {
        super();
        this.maxSpeed = maxSpeed;
        this.localeText = localeText;
        this.graphScale = graphScale;
        this.graphGrid = graphGrid;
        this.graphGrid = (int)Math.round(graphGrid*0.1)*10;
        this.graphHiGrid = graphHiGrid;
        this.graphHead = graphHead;
        this.snapSmppHistory = snapSmppHistory;
        addMouseListener(this);
        addMouseMotionListener(this);
        addKeyListener(this);
        graphFont = new Font("dialog", Font.PLAIN, 10);
        setSnap(snap);

    }

    public SmppTopGraph(SvcSnap snap, int maxSpeed, int graphScale,
                        int graphGrid, int graphHiGrid,
                        int graphHead, RemoteResourceBundle localeText,
                        SnapSmppHistory snapSmppHistory, HashSet viewList) {
        super();
        this.maxSpeed = maxSpeed;
        this.localeText = localeText;
        this.graphScale = graphScale;
//        this.graphGrid = graphGrid;
        this.graphGrid = graphGrid<=5? 5:(int)Math.round(graphGrid*0.1)*10;
        this.graphHiGrid = graphHiGrid;
        this.graphHead = graphHead;
        this.snapSmppHistory = snapSmppHistory;
        addMouseListener(this);
        addMouseMotionListener(this);
        addKeyListener(this);
        graphFont = new Font("dialog", Font.PLAIN, 10);
        setSnap(snap);
        this.smppViewList = viewList;
//        System.out.println("--------\nSmppTopGraphstart: SmppTopGraphstart(): \nthis.maxSpeed=" + this.maxSpeed + "\nthis.graphScale=" + this.graphScale +
//                "\nthis.graphGrid=" + this.graphGrid + "\nthis.graphHiGrid=" + this.graphHiGrid + "\nthis.graphHead=" + this.graphHead + "\n---------");
    }

    public SmppTopGraph(SvcSnap snap, int maxSpeed, int graphScale,int graphGrid, int graphHiGrid, int graphHead,
                        RemoteResourceBundle localeText,SnapSmppHistory snapSmppHistory,
                        HashSet viewList, boolean viewGraph) {
        super();
        this.maxSpeed = maxSpeed;
        this.localeText = localeText;
        this.graphScale = graphScale;
//        this.graphGrid = graphGrid;
//        this.graphGrid   = graphGrid<=5? 5:(int)Math.round(graphGrid*0.1)*10;
//        this.graphHiGrid = graphHiGrid<=5? 5:(int)Math.round(graphHiGrid*0.1)*10;
//        this.graphHead   = graphHiGrid<=5? 5:(int)Math.round(graphHead*0.1)*10;

        this.graphGrid   = graphGrid   < 3?   3: graphGrid;
        this.graphHiGrid = graphHiGrid < 3? 3: graphHiGrid;
        this.graphHead   = graphHead   < 3?   3: graphHead;

        this.snapSmppHistory = snapSmppHistory;
        addMouseListener(this);
        addMouseMotionListener(this);
        addKeyListener(this);
        graphFont = new Font("dialog", Font.PLAIN, 10);
        setSnap(snap);
        this.smppViewList = viewList;
        this.viewGraph = viewGraph;
//        System.out.println("--------\nSmppTopGraphstart: SmppTopGraphstart(): \nthis.maxSpeed=" + this.maxSpeed + "\nthis.graphScale=" + this.graphScale +
//                "\nthis.graphGrid=" + this.graphGrid + "\nthis.graphHiGrid=" + this.graphHiGrid + "\nthis.graphHead=" + this.graphHead + "\n---------");
    }

    public void setSnap(SvcSnap snap) {
        snapSmppHistory.addSnap(snap);
        this.snap = new SvcSnap(snap);
        if (smppComparator != null)
            this.snap.sortSmppSnaps(smppComparator);

        repaint();
    }

    public void setSnap(SvcSnap snap, HashSet viewList) {
        this.smppViewList = viewList;
        snapSmppHistory.addSnap(snap);
        this.snap = new SvcSnap(snap);
        if (smppComparator != null)
            this.snap.sortSmppSnaps(smppComparator);

        repaint();
    }

    public void setSnap(SvcSnap snap, HashSet viewList, int scale) {
        //System.out.println( "SmppTopGraph:setSnap(svs) " + snap + "\n" + viewList + "\n" + scale );
        snapSmppHistory.addSnap(snap);
        this.snap = new SvcSnap(snap);
        this.smppViewList = viewList;
        this.graphScale = scale;
        if (smppComparator != null)
            this.snap.sortSmppSnaps(smppComparator);

        repaint();
    }

    public void setSnap(SvcSnap snap, HashSet viewList, int scale, int max) {
        //System.out.println( "---\nSmppTopGraph:setSnap(svsm) '" + snap + "'\nviewList='" + viewList + "'\t\nscale=" + scale + "\t\nmaxSpeed='" + maxSpeed + "'");
        snapSmppHistory.addSnap(snap);
        this.snap = new SvcSnap(snap);
        this.smppViewList = viewList;
        this.graphScale = scale;
        this.maxSpeed = max;
        if (smppComparator != null){
            this.snap.sortSmppSnaps(smppComparator);
        }
        repaint();
    }

    public void setSnap(SvcSnap snap, HashSet viewList, int scale, int maxSpeed, float xScale, double yScale) {
        //System.out.println( "---\nSmppTopGraph:setSnap(svsmxy) '" + snap + "'\nviewList='" + viewList + "'\t\nscale=" + scale + "\t\nmaxSpeed='" + maxSpeed + "'");
        snapSmppHistory.addSnap(snap);
        this.snap = new SvcSnap(snap);
        this.smppViewList = viewList;
        this.graphScale = scale;
        this.maxSpeed = maxSpeed;
        this.yScale = yScale;
        this.xScale = (int)xScale;
        if (smppComparator != null){
            this.snap.sortSmppSnaps(smppComparator);
        }
        rest += xScale;
        repaint();
    }

    public void setSnap(SvcSnap snap, HashSet viewList, int scale, int maxSpeed, float xScale, double yScale, boolean viewGraph) {
        /*System.out.println( "---\nSmppTopGraph:setSnap(svsmxyv) '" + snap + "'\nviewList='" + viewList + "'\t\nscale=" + scale + "\t\nmaxSpeed='" + maxSpeed + "'" +
               "\nxScale='" + xScale + "'\nyScale='" + yScale +"'\nviewGraph='" + viewGraph + "'" );      */
        snapSmppHistory.addSnap(snap);
        this.snap = new SvcSnap(snap);
        this.smppViewList = viewList;
        this.graphScale = scale;
        this.maxSpeed = maxSpeed;
        this.yScale = yScale;
        this.xScale = (int)xScale;
        if (smppComparator != null){
            this.snap.sortSmppSnaps(smppComparator);
        }
        rest += xScale;
        this.viewGraph = viewGraph;
        repaint();
    }

    public void setSnap(SvcSnap snap, HashSet viewList, int scale, int maxSpeed, float xScale, double yScale, boolean viewGraph, int shiftV) {
        /*System.out.println( "---\nSmppTopGraph:setSnap(svsmxyv) '" + snap + "'\nviewList='" + viewList + "'\t\nscale=" + scale + "\t\nmaxSpeed='" + maxSpeed + "'" +
                "\nxScale='" + xScale + "'\nyScale='" + yScale +"'\nviewGraph='" + viewGraph + "'\nshiftV='" + shiftV + "'" );  */
        snapSmppHistory.addSnap(snap);
        this.snap = new SvcSnap(snap);
        this.smppViewList = viewList;
        this.graphScale = scale;
        this.maxSpeed = maxSpeed;
        this.yScale = yScale;
        this.xScale = (int)xScale;
        if (smppComparator != null){
            this.snap.sortSmppSnaps(smppComparator);
        }
        rest += xScale;
        this.viewGraph = viewGraph;
        this.shiftV = shiftV;
        repaint();
    }

    public void invalidate() {
        swich = swich?false:true;
        Font font = getFont();
        //System.out.println("invalidate():maxSpeed='" + maxSpeed + "'");
        if (font != null) {
            FontMetrics fm = getFontMetrics(font);
            FontMetrics fmg = getFontMetrics(graphFont);
            Dimension sz = getSize();
            if (sz.width > 0 && sz.height > 0) {
                if (fm.getHeight() > SmppSnap.COUNTERS * 2) {
                    rowHeight = fm.getHeight() + 1;
                } else {
                    rowHeight = SmppSnap.COUNTERS * 2;
                }
                bottomSpace = 2 * pad + fm.getDescent() + fm.getHeight();
                topSpace = pad + fm.getAscent();
                headerHeight = fm.getHeight() + pad + 3;
                counterWidth = fm.stringWidth("0000");
                smeNameWidth = fm.stringWidth("WWWWWWWWWWWWWWWW");
                smppListStart = pad + separatorWidth + pad; //igor
                smppGraphStart = smppListStart + pad + smeNameWidth + (counterWidth + pad) *
                        (SmppSnap.COUNTERS * 2) + pad + pad;
                prefsz = new Dimension((int)(smppGraphStart + 1 + maxSpeed * graphScale + pad),
                        headerHeight + rowHeight * 6 + pad);

                graphTextWidth = fmg.stringWidth("0000");
//                split = (int)(maxSpeed * graphScale + rowHeight + separatorWidth + 2 * pad);
            }
        }
        offscreen = null;
        super.invalidate();
    }

    public void paint(Graphics gg) {
        //System.out.println( "SmppTopGraph.paint() start" );

//        setSize(100, 100);

        Dimension size = getSize();

        if( !(size.width > 0 && size.height > 0) ) return;
        //System.out.println("SmppTopGraph.paint() width=" + size.width + " height=" + size.height );
        if (offscreen == null) {
            offscreen = createImage(size.width, size.height);
        }
        Graphics g = offscreen.getGraphics();
        Font font = getFont();
        FontMetrics fm = getFontMetrics(font);
        int fh = fm.getHeight();
        g.setColor(colorBackground);
        g.fillRect(0, 0, size.width, size.height);

//        g.setClip(0, 0, size.width, size.height - split);
        g.setClip(0, 0, size.width, size.height);
        drawSortState(g, fm);
        // drawing heading
        g.setColor(SystemColor.control);
        g.drawLine(pad, pad + fh + 1, size.width - pad, pad + fh + 1);

        int x = 0;
        g.setColor(headColor);
        int hpos = pad + fm.getAscent();

        x = smppListStart;
        g.setColor(smeColor);
        g.drawString(localeText.getString("snh.smename"), x + pad, hpos);
        x += smeNameWidth;
        String avgStr = localeText.getString("snh.count.avg");
        for (int i = 0; i < SmppSnap.COUNTERS; i++) {
            g.setColor(columnsColor[i]);
            drawCounterHead(g, localeText.getString("snh.count." + i), x + pad, hpos, fm);
            x += pad + counterWidth;
            drawCounterHead(g, avgStr, x + pad, hpos, fm);
            x += pad + counterWidth;
        }
        // draw graph scale
        x = smppGraphStart + 1;
        g.setColor(graphColor);
        for (int i = graphHead; ; i += graphHead) {
            int pos = x + (int)(i * graphScale);
            if (pos >= size.width - pad) break;
//            String s = String.valueOf(i);
            String s = String.valueOf(new Float(i/yScale).intValue());
            g.drawString(s, pos - (fm.stringWidth(s) / 2), hpos);
        }

        g.setColor(headColor);
        g.drawLine(pad, pad + fh + 1, size.width - pad, pad + fh + 1);

        // draw counters
        int y = headerHeight;
        x = 0;
        y = pad + fh + 1;
        x = smppListStart;
        int ii = 0;
        for (int i = 0; i < snap.smppCount; i++) {
            if(  smppViewList == null || smppViewList.contains( ( (SmppSnap)snap.smppSnaps[i]).smppId ) ){
                if ((ii % 2) == 0) {
                    g.setColor(colorHiBackground);
                    g.fillRect(x + pad, y, size.width - x - 2 * pad, rowHeight);
                }
                drawSmppSnap(g, i, x, y, size, fm);
                y += rowHeight;
                ii++;
            }
        }
//        g.setClip(0, size.height - split + separatorWidth, size.width, split - separatorWidth);
        g.setClip(0, size.height/DELIMITER, size.width, size.height);
//        g.clearRect(0, size.height - split + separatorWidth, size.width, split - separatorWidth);
//        g.setColor(Color.RED);
//        g.fillRect(0, size.height - split + separatorWidth, size.width, split - separatorWidth);
//        drawGraph(g, size);
//        g.setClip(0, 0, size.width, size.height);
        if(viewGraph){
//            g.clearRect(0, size.height/DELIMITER, size.width, size.height);
            g.setColor(colorBackground);
            g.fillRect( 0, size.height/DELIMITER, size.width, size.height );
            drawSeparator(g, size);
            g.setClip(0, size.height/DELIMITER+10, size.width, size.height);
            drawGraph(g, size);
        }
        g.setClip(0, 0, size.width, size.height);
//        System.out.println("paint():maxSpeed='" + maxSpeed + "'\n================================");
        gg.drawImage(offscreen, 0, 0, null);
        g.dispose();
    }

    int top_sep = 0;

    void drawSeparator(Graphics g, Dimension size) {
        g.setColor(SystemColor.control);
        int hp = (int)size.height/DELIMITER;
        top_sep = hp;
        g.setColor(SystemColor.controlHighlight);
        if (split < 0){ g.setColor(SystemColor.controlShadow); }
        g.drawLine(0, hp, size.width - pad, hp);
    }

    void drawSmppSnap(Graphics g, int i, int x, int y, Dimension size, FontMetrics fm) {
        int hpos = y + fm.getAscent();
        SmppSnap ss = snap.smppSnaps[i];
        if( snapSmppHistory.getCurrentSmpp() != null && snapSmppHistory.getCurrentSmpp().equals(ss.smppId) )
            g.setColor(smeHiColor);
        else
            g.setColor(smeColor);
        g.drawString(ss.smppId, x + pad, hpos);
        x += pad + smeNameWidth;
        for (int k = 0; k < SmppSnap.COUNTERS; k++) {
            g.setColor(columnsColor[k]);
            drawCounter(g, ss.smppSpeed[k], x + pad, hpos, fm);
            x += pad + counterWidth;
            drawCounter(g, ss.smppAvgSpeed[k], x + pad, hpos, fm);
            x += pad + counterWidth;
        }
        x += pad + pad;
        for (int k = 0; k < SmppSnap.COUNTERS; k++) {
            if (ss.smppSpeed[k] > 0) {
                g.setColor(columnsColor[k]);
//                g.fillRect(x + 1, y + 1 + k * 2, (int)(ss.smppSpeed[k] * graphScale), 2);
                g.fillRect(x + 1, y + 1 + k * 2, (int)(ss.smppSpeed[k] * graphScale * yScale), 2);
            }
        }
        g.setColor(graphColor);
        g.drawLine(x + pad, y, x + pad, y + rowHeight);
        x++;
        for (int k = graphGrid; ; k += graphGrid) {
            int pos = x + (int)(k * graphScale);
            if (pos >= (size.width - pad)) break;
            if ((k % graphHiGrid) == 0) {
                g.setColor(graphHiGridColor);
            } else {
                g.setColor(graphGridColor);
            }
            g.drawLine(pos, y, pos, y + rowHeight);
        }
    }

    void drawCounter(Graphics g, int cnt, int x, int y, FontMetrics fm) {
        String cc = String.valueOf(cnt);
        g.drawString(cc, x + counterWidth - fm.stringWidth(cc), y);
    }

    void drawCounterHead(Graphics g, String s, int x, int y, FontMetrics fm) {
        g.drawString(s, x + counterWidth - fm.stringWidth(s), y);
    }

    void drawSortState(Graphics g, FontMetrics fm) {
        if (smeSortId == 0) {
            drawSortGradient(g, smppListStart, smeNameWidth, fm, smeSortState == SORT_ASC);
        } else if (smeSortId > 0) {
            drawSortGradient(g, smppListStart + smeNameWidth + pad + (counterWidth + pad) * (smeSortId - 1), counterWidth, fm, smeSortState == SORT_ASC);
        }
    }

    final static int gradientStep = 10;

    void drawSortGradient(Graphics g, int x, int len, FontMetrics fm, boolean ascent) {
        int fh = fm.getHeight();
        int red = colorBackground.getRed();
        int green = colorBackground.getGreen();
        int blue = colorBackground.getBlue();
        if (ascent) {
            for (int i = pad; i <= fh; i++) {
                g.setColor(new Color(red + i * gradientStep, green + i * gradientStep, blue + i * gradientStep));
                g.drawLine(x, i, x + len, i);
            }
        } else {
            for (int i = pad; i <= fh; i++) {
                g.setColor(new Color(red + i * gradientStep, green + i * gradientStep, blue + i * gradientStep));
                g.drawLine(x, fh + pad - i, x + len, fh + pad - i);
            }
        }
    }

    int drawBars(Graphics g, int top, int y, int barwidth, SmppSnap smesnap){
// draw bars
        int spent = 0;
        int barX = pad + graphTextWidth + barSeparator + pad + barwidth-2;

        g.setColor(graphBarBGColor);

        g.drawString( "XS=" + xScale, barX-20, y+10);

        g.drawString( "YS=" + yScale, barX+10, y+10);

    // last Rejected
        g.setColor(graphBarBGColor);
        g.fillRect(barX, top, barwidth, y-top);
        g.setColor(colorGraphRejected);
        int barHeight = new Float(smesnap.smppSpeed[SmppSnap.REJECTED_INDEX] * graphScale * yScale).intValue();
        g.fillRect(barX, y - barHeight + shiftV, barwidth, barHeight + shiftV );
//        spent += barHeight;
    // last Gw Rejected
        barX += barwidth + barSeparator;
        g.setColor(graphBarBGColor);
        //        g.fillRect(barx, y - (int)(maxSpeed*yParam), barwidth, (int)(maxSpeed*yParam));
        g.fillRect(barX, top, barwidth, y-top);
        g.setColor(colorGraphGwRejected);
        barHeight = new Float(smesnap.smppSpeed[SmppSnap.GW_REJECTED_INDEX] * graphScale * yScale).intValue();
        g.fillRect(barX, y - barHeight - spent + shiftV, barwidth, barHeight + spent + shiftV );
//        spent += barHeight;
    // last Accepted
        barX += barwidth + barSeparator;
        g.setColor(graphBarBGColor);
        //        g.fillRect(barx, y - (int)(maxSpeed*yParam), barwidth, (int)(maxSpeed*yParam));
        g.fillRect(barX, top, barwidth, y-top);
        g.setColor(colorGraphAccepted);
        barHeight = new Float(smesnap.smppSpeed[SmppSnap.ACCEPTED_INDEX] * graphScale * yScale).intValue();
        g.fillRect(barX, y - barHeight - spent + shiftV, barwidth, barHeight + spent + shiftV );
        spent = 0;
    // last Failed
        barX += barwidth + barSeparator;
        g.setColor(graphBarBGColor);
        //        g.fillRect(barx, y - (int)(maxSpeed*yParam), barwidth, (int)(maxSpeed*yParam));
        g.fillRect(barX, top, barwidth, y-top);
        g.setColor(colorGraphFailed);
        barHeight = new Float(smesnap.smppSpeed[SmppSnap.FAILED_INDEX] * graphScale * yScale).intValue();
        g.fillRect(barX, y - barHeight + shiftV, barwidth, barHeight + shiftV );
//        spent += barHeight;
// last Deliver
        barX += barwidth + barSeparator;
        g.setColor(graphBarBGColor);
//        g.fillRect(barx, y - (int)(maxSpeed*yParam), barwidth, (int)(maxSpeed*yParam));
        g.fillRect(barX, top, barwidth, y-top);
        g.setColor(colorGraphDeliver);
        barHeight = new Float(smesnap.smppSpeed[SmppSnap.DELIVERED_INDEX] * graphScale * yScale).intValue();
        g.fillRect(barX, y - barHeight - spent + shiftV, barwidth, barHeight + spent + shiftV );

        return barX;
    }

    void drawHorizontal( Graphics g, int top, int y, int barX, int barWidth, int rightX ){
//  performance values
        FontMetrics fm = getFontMetrics( graphFont );
        for (int i = 0; y + shiftV - (i * graphScale) >= top; i += graphHead) {
    //        for (int i = 0; (i * graphScale) <maxSpeed; i += graphHead) {
            String s = String.valueOf( new Float(i/yScale).intValue() );
            g.drawString( s, graphTextWidth - fm.stringWidth(s), y - (int)(i * graphScale) + fm.getDescent() + 5 + shiftV);
        }
//  time line
        int lineLeft = pad + graphTextWidth + pad;
        g.drawLine(lineLeft, y, rightX - pad, y);
        y--;

        int graphXLeft = barX + barWidth*3;
//  horizontal lines
        for (int i = graphGrid; ; i += graphGrid) {
            int pos = y - (int)(i * graphScale) + shiftV;

            if( pos <= top ) break;

            g.setColor(graphGridColor);
            if ((i % graphHiGrid) == 0) {
                g.setColor(graphHiGridColor);
            } else {
                g.setColor(graphGridColor);
            }
            g.drawLine(lineLeft, pos, rightX, pos);
        }
    }



    void drawGraph_(Graphics g, Dimension size) {
        //System.out.println("drawGraph():start");

        SmppSnap smesnap = snapSmppHistory.getSmppLast();
        if (smesnap == null) return;
        if ( !viewGraph ) return;

        SmppSnap prevsnap = smesnap;
//        int spent = 0;

        g.setColor(graphHiGridColor);
        FontMetrics fm = getFontMetrics(graphFont);
        int fh = fm.getHeight();
        int height = 0;
//        height = split - separatorWidth - pad;
//        System.out.println( "size.height=" + size.height + " size.width=" + size.getWidth() );
        int top = size.height - height;
        top = top_sep + 10;
//        System.out.println("top="+ top + " | heigth=" + height);
        int y = size.height - pad - fh - pad;
        int barwidth = (graphTextWidth - 3 * barSeparator - 5 * pad) / 2;
        g.setFont(graphFont);
//// draw bars
        int barX = drawBars( g, y, top, barwidth, smesnap );

//        int barx = pad + graphTextWidth + barSeparator + pad + barwidth-2;
//        g.setColor(graphBarBGColor);
//
//        g.drawString( "XS=" + xScale, barx-20, y+10);
//        g.drawString( "YS=" + yScale, barx+10, y+10);
//
//    // last Rejected
//        g.setColor(graphBarBGColor);
//        g.fillRect(barx, top, barwidth, y-top);
//        g.setColor(colorGraphRejected);
//        int barHeight = new Float(smesnap.smppSpeed[SmppSnap.REJECTED_INDEX] * graphScale * yScale).intValue();
//        g.fillRect(barx, y - barHeight + shiftV, barwidth, barHeight + shiftV );
////        spent += barHeight;
//    // last Gw Rejected
//        barx += barwidth + barSeparator;
//        g.setColor(graphBarBGColor);
//        //        g.fillRect(barx, y - (int)(maxSpeed*yParam), barwidth, (int)(maxSpeed*yParam));
//        g.fillRect(barx, top, barwidth, y-top);
//        g.setColor(colorGraphGwRejected);
//        barHeight = new Float(smesnap.smppSpeed[SmppSnap.GW_REJECTED_INDEX] * graphScale * yScale).intValue();
//        g.fillRect(barx, y - barHeight - spent + shiftV, barwidth, barHeight + spent + shiftV );
////        spent += barHeight;
//    // last Accepted
//        barx += barwidth + barSeparator;
//        g.setColor(graphBarBGColor);
//        //        g.fillRect(barx, y - (int)(maxSpeed*yParam), barwidth, (int)(maxSpeed*yParam));
//        g.fillRect(barx, top, barwidth, y-top);
//        g.setColor(colorGraphAccepted);
//        barHeight = new Float(smesnap.smppSpeed[SmppSnap.ACCEPTED_INDEX] * graphScale * yScale).intValue();
//        g.fillRect(barx, y - barHeight - spent + shiftV, barwidth, barHeight + spent + shiftV );
//        spent = 0;
//    // last Failed
//        barx += barwidth + barSeparator;
//        g.setColor(graphBarBGColor);
//        //        g.fillRect(barx, y - (int)(maxSpeed*yParam), barwidth, (int)(maxSpeed*yParam));
//        g.fillRect(barx, top, barwidth, y-top);
//        g.setColor(colorGraphFailed);
//        barHeight = new Float(smesnap.smppSpeed[SmppSnap.FAILED_INDEX] * graphScale * yScale).intValue();
//        g.fillRect(barx, y - barHeight + shiftV, barwidth, barHeight + shiftV );
////        spent += barHeight;
//// last Deliver
//        barx += barwidth + barSeparator;
//        g.setColor(graphBarBGColor);
////        g.fillRect(barx, y - (int)(maxSpeed*yParam), barwidth, (int)(maxSpeed*yParam));
//        g.fillRect(barx, top, barwidth, y-top);
//        g.setColor(colorGraphDeliver);
//        barHeight = new Float(smesnap.smppSpeed[SmppSnap.DELIVERED_INDEX] * graphScale * yScale).intValue();
//        g.fillRect(barx, y - barHeight - spent + shiftV, barwidth, barHeight + spent + shiftV );
////  Y
//        g.setColor(graphColor);
////        System.out.println("drawGraph():fm.getHeight()='" + fm.getHeight() + "'\t top='" + top + "'");

        drawHorizontal( g, y, top, barX, barwidth, (size.width - pad) );
//  performance values
//        for (int i = 0; y + shiftV - (i * graphScale) >= top; i += graphHead) {
//    //        for (int i = 0; (i * graphScale) <maxSpeed; i += graphHead) {
//            String s = String.valueOf( new Float(i/yScale).intValue() );
//            g.drawString( s, graphTextWidth - fm.stringWidth(s), y - (int)(i * graphScale) + fm.getDescent() + 5 + shiftV);
//        }
////  time line
//        int lineLeft = pad + graphTextWidth + pad;
//        g.drawLine(lineLeft, y, size.width - pad - pad, y);
//        y--;
//
//
////  horizontal lines
//        for (int i = graphGrid; ; i += graphGrid) {
//            int pos = y - (int)(i * graphScale) + shiftV;
//
//            if( pos <= top ) break;
//
//            g.setColor(graphGridColor);
//            if ((i % graphHiGrid) == 0) {
//                g.setColor(graphHiGridColor);
//            } else {
//                g.setColor(graphGridColor);
//            }
//            g.drawLine(lineLeft, pos, size.width - pad, pos);
//        }

// draw vertical lines
        int right = size.width - pad - pad;
//        int graphStart = 2 * pad + separatorWidth;




//  draw separator bars/graph
        int graphXLeft = barX + barwidth*3;
//        int posX = graphXLeft;
//
//
//        g.setColor(colorBackground);
//        for(int ii = 1; ii<=barwidth; ii++){
//            posX = graphXLeft-ii;
//            g.drawLine(posX, top, posX, y+1);
//        }

        java.util.Set timePoints = drawVertical( g, top, y, barwidth, right, graphXLeft );

//        int start = right - rest;
//        java.util.Set timePoints = new java.util.TreeSet();
////        int toLeft = 0;
//        int toRight = 0;
//        for (int i = 0; ; i += graphGrid) {
//            int posToLeft = start - (int)(i * graphScale);
//            int posToRight = start + (int)(i * graphScale);
//            if (posToLeft < graphXLeft && posToRight > right) {
//                break;
//            }
//            boolean add = false;
//            if( (i % graphHiGrid) == 0 ){
//                if( (++toRight % 2)==0 ){
//                    add = true;
//                }
//            }
//            if(posToLeft > graphXLeft){
//                if ((i % graphHiGrid) == 0){
//                    g.setColor(graphHiGridColor);
//                    if( add ){
//                        timePoints.add( new Integer(posToLeft) );
//                    }
//                }
//                else{
//                    g.setColor(graphGridColor);
//                }
//                g.drawLine(posToLeft, top+1, posToLeft, y-1);
//            }
//
//            if( posToRight < right && posToRight > graphXLeft ){
//                if ((i % graphHiGrid) == 0){
//                    g.setColor(graphHiGridColor);
//
//                    if( add ) {
//                        timePoints.add( new Integer(posToRight) );
//                    }
//                }
//                else{
//                    g.setColor(graphGridColor);
//                }
//                g.drawLine(posToRight, top+1, posToRight, y-1);
//            }
//
//        }

//        int gsz = (snapSmppHistory.countSmmp - 1) * graphScale;
        int gsz = (snapSmppHistory.countSmmp - 1) * xScale;
//  draw graph

        for (int i = 0; i < gsz; i += xScale) {
            int pos = right - i;
            if (pos-xScale < graphXLeft) { break; }

            if (prevsnap != null) {
                smesnap = snapSmppHistory.getPrevSmpp();
                if (smesnap != null && pos>graphXLeft) {
//                    drawGraphLine(g, pos, y, smesnap.smppSpeed[SmppSnap.REJECTED_INDEX], prevsnap.smppSpeed[SmppSnap.REJECTED_INDEX], 0, 0, colorGraphRejected);
//
//                    drawGraphLine(g, pos, y, smesnap.smppSpeed[SmppSnap.GW_REJECTED_INDEX], prevsnap.smppSpeed[SmppSnap.GW_REJECTED_INDEX],
//                                  smesnap.smppSpeed[SmppSnap.REJECTED_INDEX], prevsnap.smppSpeed[SmppSnap.REJECTED_INDEX], colorGraphGwRejected);
//
//                    drawGraphLine(g, pos, y, smesnap.smppSpeed[SmppSnap.ACCEPTED_INDEX], prevsnap.smppSpeed[SmppSnap.ACCEPTED_INDEX],
//                                  smesnap.smppSpeed[SmppSnap.GW_REJECTED_INDEX]+smesnap.smppSpeed[SmppSnap.REJECTED_INDEX],
//                                  prevsnap.smppSpeed[SmppSnap.GW_REJECTED_INDEX]+prevsnap.smppSpeed[SmppSnap.REJECTED_INDEX], colorGraphAccepted);
//
//                    drawGraphLine(g, pos, y, smesnap.smppSpeed[SmppSnap.FAILED_INDEX], prevsnap.smppSpeed[SmppSnap.FAILED_INDEX], 0, 0, colorGraphFailed);
//
//                    drawGraphLine(g, pos, y, smesnap.smppSpeed[SmppSnap.DELIVERED_INDEX], prevsnap.smppSpeed[SmppSnap.DELIVERED_INDEX],
//                                  smesnap.smppSpeed[SmppSnap.FAILED_INDEX], prevsnap.smppSpeed[SmppSnap.FAILED_INDEX], colorGraphDeliver);
                    int add = shiftV;
                    drawGraphLine( g, pos, y, smesnap.smppSpeed[SmppSnap.REJECTED_INDEX], prevsnap.smppSpeed[SmppSnap.REJECTED_INDEX],
                                   0, 0, colorGraphRejected, add );

                    drawGraphLine( g, pos, y, smesnap.smppSpeed[SmppSnap.GW_REJECTED_INDEX], prevsnap.smppSpeed[SmppSnap.GW_REJECTED_INDEX],
                                   0, 0, colorGraphGwRejected, add );

                    int value = smesnap.smppSpeed[SmppSnap.ACCEPTED_INDEX];
                    int valuePrev = prevsnap.smppSpeed[SmppSnap.ACCEPTED_INDEX];

                    drawGraphLine( g, pos, y, value, valuePrev,
                                   0, 0, colorGraphAccepted, add );

                    value += 300;
                    valuePrev += 300;
                    drawGraphLine( g, pos, y, value, valuePrev,
                                   0, 0, colorGraphAccepted, add );

                    drawGraphLine( g, pos, y, smesnap.smppSpeed[SmppSnap.FAILED_INDEX], prevsnap.smppSpeed[SmppSnap.FAILED_INDEX],
                                   0, 0, colorGraphFailed, add );

                    drawGraphLine( g, pos, y, smesnap.smppSpeed[SmppSnap.DELIVERED_INDEX], prevsnap.smppSpeed[SmppSnap.DELIVERED_INDEX],
                                   0, 0, colorGraphDeliver, add );
                }
                prevsnap = smesnap;
            }
        }
        g.setColor(graphColor);
//  draw time
        Calendar cal = Calendar.getInstance();
        SimpleDateFormat sdf = new SimpleDateFormat("HH:mm:ss");
        cal = Calendar.getInstance();
        g.setColor( new Color(0, 224, 0) );
        int ty = size.height - pad - fm.getDescent();
        java.util.Iterator iter = timePoints.iterator();
        while( iter.hasNext() ){
            cal = Calendar.getInstance();
            int val = (int)( (Integer)(iter.next()) ).intValue();
            int app = (right-val)/xScale;
            cal.add( Calendar.SECOND, -app );
            String str = sdf.format( cal.getTime() );
            int sw = fm.stringWidth( String.valueOf(str) ) / 2;
            g.drawString( str, val - sw, ty);
            g.drawLine( val, top, val, y+4 );
        }

        g.setColor(colorBackground);
        g.drawLine(0, top - 1, 0, size.height);
        g.drawLine(separatorWidth + 1, top - 1, separatorWidth + 1, size.height);
        g.setFont(getFont());
        //System.out.println("drawGraph():end");
    }

    void drawGraph(Graphics g, Dimension size) {
        //System.out.println("drawGraph():start");

        SmppSnap smesnap = snapSmppHistory.getSmppLast();
        if (smesnap == null) return;
        if ( !viewGraph ) return;

        SmppSnap prevsnap = smesnap;
//        int spent = 0;
        g.setColor(graphHiGridColor);
        FontMetrics fm = getFontMetrics(graphFont);
        int fh = fm.getHeight();
        int height = 0;
        int top = size.height - height;
        top = top_sep + 10;
        int y = size.height - pad - fh - pad;
        int barwidth = (graphTextWidth - 3 * barSeparator - 5 * pad) / 2;
        g.setFont(graphFont);

// draw bars
        int barX = pad + graphTextWidth + barSeparator + pad + barwidth-2;
        barX = drawBars( g, top, y, barwidth, smesnap );

// draw horizontal lines
        drawHorizontal( g, top, y, barX, barwidth, (size.width - pad) );

        int right = size.width - pad - pad;
        int graphXLeft = barX + barwidth*3;
// draw vertical lines
        java.util.Set timePoints = new java.util.TreeSet();
        timePoints = drawVertical( g, top, y, barwidth, right, graphXLeft );

        int gsz = (snapSmppHistory.countSmmp - 1) * xScale;
// draw graph
        for (int i = 0; i < gsz; i += xScale) {
            int pos = right - i;
            if (pos-xScale < graphXLeft) { break; }

            if (prevsnap != null) {
                smesnap = snapSmppHistory.getPrevSmpp();
                if (smesnap != null && pos>graphXLeft) {
                    int add = shiftV;
                    drawGraphLine( g, pos, y, smesnap.smppSpeed[SmppSnap.REJECTED_INDEX], prevsnap.smppSpeed[SmppSnap.REJECTED_INDEX],
                                   0, 0, colorGraphRejected, add );

                    drawGraphLine( g, pos, y, smesnap.smppSpeed[SmppSnap.GW_REJECTED_INDEX], prevsnap.smppSpeed[SmppSnap.GW_REJECTED_INDEX],
                                   0, 0, colorGraphGwRejected, add );

                    int value = smesnap.smppSpeed[SmppSnap.ACCEPTED_INDEX];
                    int valuePrev = prevsnap.smppSpeed[SmppSnap.ACCEPTED_INDEX];

                    drawGraphLine( g, pos, y, value, valuePrev,
                                   0, 0, colorGraphAccepted, add );

                    value += 300;
                    valuePrev += 300;
                    drawGraphLine( g, pos, y, value, valuePrev,
                                   0, 0, colorGraphAccepted, add );

                    drawGraphLine( g, pos, y, smesnap.smppSpeed[SmppSnap.FAILED_INDEX], prevsnap.smppSpeed[SmppSnap.FAILED_INDEX],
                                   0, 0, colorGraphFailed, add );

                    drawGraphLine( g, pos, y, smesnap.smppSpeed[SmppSnap.DELIVERED_INDEX], prevsnap.smppSpeed[SmppSnap.DELIVERED_INDEX],
                                   0, 0, colorGraphDeliver, add );
                }
                prevsnap = smesnap;
            }
        }
        if( null != timePoints ){
            drawTime( g, timePoints, size, top, y, right );
        }
        //System.out.println("drawGraph():end");
    }

    void drawTime( Graphics g, java.util.Set timePoints, Dimension size, int top, int y, int right){
        FontMetrics fm = getFontMetrics( graphFont );

        g.setColor(graphColor);
        Calendar cal = Calendar.getInstance();
        SimpleDateFormat sdf = new SimpleDateFormat("HH:mm:ss");
        cal = Calendar.getInstance();
        g.setColor( new Color(0, 224, 0) );
        int ty = size.height - pad - fm.getDescent();
        java.util.Iterator iter = timePoints.iterator();
//  draw time
        while( iter.hasNext() ){
            cal = Calendar.getInstance();
            int val = (int)( (Integer)(iter.next()) ).intValue();
            if( val < 0 ){
                break;
            }
            int app = (right-val)/xScale;
            cal.add( Calendar.SECOND, -app );
            String str = sdf.format( cal.getTime() );
            int sw = fm.stringWidth( String.valueOf(str) ) / 2;
            g.drawString( str, val - sw, ty);
            g.drawLine( val, top, val, y+4 );
        }

        g.setColor(colorBackground);
        g.drawLine(0, top - 1, 0, size.height);
        g.drawLine(separatorWidth + 1, top - 1, separatorWidth + 1, size.height);
        g.setFont(getFont());
    }

    private java.util.Set drawVertical( Graphics g, int top, int y, int barwidth, int right, int graphXLeft ) {
        int posX = graphXLeft;
//  draw separator bars/graph
        g.setColor(colorBackground);
        for(int ii = 1; ii<=barwidth; ii++){
            posX = graphXLeft-ii;
            g.drawLine(posX, top, posX, y+1);
        }

        int start = right - rest;
        java.util.Set timePoints = new java.util.TreeSet();
//        int toLeft = 0;
        int toRight = 0;
        for (int i = 0; ; i += graphGrid) {
            int posToLeft = start - (int)(i * graphScale);
            int posToRight = start + (int)(i * graphScale);
            if (posToLeft < graphXLeft && posToRight > right) {
                break;
            }
            boolean add = false;
            if( (i % graphHiGrid) == 0 ){
                if( (++toRight % 2)==0 ){
                    add = true;
                }
            }
            if(posToLeft > graphXLeft){
                if ((i % graphHiGrid) == 0){
                    g.setColor(graphHiGridColor);
                    if( add ){
                        timePoints.add( new Integer(posToLeft) );
                    }
                }
                else{
                    g.setColor(graphGridColor);
                }
                g.drawLine(posToLeft, top+1, posToLeft, y-1);
            }

            if( posToRight < right && posToRight > graphXLeft ){
                if ((i % graphHiGrid) == 0){
                    g.setColor(graphHiGridColor);

                    if( add ) {
                        timePoints.add( new Integer(posToRight) );
                    }
                }
                else{
                    g.setColor(graphGridColor);
                }
                g.drawLine(posToRight, top+1, posToRight, y-1);
            }

        }
        return timePoints;

    }

    protected void drawGraphLine(Graphics g, int x, int y, int snapVal, int prevSnapVal, int underGraphVal, int underGraphPrevVal, Color color , int add) {
        if (snapVal == 0 && prevSnapVal == 0) return;
        g.setColor(color);
        int x1 = new Float( (x - xScale) ).intValue();
        int x2 = new Float(x).intValue();
        int y1 = (y - (int)( (snapVal + underGraphVal) * yScale ) + add );
        int y2 = (y - (int)( (prevSnapVal + underGraphPrevVal) * yScale ) + add );
//        System.out.println("x1='" + x1 + "'\tx2='" + x2+ "'\ty1='" + y1+ "'\ty2='" + y2 + "'\n-------------");
        g.drawLine(x1, y1, x2, y2);
//        g.drawLine(x - graphScale, y - (snapVal + underGraphVal) * graphScale, x, y - (prevSnapVal + underGraphPrevVal) * graphScale);
    }


    public void update(Graphics gg) {
        paint(gg);
    }

    Dimension prefsz = new Dimension(80, 200);

    public Dimension getPreferredSize() {
        return prefsz;
    }

    Dimension minsz = new Dimension(0, 0);

    public Dimension getMinimumSize() {
        return minsz;
    }

    public void mouseClicked(MouseEvent e) {
        //System.out.println("Mouse clicked: " + e.getX() + "/" + e.getY());
        Dimension size = getSize();
        int y = e.getY();
        int x = e.getX();
        if (y < headerHeight) {
            //System.out.println("Mouse clicked:(y < headerHeight)");
            if (x > smppListStart && x < smppGraphStart) {
                //System.out.println("Mouse clicked:(x > smppListStart && x < smppGraphStart)");
                x -= smppListStart;
                int sortid = -1;
                if (x < smeNameWidth) {
                    sortid = 0;
                } else {
                    sortid = ((x - smeNameWidth) / (counterWidth + pad)) + 1;
                }
                if (smeSortId == sortid) {
                    if (smeSortState == SORT_ASC) {
                        smeSortId = -1;
                        smppComparator = null;
                    } else {
                        smeSortState = SORT_ASC;
                        setSmppComparator(sortid, true);
                    }
                } else {
                    smeSortId = sortid;
                    smeSortState = SORT_DESC;
                    setSmppComparator(sortid, false);
                }
            }
//        } else if (y < size.height - split) {
        } else if (y < size.height) {
            //System.out.println("Mouse clicked:!(y < headerHeight)");
            // lists area
            if (x > smppListStart && x < smppListStart + smeNameWidth) {
                //System.out.println("Mouse clicked:(x > smppListStart && x < smppListStart + smeNameWidth)");
                // click on sme name
                int idx = (y - headerHeight) / rowHeight;
                //System.out.println("idx =" + idx);
                //System.out.println("snap.smppCount =" + snap.smppCount);
                //System.out.println("snap.smppSnaps.length =" + snap.smppSnaps.length);
//                viewGraph = true;
                if (idx < snap.smppCount) {
                    //System.out.println( "(idx < snap.smppCount) =" + snap.smppCount );
                    snapSmppHistory.setCurrentSmpp(snap.smppSnaps[idx].smppId);
                    if (split == -1) split = (int)(maxSpeed * graphScale) + rowHeight + separatorWidth + 2 * pad;
                    split = 1;
                    invalidate();
                    repaint();
                }
            }
        } else {
            // graphs area
        }
    }

    void setSmppComparator(int sortid, boolean asc) {
        if (sortid == 0)
            smppComparator = new SmppIdComparator(asc);
        else if (((sortid - 1) % 2) == 0)
            smppComparator = new SmppSpeedComparator(asc, (sortid - 1) / 2);
        else
            smppComparator = new SmppAverageSpeedComparator(asc, (sortid - 1) / 2);
    }

    public void mousePressed(MouseEvent e) {
    }

    public void mouseReleased(MouseEvent e) {
    }

    public void mouseEntered(MouseEvent e) {
    }

    public void mouseExited(MouseEvent e) {
    }

    public void mouseDragged(MouseEvent e) {
    }

    public void mouseMoved(MouseEvent e) {
        mouseX = e.getX();
        mouseY = e.getY();
    }

    public void keyTyped(KeyEvent e) {
//    System.out.println("Key typed: "+e.getKeyChar());
    }

    public void keyPressed(KeyEvent e) {
//    System.out.println("Key pressed: "+e.getKeyCode());
        switch (e.getKeyCode()) {
            case KeyEvent.VK_UP:
                shiftUp();
                break;
            case KeyEvent.VK_DOWN:
                shiftDown();
                break;
            default:
                break;
        }
    }

    public void keyReleased(KeyEvent e) {
    }

    public void shiftUp() {
        Dimension sz = getSize();
        if (sz.height - split < headerHeight + rowHeight * 6) return;
        split += shiftStep;
        //System.out.println("Split = " + split);
        invalidate();
        repaint();
    }

    public void shiftDown() {
        if (split <= 0) return;
        split -= shiftStep;
        if (split < 0) split = 0;
        //System.out.println("Split = " + split);
        invalidate();
        repaint();
    }
}
