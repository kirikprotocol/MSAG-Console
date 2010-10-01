/*
 * Copyright (c) 2006 SibInco Inc. All Rights Reserved.
 */

package ru.sibinco.scag.scmon.applet;

import ru.sibinco.scag.scmon.snap.HttpSnap;
import ru.sibinco.scag.scmon.ScSnap;
import ru.sibinco.scag.util.RemoteResourceBundle;

import java.awt.*;
import java.awt.event.MouseListener;
import java.awt.event.MouseMotionListener;
import java.awt.event.KeyListener;
import java.awt.event.MouseEvent;
import java.awt.event.KeyEvent;
import java.util.ResourceBundle;
import java.util.Comparator;
import java.util.HashSet;
import java.util.Calendar;
import java.text.SimpleDateFormat;

/**
 * The <code>HttpTopGraph</code> class represents
 * <p><p/>
 * Date: 11.01.2006
 * Time: 15:15:14
 *
 * @author &lt;a href="mailto:igor@sibinco.ru"&gt;Igor Klimenko&lt;/a&gt;
 */
public class HttpTopGraph extends Canvas implements MouseListener, MouseMotionListener, KeyListener {
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
            graphHiGridColor = new Color(colorBackground.getRed(), colorBackground.getGreen() + 128, colorBackground.getBlue());
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
            new Color(128, 128, 224), // request
            new Color(224, 224, 224), // requestRejected
            new Color(128, 224, 128), // response
            new Color(128, 224, 224), // responseRejected
            new Color(224, 184, 128), // delivered
            new Color(224, 128, 128) // Failed
    };

    private static final Color colorGraphRequested = Color.blue;
    private static final Color colorGraphRequestRejected = Color.white;
    private static final Color colorGraphResponsed = Color.green;
    private static final Color colorGraphResponseRejected = Color.cyan;
    private static final Color colorGraphDelivered = Color.orange;
    private static final Color colorGraphFailed = Color.red;





    Image offscreen;
    int bottomSpace = pad;
    int topSpace = pad;
    int counterWidth = 0;
    int smeNameWidth = 0;
    int maxSpeed = 50;
    int rowHeight = HttpSnap.COUNTERS * 2;
    ResourceBundle localeText;
    int graphScale;
    int graphGrid;
    int graphHiGrid;
    int graphHead;
    int headerHeight;
    int httpListStart;
    int httpGraphStart;
    int separatorWidth = 0;

    int mouseX;
    int mouseY;

    private static final int SORT_DESC = 1;
    private static final int SORT_ASC = 2;
    int smeSortId = -1;
    int smeSortState;
    Comparator httpComparator = null;

    int split = -1;
    int shiftStep = 10;

    ScSnap snap;
    SnapHttpHistory snapHttpHistory;

    Font graphFont;
    int graphTextWidth;
    int barSeparator = 4;

    HashSet httpViewList;
    double yScale = 1;
    int xScale = 10;
    int rest = 0;
    public static final int DELIMITER = 2;
    boolean viewGraph = false;


    public HttpTopGraph(ScSnap snap, int maxSpeed, int graphScale,
                        int graphGrid, int graphHiGrid,
                        int graphHead, ResourceBundle localeText,
                        SnapHttpHistory snapHttpHistory) {
        super();
        this.maxSpeed = maxSpeed;
        this.localeText = localeText;
        this.graphScale = graphScale;
        this.graphGrid = graphGrid;
        this.graphHiGrid = graphHiGrid;
        this.graphHead = graphHead;
        this.snapHttpHistory = snapHttpHistory;
        addMouseListener(this);
        addMouseMotionListener(this);
        addKeyListener(this);
        graphFont = new Font("dialog", Font.PLAIN, 10);
        setSnap(snap);

    }

    public HttpTopGraph(ScSnap snap, int maxSpeed, int graphScale,
                        int graphGrid, int graphHiGrid,
                        int graphHead, ResourceBundle localeText,
                        SnapHttpHistory snapHttpHistory, HashSet viewList) {
        super();
        this.maxSpeed = maxSpeed;
        this.localeText = localeText;
        this.graphScale = graphScale;
        this.graphGrid = graphGrid;
        this.graphHiGrid = graphHiGrid;
        this.graphHead = graphHead;
        this.snapHttpHistory = snapHttpHistory;
        this.httpViewList = viewList;
        addMouseListener(this);
        addMouseMotionListener(this);
        addKeyListener(this);
        graphFont = new Font("dialog", Font.PLAIN, 10);
        setSnap(snap);

    }

    public void setSnap(ScSnap snap) {
        snapHttpHistory.addSnap(snap);
        this.snap = new ScSnap(snap);
        if (httpComparator != null)
            this.snap.sortHttpSnaps(httpComparator);

        repaint();
    }

    public void setSnap(ScSnap snap, HashSet viewList, int scale) {
        snapHttpHistory.addSnap(snap);
        this.snap = new ScSnap(snap);
        this.httpViewList = viewList;
        this.graphScale = scale;
        if (httpComparator != null)
            this.snap.sortHttpSnaps(httpComparator);

        repaint();
    }

    public void setSnap(ScSnap snap, HashSet viewList, int scale, int maxSpeed, float xScale, double yScale, boolean viewGraph) {
        //System.out.println( "HttpTopGraph:setSnap() '" + snap + "'\nviewList='" + viewList + "'\t\nscale=" + scale + "\t\nmaxSpeed='" + maxSpeed + "'");
        snapHttpHistory.addSnap(snap);
        this.snap = new ScSnap(snap);
        this.httpViewList = viewList;
        this.graphScale = scale;
        this.maxSpeed = maxSpeed;
        this.yScale = yScale;
        this.xScale = (int)xScale;
        if (httpComparator != null)
            this.snap.sortSmppSnaps(httpComparator);
        rest += xScale;
        this.viewGraph = viewGraph;
        repaint();
    }

    public void invalidate() {
        Font font = getFont();
        if (font != null) {

            FontMetrics fm = getFontMetrics(font);
            FontMetrics fmg = getFontMetrics(graphFont);
            Dimension sz = getSize();
            if (sz.width > 0 && sz.height > 0) {
                if (fm.getHeight() > HttpSnap.COUNTERS * 2) {
                    rowHeight = fm.getHeight() + 1;
                } else {
                    rowHeight = HttpSnap.COUNTERS * 2;
                }
                bottomSpace = 2 * pad + fm.getDescent() + fm.getHeight();
                topSpace = pad + fm.getAscent();
                headerHeight = fm.getHeight() + pad + 3;
                counterWidth = fm.stringWidth("0000");
                smeNameWidth = fm.stringWidth("WWWWWWWWWWWWWWWW");
                httpListStart = pad + separatorWidth + pad; //igor
                httpGraphStart = httpListStart + pad + smeNameWidth + (counterWidth + pad) *
                        (HttpSnap.COUNTERS * 2) + pad + pad;
                prefsz = new Dimension(httpGraphStart + 1 + maxSpeed * graphScale + pad,
                        headerHeight + rowHeight * 6 + pad);

                graphTextWidth = fmg.stringWidth("0000");
            }
        }
        offscreen = null;
        super.invalidate();
    }

    public final int indentVert = 10;

    public void paint(Graphics gg) {
        Dimension size = getSize();
        if (!(size.width > 0 && size.height > 0)) return;
        if (offscreen == null) {
            offscreen = createImage(size.width, size.height);
        }
        Graphics g = offscreen.getGraphics();
        Font font = getFont();
        FontMetrics fm = getFontMetrics(font);
        int fh = fm.getHeight();
        g.setColor(colorBackground);
        g.fillRect(0, 0, size.width, size.height);

        g.setClip(0, 0, size.width, size.height - split);
        drawSortState(g, fm);
        // drawing heading
        g.setColor(SystemColor.control);
        g.drawLine(pad, pad + fh + 1, size.width - pad, pad + fh + 1);

        int x = 0;
        g.setColor(headColor);
        int hpos = pad + fm.getAscent();

        x = httpListStart;
        g.setColor(smeColor);
        g.drawString(localeText.getString("snh.httpname"), x + pad, hpos);
        x += smeNameWidth;
        String avgstr = localeText.getString("http.count.avg");
        for (int i = 0; i < HttpSnap.COUNTERS; i++) {
            g.setColor(columnsColor[i]);
            drawCounterHead(g, localeText.getString("http.count." + i), x + pad, hpos, fm);
            x += pad + counterWidth;
            drawCounterHead(g, avgstr, x + pad, hpos, fm);
            x += pad + counterWidth;
        }
        // draw graph scale
        x = httpGraphStart + 1;
        g.setColor(graphColor);
        for (int i = graphHead; ; i += graphHead) {
            int pos = x + i * graphScale;
            if (pos >= size.width - pad) break;
            String s = String.valueOf(i);
            g.drawString(s, pos - (fm.stringWidth(s) / 2), hpos);
        }

        g.setColor(headColor);
        g.drawLine(pad, pad + fh + 1, size.width - pad, pad + fh + 1);

        // draw counters
        int y = headerHeight;
        x = 0;
        //y = pad + fh + 3;
        y = pad + fh + 1;
        x = httpListStart;
        //System.out.println("Sc:HttpTopGraph:snap.httpCount=" + snap.httpCount);
        for (int i = 0; i < snap.httpCount; i++) {
            if ((i % 2) == 0) {
                g.setColor(colorHiBackground);
                g.fillRect(x + pad, y, size.width - x - 2 * pad, rowHeight);
            }
            drawHttpSnap(g, i, x, y, size, fm);
            y += rowHeight;
        }
        if(viewGraph){
//            g.clearRect(0, size.height/DELIMITER, size.width, size.height);
            g.setColor(colorBackground);
            g.fillRect( 0, size.height/DELIMITER, size.width, size.height );
            drawSeparator(g, size);
            g.setClip(0, size.height/DELIMITER + indentVert, size.width, size.height);
            drawGraph(g, size);
        }
//        g.setClip(0, size.height - split + separatorWidth, size.width, split - separatorWidth);
//        drawGraph(g, size);
        g.setClip(0, 0, size.width, size.height);
//        drawSeparator(g, size);
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

    void drawSeparator_l(Graphics g, Dimension size) {
        g.setColor(SystemColor.control);
        int stepUp = indentVert;
        g.fillRect(pad, 0, separatorWidth, size.height);
        if (split > 0) g.fillRect(0, size.height - split - stepUp, size.width, separatorWidth);
        int sl = (separatorWidth - 2) / 2;
        //int vp = errListWidth + pad + sl;
        int vp = pad + sl;
//        int hp = size.height - split + sl - stepUp;
        int hp = (int)size.height/DELIMITER;
        top_sep = hp;
//        graphScale = size.height*2/3/maxSpeed;
//        yParam = size.height*2/3/maxSpeed;
        g.setColor(SystemColor.controlHighlight);
//        g.drawLine(vp + 1, pad + 1, vp + 1, size.height - pad);
//        g.drawLine(vp + 1, pad + 1, vp + 1, top_sep);
//        if (split > 0) g.drawLine(pad + 1, hp + 1, size.width - pad - 1, hp + 1);
        g.drawLine(pad + 1, hp + 1, size.width - pad - 1, hp + 1);
        g.setColor(SystemColor.controlShadow);
//        g.drawLine(vp, pad, vp, size.height - pad - 1);
        g.drawLine(vp, pad, vp, size.height - pad );
//        if (split > 0) g.drawLine(pad, hp, size.width - pad, hp);
        g.drawLine(pad, hp, size.width - pad, hp);
    }

    void drawSeparator_(Graphics g, Dimension size) {
        g.setColor(SystemColor.control);
        g.fillRect(pad, 0, separatorWidth, size.height);
        if (split > 0) g.fillRect(0, size.height - split, size.width, separatorWidth);
        int sl = (separatorWidth - 2) / 2;
        int vp = pad + sl;
        int hp = size.height - split + sl;
        g.setColor(SystemColor.controlHighlight);
        g.drawLine(vp + 1, pad + 1, vp + 1, size.height - pad);
        if (split > 0) g.drawLine(pad + 1, hp + 1, size.width - pad - 1, hp + 1);
        g.setColor(SystemColor.controlShadow);
        g.drawLine(vp, pad, vp, size.height - pad - 1);
        if (split > 0) g.drawLine(pad, hp, size.width - pad, hp);
    }

    void drawHttpSnap(Graphics g, int i, int x, int y, Dimension size, FontMetrics fm) {
        int hpos = y + fm.getAscent();
        HttpSnap ss = snap.httpSnaps[i];
        if (snapHttpHistory.getCurrentHttp() != null && snapHttpHistory.getCurrentHttp().equals(ss.httpId))
            g.setColor(smeHiColor);
        else
            g.setColor(smeColor);
        g.drawString(ss.httpId, x + pad, hpos);
        x += pad + smeNameWidth;
        for (int k = 0; k < HttpSnap.COUNTERS; k++) {
            g.setColor(columnsColor[k]);
            drawCounter(g, ss.httpSpeed[k], x + pad, hpos, fm);
            x += pad + counterWidth;
            drawCounter(g, ss.httpAvgSpeed[k], x + pad, hpos, fm);
            x += pad + counterWidth;
        }
        x += pad + pad;
        for (int k = 0; k < HttpSnap.COUNTERS; k++) {
            if (ss.httpSpeed[k] > 0) {
                g.setColor(columnsColor[k]);
                g.fillRect(x + 1, y + 1 + k * 2, ss.httpSpeed[k] * graphScale, 2);
            }
        }
        g.setColor(graphColor);
        g.drawLine(x + pad, y, x + pad, y + rowHeight);
        x++;
        for (int k = graphGrid; ; k += graphGrid) {
            int pos = x + (k * graphScale);
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
            drawSortGradient(g, httpListStart, smeNameWidth, fm, smeSortState == SORT_ASC);
        } else if (smeSortId > 0) {
            drawSortGradient(g, httpListStart + smeNameWidth + pad + (counterWidth + pad) * (smeSortId - 1), counterWidth, fm, smeSortState == SORT_ASC);
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

    void drawGraph(Graphics g, Dimension size) {
//        System.out.println("drawGraph():start");
        g.setColor(graphHiGridColor);
        FontMetrics fm = getFontMetrics(graphFont);
        int fh = fm.getHeight();
        int height = 0;
        //height = split - separatorWidth - pad;
//        System.out.println( "size.height=" + size.height + " size.width=" + size.getWidth() );
        int top = 0;
        //top = size.height - height;
        top = top_sep + 10 ;
//        System.out.println("top="+ top + " | heigth=" + height);
        int y = size.height - pad - fh - pad;
        int barwidth = (graphTextWidth - 3 * barSeparator - 5 * pad) / 2;
        barwidth = 6;
        g.setFont(graphFont);
// draw bars
        int barx = pad + graphTextWidth + barSeparator + pad + barwidth-2;
        barx = pad + graphTextWidth + barSeparator + pad;

        g.setColor(graphBarBGColor);
        HttpSnap smesnap = snapHttpHistory.getHttpLast();

        if (smesnap == null) return;
        if ( !viewGraph ) return;

        int spent = 0;

        g.drawString( "XS=" + xScale, barx-20, y+10);
        g.drawString( "YS=" + yScale, barx+10, y+10);
        int barHeight = 0;

    // last Rejected
//        if( smesnap.httpSpeed[HttpSnap.REQUEST_REJECTED_INDEX] != 0 ){
            g.setColor(graphBarBGColor);
            g.fillRect(barx, top, barwidth, y-top);
            g.setColor(colorGraphRequestRejected);
            barHeight = new Float(smesnap.httpSpeed[HttpSnap.REQUEST_REJECTED_INDEX] * graphScale * yScale).intValue();
            g.fillRect(barx, y - barHeight, barwidth, barHeight);
            barx += barwidth + barSeparator;
    //        spent += barHeight;
//        g.setColor(colorGraphRequestRejected);
//        int barheight = smesnap.httpSpeed[HttpSnap.REQUEST_REJECTED_INDEX] * graphScale;

    // last Gw Rejected
//        }
//        if( smesnap.httpSpeed[HttpSnap.RESPONSE_REJECTED_INDEX] != 0 ){
            g.setColor(graphBarBGColor);
        //        g.fillRect(barx, y - (int)(maxSpeed*yParam), barwidth, (int)(maxSpeed*yParam));
            g.fillRect(barx, top, barwidth, y-top);
            g.setColor(colorGraphResponseRejected);
            barHeight = new Float(smesnap.httpSpeed[HttpSnap.RESPONSE_REJECTED_INDEX] * graphScale * yScale).intValue();
            g.fillRect(barx, y - barHeight - spent, barwidth, barHeight + spent);
            barx += barwidth + barSeparator;
//        spent += barHeight;
//            g.setColor(colorGraphResponseRejected);
//            barheight = smesnap.httpSpeed[HttpSnap.RESPONSE_REJECTED_INDEX] * graphScale;

//        }
    // last Accepted
//        if( smesnap.httpSpeed[HttpSnap.REQUEST_INDEX] != 0 ){
            g.setColor(graphBarBGColor);
            //        g.fillRect(barx, y - (int)(maxSpeed*yParam), barwidth, (int)(maxSpeed*yParam));
            g.fillRect(barx, top, barwidth, y-top);
            g.setColor(colorGraphRequested);
            barHeight = new Float(smesnap.httpSpeed[HttpSnap.REQUEST_INDEX] * graphScale * yScale).intValue();
            g.fillRect(barx, y - barHeight - spent, barwidth, barHeight + spent);
            barx += barwidth + barSeparator;
            spent = 0;
//        g.setColor(colorGraphRequested);
//        barheight = smesnap.httpSpeed[HttpSnap.REQUEST_INDEX] * graphScale;
//        }
//     last Failed
//        if( smesnap.httpSpeed[HttpSnap.FAILED_INDEX] != 0 ){
            g.setColor(colorGraphFailed);
            //        g.fillRect(barx, y - (int)(maxSpeed*yParam), barwidth, (int)(maxSpeed*yParam));
            g.fillRect(barx, top, barwidth, y-top);
            g.setColor(colorGraphFailed);
            barHeight = new Float(smesnap.httpSpeed[HttpSnap.FAILED_INDEX] * graphScale * yScale).intValue();
            g.fillRect(barx, y - barHeight, barwidth, barHeight);
            barx += barwidth + barSeparator;
    //        spent += barHeight;
//        g.setColor(colorGraphFailed);
//        barheight = smesnap.httpSpeed[HttpSnap.FAILED_INDEX] * graphScale;
//        }
// last Deliver
//        if( smesnap.httpSpeed[HttpSnap.DELIVERED_INDEX] != 0 ){
            g.setColor(graphBarBGColor);
    //        g.fillRect(barx, y - (int)(maxSpeed*yParam), barwidth, (int)(maxSpeed*yParam));
            g.fillRect(barx, top, barwidth, y-top);
            g.setColor(colorGraphDelivered);
            barHeight = new Float(smesnap.httpSpeed[HttpSnap.DELIVERED_INDEX] * graphScale * yScale).intValue();
            g.fillRect(barx, y - barHeight - spent, barwidth, barHeight + spent);
            barx += barwidth + barSeparator;
//        g.setColor(colorGraphDelivered);
//        barheight = smesnap.httpSpeed[HttpSnap.DELIVERED_INDEX] * graphScale;
//        }
// last Response        
//        if( smesnap.httpSpeed[HttpSnap.RESPONSE_INDEX] != 0 ){
            g.setColor(colorGraphResponsed);
    //        g.fillRect(barx, y - (int)(maxSpeed*yParam), barwidth, (int)(maxSpeed*yParam));
            g.fillRect(barx, top, barwidth, y-top);
            g.setColor(colorGraphDelivered);
            barHeight = new Float(smesnap.httpSpeed[HttpSnap.RESPONSE_INDEX] * graphScale * yScale).intValue();
            g.fillRect(barx, y - barHeight - spent, barwidth, barHeight + spent);
            barx += barwidth + barSeparator;
//        g.setColor(colorGraphResponsed);
//        barheight = smesnap.httpSpeed[HttpSnap.RESPONSE_INDEX] * graphScale;
//        }

//  Y
        g.setColor(graphColor);
        for (int i = 0; y - (i * graphScale) >= top; i += graphHead) {
    //        for (int i = 0; (i * graphScale) <maxSpeed; i += graphHead) {
            String s = String.valueOf( new Float(i/yScale).intValue() );
            g.drawString(s, graphTextWidth - fm.stringWidth(s), y - (int)(i * graphScale) + fm.getDescent()+5);
        }
//  time line
        int lineLeft = pad + graphTextWidth + pad;
        int graphXLeft = barx + barwidth;

        g.drawLine(lineLeft, y, size.width - pad - pad, y);
        y--;


//  draw separator bars/graph
//        int posX = graphXLeft;
//        g.setColor(colorBackground);
//        for(int ii = 1; ii<=barwidth; ii++){
//            posX = graphXLeft-ii;
//            g.drawLine(posX, top, posX, y+1);
//        }


//  horizontal lines
        for (int i = graphGrid; ; i += graphGrid) {
            int pos = y - (int)(i * graphScale);

            if( pos <= top ) break;

            g.setColor(graphGridColor);
            if ((i % graphHiGrid) == 0) {
                g.setColor(graphHiGridColor);
            } else {
                g.setColor(graphGridColor);
            }
            g.drawLine(lineLeft, pos, size.width - pad, pos);
        }
//  draw separator bars/graph
        g.setColor( colorBackground );
        g.fillRect( barx, top, barwidth, y - top + 2);

// draw vertical lines
        int right = size.width - pad - pad;
//        int graphStart = 2 * pad + separatorWidth;

        HttpSnap prevsnap = smesnap;
//        int lastVert = 0 ;

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

//        int gsz = (snapSmppHistory.countSmmp - 1) * graphScale;
        int gsz = (snapHttpHistory.countHttp - 1) * xScale;
//  draw graph
        for (int i = 0; i < gsz; i += xScale) {
            int pos = right - i;
            if (pos-xScale < graphXLeft) { break; }

            if (prevsnap != null) {
                smesnap = snapHttpHistory.getPrevHttp();
                if (smesnap != null && pos>graphXLeft) {

                    drawGraphLine(g, pos, y, smesnap.httpSpeed[HttpSnap.REQUEST_REJECTED_INDEX], prevsnap.httpSpeed[HttpSnap.REQUEST_REJECTED_INDEX],
                                  0, 0, colorGraphRequestRejected);

                    drawGraphLine(g, pos, y, smesnap.httpSpeed[HttpSnap.REQUEST_INDEX], prevsnap.httpSpeed[HttpSnap.REQUEST_INDEX],
                                  0, 0, colorGraphRequested);

                    drawGraphLine(g, pos, y, smesnap.httpSpeed[HttpSnap.RESPONSE_INDEX], prevsnap.httpSpeed[HttpSnap.RESPONSE_INDEX],
                                  0, 0, colorGraphResponsed);

                    drawGraphLine(g, pos, y, smesnap.httpSpeed[HttpSnap.FAILED_INDEX], prevsnap.httpSpeed[HttpSnap.FAILED_INDEX],
                                  0, 0, colorGraphFailed);

                    drawGraphLine(g, pos, y, smesnap.httpSpeed[HttpSnap.RESPONSE_REJECTED_INDEX], prevsnap.httpSpeed[HttpSnap.RESPONSE_REJECTED_INDEX],
                                  0, 0, colorGraphResponseRejected);

                    drawGraphLine(g, pos, y, smesnap.httpSpeed[HttpSnap.DELIVERED_INDEX], prevsnap.httpSpeed[HttpSnap.DELIVERED_INDEX],
                                  0, 0, colorGraphDelivered);
                }
                prevsnap = smesnap;
            }
        }
        g.setColor(graphColor);
// draw time
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
        //System.out.println("Sc:HttpTopGraph:drawGraph():end");
    }

    void drawGraph_(Graphics g, Dimension size) {
        g.setColor(graphHiGridColor);
        FontMetrics fm = getFontMetrics(graphFont);
        int fh = fm.getHeight();
        int height = split - separatorWidth - pad;
        int top = size.height - height;
        int y = size.height - pad - fh - pad;
        int barwidth = (graphTextWidth - 3 * barSeparator - 2 * pad) / 2;
        g.setFont(graphFont);
        //******************************* draw bars *******************************************//
        int barx = pad + graphTextWidth + barSeparator;
        g.setColor(graphBarBGColor);
        g.fillRect(barx, top - pad, barwidth, height - fh - pad);

        HttpSnap smesnap = snapHttpHistory.getHttpLast();
        if (smesnap == null) return;
        int spent = 0;

        //*************************** last Request Rejected ***********************************//
        g.setColor(colorGraphRequestRejected);
        int barheight = smesnap.httpSpeed[HttpSnap.REQUEST_REJECTED_INDEX] * graphScale;
        g.fillRect(barx, y - spent - barheight, barwidth, barheight);
        spent += barheight;

        // last Response Rejected
        g.setColor(colorGraphResponseRejected);
        barheight = smesnap.httpSpeed[HttpSnap.RESPONSE_REJECTED_INDEX] * graphScale;
        g.fillRect(barx, y - spent - barheight, barwidth, barheight);
        spent += barheight;

        //******************************** last Requested *************************************//
        g.setColor(colorGraphRequested);
        barheight = smesnap.httpSpeed[HttpSnap.REQUEST_INDEX] * graphScale;
        g.fillRect(barx, y - spent - barheight, barwidth, barheight);
        spent += barheight;

        barx += barwidth + barSeparator;
        spent = 0;
        g.setColor(graphBarBGColor);
        g.fillRect(barx, top - pad, barwidth, height - fh - pad);

        //********************************* last Failed ***************************************//
        g.setColor(colorGraphFailed);
        barheight = smesnap.httpSpeed[HttpSnap.FAILED_INDEX] * graphScale;
        g.fillRect(barx, y - spent - barheight, barwidth, barheight);
        spent += barheight;

        //******************************** last Delivered *************************************//
        g.setColor(colorGraphDelivered);
        barheight = smesnap.httpSpeed[HttpSnap.DELIVERED_INDEX] * graphScale;
        g.fillRect(barx, y - spent - barheight, barwidth, barheight);
        spent += barheight;

        //******************************** last Responsed *************************************//
        g.setColor(colorGraphResponsed);
        barheight = smesnap.httpSpeed[HttpSnap.RESPONSE_INDEX] * graphScale;
        g.fillRect(barx, y - spent - barheight, barwidth, barheight);
        spent += barheight;

        g.setColor(graphColor);
        for (int i = 0; y - (i * graphScale) > top; i += graphHead) {
            String s = String.valueOf(i);
            g.drawString(s, graphTextWidth - fm.stringWidth(s), y - (i * graphScale) + fm.getDescent());
        }
        int lineLeft = pad + graphTextWidth + pad;
        g.drawLine(lineLeft, y, size.width - pad - pad, y);
        y--;
        for (int i = graphGrid; ; i += graphGrid) {
            int pos = y - i * graphScale;
            if (pos <= top) break;
            if ((i % graphHiGrid) == 0) {
                g.setColor(graphHiGridColor);
                g.drawLine(lineLeft, pos, size.width - pad, pos);
            } else {
                g.setColor(graphGridColor);
                g.drawLine(lineLeft, pos, size.width - pad, pos);
            }
        }
        //***** draw vertical scale ******//
        int right = size.width - pad - pad;
        int graphStart = 2 * pad + separatorWidth;
        HttpSnap prevsnap = smesnap;
        for (int i = 0; ; i += graphGrid) {
            int pos = right - i * graphScale;
            if (pos < graphStart) break;
            if ((i % graphHiGrid) == 0)
                g.setColor(graphHiGridColor);
            else
                g.setColor(graphGridColor);
            g.drawLine(pos, top, pos, y);
        }
        int gsz = (snapHttpHistory.countHttp - 1) * graphScale;
        for (int i = 0; i < gsz; i += graphScale) {
            int pos = right - i;
            if (pos < graphStart) break;

            if (prevsnap != null) {
                smesnap = snapHttpHistory.getPrevHttp();
                if (smesnap != null) {
                    drawGraphLine(g, pos, y, smesnap.httpSpeed[HttpSnap.REQUEST_REJECTED_INDEX], prevsnap.httpSpeed[HttpSnap.REQUEST_REJECTED_INDEX], 0, 0, colorGraphRequestRejected);
                    drawGraphLine(g, pos, y, smesnap.httpSpeed[HttpSnap.REQUEST_INDEX], prevsnap.httpSpeed[HttpSnap.REQUEST_INDEX],
                            smesnap.httpSpeed[HttpSnap.REQUEST_REJECTED_INDEX], prevsnap.httpSpeed[HttpSnap.REQUEST_REJECTED_INDEX], colorGraphRequested);
                    drawGraphLine(g, pos, y, smesnap.httpSpeed[HttpSnap.RESPONSE_INDEX], prevsnap.httpSpeed[HttpSnap.RESPONSE_INDEX],
                            smesnap.httpSpeed[HttpSnap.REQUEST_INDEX], prevsnap.httpSpeed[HttpSnap.REQUEST_INDEX], colorGraphResponsed);

                    drawGraphLine(g, pos, y, smesnap.httpSpeed[HttpSnap.FAILED_INDEX], prevsnap.httpSpeed[HttpSnap.FAILED_INDEX], 0, 0, colorGraphFailed);
                    drawGraphLine(g, pos, y, smesnap.httpSpeed[HttpSnap.RESPONSE_REJECTED_INDEX], prevsnap.httpSpeed[HttpSnap.RESPONSE_REJECTED_INDEX],
                            smesnap.httpSpeed[HttpSnap.FAILED_INDEX], prevsnap.httpSpeed[HttpSnap.FAILED_INDEX], colorGraphResponseRejected);
                    drawGraphLine(g, pos, y, smesnap.httpSpeed[HttpSnap.DELIVERED_INDEX], prevsnap.httpSpeed[HttpSnap.DELIVERED_INDEX],
                            smesnap.httpSpeed[HttpSnap.RESPONSE_REJECTED_INDEX], prevsnap.httpSpeed[HttpSnap.RESPONSE_REJECTED_INDEX], colorGraphDelivered);
                }
                prevsnap = smesnap;
            }
        }
        g.setColor(graphColor);
        // draw time scale
        int ty = size.height - pad - fm.getDescent();
        for (int i = 30; ; i += 30) {
            int pos = right - i * graphScale;
            String s = String.valueOf(i);
            int sw = fm.stringWidth(s) / 2;
            if (pos - sw < graphStart) break;
            g.drawString(s, pos - sw, ty);
        }
        g.setColor(colorBackground);
        g.drawLine(0, top - 1, 0, size.height);
        g.drawLine(separatorWidth + 1, top - 1, separatorWidth + 1, size.height);
        g.setFont(getFont());
    }

    protected void drawGraphLine(Graphics g, int x, int y, int snapVal, int prevSnapVal, int underGraphVal, int underGraphPrevVal, Color color) {
        if (snapVal == 0 && prevSnapVal == 0) return;
        g.setColor(color);
        g.drawLine(x - graphScale, y - (snapVal + underGraphVal) * graphScale, x, y - (prevSnapVal + underGraphPrevVal) * graphScale);
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

            if (x > httpListStart && x < httpGraphStart) {
                x -= httpListStart;
                int sortid = -1;
                if (x < smeNameWidth) {
                    sortid = 0;
                } else {
                    sortid = ((x - smeNameWidth) / (counterWidth + pad)) + 1;
                }
                if (smeSortId == sortid) {
                    if (smeSortState == SORT_ASC) {
                        smeSortId = -1;
                        httpComparator = null;
                    } else {
                        smeSortState = SORT_ASC;
                        setHttpComparator(sortid, true);
                    }
                } else {
                    smeSortId = sortid;
                    smeSortState = SORT_DESC;
                    setHttpComparator(sortid, false);
                }
            }
        } else if (y < size.height - split) {
            // lists area
            if (x > httpListStart && x < httpListStart + smeNameWidth) {
                // click on sme name
                int idx = (y - headerHeight) / rowHeight;
                if (idx < snap.httpCount) {
                    snapHttpHistory.setCurrentHttp(snap.httpSnaps[idx].httpId);
//                    if (split == -1) split = maxSpeed * graphScale + rowHeight + separatorWidth + 2 * pad;
                    split = 1;
                    invalidate();
                    repaint();
                }
            }
        } else {
            // graphs area
        }
    }

    void setHttpComparator(int sortid, boolean asc) {
        if (sortid == 0)
            httpComparator = new HttpIdComparator(asc);
        else if (((sortid - 1) % 2) == 0)
            httpComparator = new HttpSpeedComparator(asc, (sortid - 1) / 2);
        else
            httpComparator = new HttpAverageSpeedComparator(asc, (sortid - 1) / 2);
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
    }

    public void keyPressed(KeyEvent e) {
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
