package ru.sibinco.scag.perfmon.applet;

import ru.sibinco.scag.perfmon.PerfSnap;

import java.awt.*;
import java.util.*;

public class PerformanceGraph extends Canvas {
  static final int pixInGrid = 5;
  static final int pad = 2;
  private static final int MAX_SNAP_LIST_SIZE = 4096;

  private static final Color colorBackground = Color.black;
  private static final Color colorGrid = new Color(0, 64, 0);
  private static final Color colorGridLight = new Color(0, 128, 0);
  private static final Color colorGridMin = new Color(0, 224, 0);
  //smpp
  private static final Color colorGraphAccepted = Color.blue;
  private static final Color colorGraphRejected = Color.white;
  private static final Color colorGraphDelivered = Color.green;
  private static final Color colorGraphGwRejected = Color.cyan;
  private static final Color colorGraphFailed = Color.red;
  //http
  private static final Color colorGraphRequest = Color.blue;
  private static final Color colorGraphRequestRejected = Color.white;
  private static final Color colorGraphResponse = Color.cyan;
  private static final Color colorGraphResponseRejected = Color.yellow;
  private static final Color colorGraphDeliveredHTTP = Color.green;
  private static final Color colorGraphFailedHTTP = Color.red;

  Image offscreen;
  int numGrids = 1;
  int gridsInBlock = 1;
  int bottomSpace = pad;
  int topSpace = pad;
  int graphWidth = 0;

  int vertNumGrids = 0;
  int vertGridWidth = 5;
  int vertLightGrid = 4;
  int vertMinuteGrid;

  Dimension prefsz = new Dimension(80, 200);

  Vector snaps = new Vector(MAX_SNAP_LIST_SIZE);


  public PerformanceGraph(int vertLightGrid, int vertMinuteGrid, PerfSnap snap) {
    super();
    this.vertLightGrid = vertLightGrid;
    this.vertMinuteGrid = vertMinuteGrid;
    snaps.addElement(new PerfSnap(snap));
  }

  public void addSnap(PerfSnap snap) {
    if(snaps.size() == MAX_SNAP_LIST_SIZE) {
      snaps.removeElementAt(0);
    }
    snaps.addElement(new PerfSnap(snap));
    repaint();
  }

  public void invalidate() {
    Font font = getFont();
    if(font != null) {
      FontMetrics fm = getFontMetrics(font);
      Dimension sz = getSize();
      if(sz.width > 0 && sz.height > 0) {
        bottomSpace = 2*pad + fm.getDescent() + fm.getHeight();
        topSpace = pad + fm.getAscent();
        numGrids = (sz.height - bottomSpace - topSpace)/pixInGrid;
        gridsInBlock = (numGrids + 2)/PerfMon.block;
        graphWidth = sz.width - 2*pad;
        vertNumGrids = (graphWidth + vertGridWidth - 1)/vertGridWidth;
      }
    }
    offscreen = null;
    super.invalidate();
  }


  public void paint(Graphics gg) {
    Dimension size = getSize();
    if(!(size.width > 0 && size.height > 0)) return;
    if(offscreen == null) {
      offscreen = createImage(size.width, size.height);
    }
    Graphics g = offscreen.getGraphics();

    Font font = getFont();
    FontMetrics fm = getFontMetrics(font);

    g.setColor(colorBackground);
    g.fillRect(0, 0, size.width, size.height);

    int gmax = (PerfMon.block)*gridsInBlock;

    PerfSnap lastSnap = (PerfSnap) snaps.elementAt(snaps.size() - 1);
    int gridShift = ((lastSnap.sctime%vertGridWidth)*PerfMon.pixPerSecond)%vertGridWidth;

    int posx = size.width - pad - gridShift;
    int posy1 = size.height - bottomSpace - gmax*pixInGrid;
    int posy2 = size.height - bottomSpace;

    // draw dark grid
    // vertical grids
    g.setColor(colorGrid);
    for(int i = 0; i < vertNumGrids; i++) {
      g.drawLine(posx - i*vertGridWidth, posy1, posx - i*vertGridWidth, posy2);
    }

    // horizontal grids
    for(int i = 0; i <= gmax; i++) {
      int yy = size.height - bottomSpace - i*pixInGrid;
      if((i%gridsInBlock) == 0) {
        g.setColor(colorGridLight);
      } else {
        g.setColor(colorGrid);
      }
      g.drawLine(pad, yy, size.width - pad, yy);
    }


    //draw lighter grid
    gridShift = ((lastSnap.sctime%(vertGridWidth*vertLightGrid))*PerfMon.pixPerSecond)%(vertGridWidth*vertLightGrid);
    posx = size.width - pad - gridShift;
    int cnt = (graphWidth + vertLightGrid*vertGridWidth - 1)/(vertLightGrid*vertGridWidth);
    g.setColor(colorGridLight);
    for(int i = 0; i < cnt; i++) {
      g.drawLine(posx - i*vertGridWidth*vertLightGrid, posy1, posx - i*vertGridWidth*vertLightGrid, posy2);
    }

    int vertMinuteGridPix = vertMinuteGrid*PerfMon.pixPerSecond;
    //draw minute grid
    gridShift = (((lastSnap.sctime)%(vertGridWidth*vertMinuteGridPix))*PerfMon.pixPerSecond)%(vertGridWidth*vertMinuteGridPix);
    posx = size.width - pad - gridShift;
    long sctime = lastSnap.sctime - gridShift/PerfMon.pixPerSecond;
    cnt = (graphWidth + vertMinuteGridPix*vertGridWidth - 1)/(vertMinuteGridPix*vertGridWidth);
    g.setColor(colorGridMin);
    for(int i = 0; i < cnt; i++) {
      int xx = posx - (i*vertGridWidth*vertMinuteGridPix);
      g.drawLine(xx, posy1 - 2, xx, posy2 + 2);
      String strSctime = PerfMon.gridFormat.format(new Date(sctime*1000));
      sctime -= vertGridWidth*vertMinuteGridPix/PerfMon.pixPerSecond;
      g.drawChars(strSctime.toCharArray(), 0, strSctime.length(),
              xx - fm.charsWidth(strSctime.toCharArray(), 0, strSctime.length())/2,
              size.height - pad - fm.getDescent());
    }

    //draw graph
    // prepare arrays for polyline
    if(snaps.size() > 1) {
      int maxheight = gmax*5;
      int viewableGraph = graphWidth/PerfMon.pixPerSecond;
      int idx = snaps.size() - 1;
      int posy = size.height - bottomSpace;
      int cntg = 0;
      posx = size.width - pad;
      for(int i = 0; i < viewableGraph && idx > 0; i++, cntg++) {
        posx -= PerfMon.pixPerSecond;
        PerfSnap prevSnap = (PerfSnap) snaps.elementAt(idx);
        PerfSnap snap = (PerfSnap) snaps.elementAt(--idx);
        if (PerfMon.statMode.equals(PerfMon.smppStatMode)) {
        // smpp
        // draw submit graphs
        if( PerfMon.viewMode == PerfMon.VIEWMODE_SEPARATE || (PerfMon.viewMode == PerfMon.VIEWMODE_IO && PerfMon.viewInputEnabled) ) {
          if(PerfMon.viewMode == PerfMon.VIEWMODE_IO || (PerfMon.viewMode == PerfMon.VIEWMODE_SEPARATE && PerfMon.viewRejectedEnabled))
          drawGraphLine(g, posy, posx,
                  (int) snap.smppSnap.last[PerfSnap.SMPPSnap.IDX_REJECTED],
                  (int) prevSnap.smppSnap.last[PerfSnap.SMPPSnap.IDX_REJECTED],
                  0,
                  0,
                  maxheight, colorGraphRejected);
          if(PerfMon.viewMode == PerfMon.VIEWMODE_IO || (PerfMon.viewMode == PerfMon.VIEWMODE_SEPARATE && PerfMon.viewAcceptedEnabled))
          drawGraphLine(g, posy, posx,
                  (int) snap.smppSnap.last[PerfSnap.SMPPSnap.IDX_ACCEPTED], (int) prevSnap.smppSnap.last[PerfSnap.SMPPSnap.IDX_ACCEPTED],
                  (PerfMon.viewMode==PerfMon.VIEWMODE_IO)?(int) (snap.smppSnap.last[PerfSnap.SMPPSnap.IDX_REJECTED]):0,
                  (PerfMon.viewMode==PerfMon.VIEWMODE_IO)?(int) (prevSnap.smppSnap.last[PerfSnap.SMPPSnap.IDX_REJECTED]):0,
                  maxheight, colorGraphAccepted);
        }
        //draw deliver graphs
        if( PerfMon.viewMode == PerfMon.VIEWMODE_SEPARATE || (PerfMon.viewMode == PerfMon.VIEWMODE_IO && PerfMon.viewOutputEnabled) ) {
          if(PerfMon.viewMode == PerfMon.VIEWMODE_IO || (PerfMon.viewMode == PerfMon.VIEWMODE_SEPARATE && PerfMon.viewGwRejectedEnabled))
          drawGraphLine(g, posy, posx,
                  (int) snap.smppSnap.last[PerfSnap.SMPPSnap.IDX_GW_REJECTED], (int) prevSnap.smppSnap.last[PerfSnap.SMPPSnap.IDX_GW_REJECTED],
                  0, 0,
                  maxheight, colorGraphGwRejected);
          if(PerfMon.viewMode == PerfMon.VIEWMODE_IO || (PerfMon.viewMode == PerfMon.VIEWMODE_SEPARATE && PerfMon.viewDeliveredEnabled))
          drawGraphLine(g, posy, posx,
                  (int) snap.smppSnap.last[PerfSnap.SMPPSnap.IDX_DELIVERED], (int) prevSnap.smppSnap.last[PerfSnap.SMPPSnap.IDX_DELIVERED],
                  (PerfMon.viewMode==PerfMon.VIEWMODE_IO)?(int) snap.smppSnap.last[PerfSnap.SMPPSnap.IDX_GW_REJECTED]:0,
                  (PerfMon.viewMode==PerfMon.VIEWMODE_IO)?(int) prevSnap.smppSnap.last[PerfSnap.SMPPSnap.IDX_GW_REJECTED]:0,
                  maxheight, colorGraphDelivered);
        }
        // draw transaction graphs
        if( PerfMon.viewMode == PerfMon.VIEWMODE_SEPARATE || (PerfMon.viewMode == PerfMon.VIEWMODE_IO && PerfMon.viewTransactionEnabled) ) {
          if(PerfMon.viewMode == PerfMon.VIEWMODE_IO || (PerfMon.viewMode == PerfMon.VIEWMODE_SEPARATE && PerfMon.viewFailedEnabled))
          drawGraphLine(g, posy, posx,
                  (int) snap.smppSnap.last[PerfSnap.SMPPSnap.IDX_FAILED], (int) prevSnap.smppSnap.last[PerfSnap.SMPPSnap.IDX_FAILED],
                  0,
                  0,
                  maxheight, colorGraphFailed);
        }
       } else {
        // http
        // draw request graphs
          if( PerfMon.viewMode == PerfMon.VIEWMODE_SEPARATE || (PerfMon.viewMode == PerfMon.VIEWMODE_IO && PerfMon.viewInputEnabled) ) {
            if(PerfMon.viewMode == PerfMon.VIEWMODE_IO || (PerfMon.viewMode == PerfMon.VIEWMODE_SEPARATE && PerfMon.viewRequestRejectedEnabled))
            drawGraphLine(g, posy, posx,
                    (int) snap.httpSnap.last[PerfSnap.HTTPSnap.IDX_REQUEST_REJECTED],
                    (int) prevSnap.httpSnap.last[PerfSnap.HTTPSnap.IDX_REQUEST_REJECTED],
                    0,
                    0,
                    maxheight, colorGraphRequestRejected);
            if(PerfMon.viewMode == PerfMon.VIEWMODE_IO || (PerfMon.viewMode == PerfMon.VIEWMODE_SEPARATE && PerfMon.viewRequestEnabled))
            drawGraphLine(g, posy, posx,
                    (int) snap.httpSnap.last[PerfSnap.HTTPSnap.IDX_REQUEST], (int) prevSnap.httpSnap.last[PerfSnap.HTTPSnap.IDX_REQUEST],
                    (PerfMon.viewMode==PerfMon.VIEWMODE_IO)?(int) (snap.httpSnap.last[PerfSnap.HTTPSnap.IDX_REQUEST_REJECTED]):0,
                    (PerfMon.viewMode==PerfMon.VIEWMODE_IO)?(int) (prevSnap.httpSnap.last[PerfSnap.HTTPSnap.IDX_REQUEST_REJECTED]):0,
                    maxheight, colorGraphRequest);
          }
        // draw response graphs
          if( PerfMon.viewMode == PerfMon.VIEWMODE_SEPARATE || (PerfMon.viewMode == PerfMon.VIEWMODE_IO && PerfMon.viewOutputEnabled) ) {
            if(PerfMon.viewMode == PerfMon.VIEWMODE_IO || (PerfMon.viewMode == PerfMon.VIEWMODE_SEPARATE && PerfMon.viewResponseRejectedEnabled))
            drawGraphLine(g, posy, posx,
                    (int) snap.httpSnap.last[PerfSnap.HTTPSnap.IDX_RESPONSE_REJECTED], (int) prevSnap.httpSnap.last[PerfSnap.HTTPSnap.IDX_RESPONSE_REJECTED],
                    0, 0,
                    maxheight, colorGraphResponseRejected);
            if(PerfMon.viewMode == PerfMon.VIEWMODE_IO || (PerfMon.viewMode == PerfMon.VIEWMODE_SEPARATE && PerfMon.viewResponseEnabled))
            drawGraphLine(g, posy, posx,
                    (int) snap.httpSnap.last[PerfSnap.HTTPSnap.IDX_RESPONSE], (int) prevSnap.httpSnap.last[PerfSnap.HTTPSnap.IDX_RESPONSE],
                    (PerfMon.viewMode==PerfMon.VIEWMODE_IO)?(int) snap.httpSnap.last[PerfSnap.HTTPSnap.IDX_RESPONSE_REJECTED]:0,
                    (PerfMon.viewMode==PerfMon.VIEWMODE_IO)?(int) prevSnap.httpSnap.last[PerfSnap.HTTPSnap.IDX_RESPONSE_REJECTED]:0,
                    maxheight, colorGraphResponse);
          }
        // draw transaction graphs
          if( PerfMon.viewMode == PerfMon.VIEWMODE_SEPARATE || (PerfMon.viewMode == PerfMon.VIEWMODE_IO && PerfMon.viewTransactionEnabled) ) {
            if(PerfMon.viewMode == PerfMon.VIEWMODE_IO || (PerfMon.viewMode == PerfMon.VIEWMODE_SEPARATE && PerfMon.viewFailedHTTPEnabled))
            drawGraphLine(g, posy, posx,
                    (int) snap.httpSnap.last[PerfSnap.HTTPSnap.IDX_FAILED], (int) prevSnap.httpSnap.last[PerfSnap.HTTPSnap.IDX_FAILED],
                    0,
                    0,
                    maxheight, colorGraphFailedHTTP);
          }
          if( PerfMon.viewMode == PerfMon.VIEWMODE_SEPARATE || (PerfMon.viewMode == PerfMon.VIEWMODE_IO && PerfMon.viewTransactionEnabled) ) {
            if(PerfMon.viewMode == PerfMon.VIEWMODE_IO || (PerfMon.viewMode == PerfMon.VIEWMODE_SEPARATE && PerfMon.viewDeliveredHTTPEnabled))
            drawGraphLine(g, posy, posx,
                    (int) snap.httpSnap.last[PerfSnap.HTTPSnap.IDX_DELIVERED], (int) prevSnap.httpSnap.last[PerfSnap.HTTPSnap.IDX_DELIVERED],
                    (PerfMon.viewMode==PerfMon.VIEWMODE_IO)?(int) snap.httpSnap.last[PerfSnap.HTTPSnap.IDX_FAILED]:0,
                    (PerfMon.viewMode==PerfMon.VIEWMODE_IO)?(int) prevSnap.httpSnap.last[PerfSnap.HTTPSnap.IDX_FAILED]:0,
                    maxheight, colorGraphDeliveredHTTP);
          }
        }
      }
    }

    gg.drawImage(offscreen, 0, 0, null);
    g.dispose();
  }

  protected void drawGraphLine(Graphics g, int y, int x, int snapVal, int prevSnapVal, int underGraphVal, int underGraphPrevVal, int maxheight, Color color) {
    if(snapVal == 0 && prevSnapVal == 0) return;
    g.setColor(color);
    g.drawLine(x, y - (maxheight*(snapVal + underGraphVal))/PerfMon.scale, x + PerfMon.pixPerSecond, y - (maxheight*(prevSnapVal + underGraphPrevVal))/PerfMon.scale);
  }


  public Dimension getPreferredSize() {
    return prefsz;
  }

  public void update(Graphics gg) {
    paint(gg);
  }
}
