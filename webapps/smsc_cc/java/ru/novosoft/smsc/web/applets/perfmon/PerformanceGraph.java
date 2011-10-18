package ru.novosoft.smsc.web.applets.perfmon;

import ru.novosoft.smsc.admin.perfmon.PerfSnap;

import java.awt.*;
import java.util.Date;
import java.util.Vector;

/**
 * author: Aleksandr Khalitov
 */
class PerformanceGraph extends Canvas {
  private static final int pixInGrid = 5;
  private static final int pad = 2;
  private static final int MAX_SNAP_LIST_SIZE = 4096;

  private static final Color colorBackground = Color.black;
  private static final Color colorGrid = new Color(0, 64, 0);
  private static final Color colorGridLight = new Color(0, 128, 0);
  private static final Color colorGridMin = new Color(0, 224, 0);
  private static final Color colorGraphDeliver = Color.green;
  private static final Color colorGraphDeliverErr = Color.red;
  private static final Color colorGraphTempErr = Color.orange;
  private static final Color colorGraphRetry = Color.cyan;
  private static final Color colorGraphSubmit = Color.blue;
  private static final Color colorGraphSubmitErr = Color.white;

  private Image offscreen;
  private int numGrids = 1;
  private int gridsInBlock = 1;
  private int bottomSpace = pad;
  private int topSpace = pad;
  private int graphWidth = 0;

  private int vertNumGrids = 0;
  private final int vertGridWidth = 5;
  private int vertLightGrid = 4;
  private final int vertMinuteGrid;

  private final Dimension prefsz = new Dimension(80, 200);

  private final Vector snaps = new Vector(MAX_SNAP_LIST_SIZE);

  private final PerfMon perfMon;


  public PerformanceGraph(int vertLightGrid, int vertMinuteGrid, PerfSnap snap, PerfMon perfMon) {
    super();
    this.vertLightGrid = vertLightGrid;
    this.vertMinuteGrid = vertMinuteGrid;
    this.perfMon = perfMon;
    snaps.addElement(new PerfSnap(snap));
  }

  public void addSnap(PerfSnap snap) {
    if (snaps.size() == MAX_SNAP_LIST_SIZE) {
      snaps.removeElementAt(0);
    }
    snaps.addElement(new PerfSnap(snap));
    repaint();
  }

  public void invalidate() {
    Font font = getFont();
    if (font != null) {
      FontMetrics fm = getFontMetrics(font);
      Dimension sz = getSize();
      if (sz.width > 0 && sz.height > 0) {
        bottomSpace = 2 * pad + fm.getDescent() + fm.getHeight();
        topSpace = pad + fm.getAscent();
        numGrids = (sz.height - bottomSpace - topSpace) / pixInGrid;
        gridsInBlock = (numGrids + 2) / perfMon.block;
        graphWidth = sz.width - 2 * pad;
        vertNumGrids = (graphWidth + vertGridWidth - 1) / vertGridWidth;
      }
    }
    offscreen = null;
    super.invalidate();
  }


  public void paint(Graphics gg) {
    Dimension size = getSize();
    if (!(size.width > 0 && size.height > 0)) return;
    if (offscreen == null) {
      offscreen = createImage(size.width, size.height);
    }
    Graphics g = offscreen.getGraphics();

    Font font = getFont();
    FontMetrics fm = getFontMetrics(font);

    g.setColor(colorBackground);
    g.fillRect(0, 0, size.width, size.height);

    int gmax = (perfMon.block) * gridsInBlock;

    PerfSnap lastSnap = (PerfSnap) snaps.elementAt(snaps.size() - 1);
    int gridShift = (int) (((lastSnap.sctime % vertGridWidth) * perfMon.pixPerSecond) % vertGridWidth);

    int posx = size.width - pad - gridShift;
    int posy1 = size.height - bottomSpace - gmax * pixInGrid;
    int posy2 = size.height - bottomSpace;

    // draw dark grid
    // vertical grids
    g.setColor(colorGrid);
    for (int i = 0; i < vertNumGrids; i++) {
      g.drawLine(posx - i * vertGridWidth, posy1, posx - i * vertGridWidth, posy2);
    }

    // horizontal grids
    for (int i = 0; i <= gmax; i++) {
      int yy = size.height - bottomSpace - i * pixInGrid;
      if ((i % gridsInBlock) == 0) {
        g.setColor(colorGridLight);
      } else {
        g.setColor(colorGrid);
      }
      g.drawLine(pad, yy, size.width - pad, yy);
    }


    //draw lighter grid
    gridShift = (int) (((lastSnap.sctime % (vertGridWidth * vertLightGrid)) * perfMon.pixPerSecond) % (vertGridWidth * vertLightGrid));
    posx = size.width - pad - gridShift;
    int cnt = (graphWidth + vertLightGrid * vertGridWidth - 1) / (vertLightGrid * vertGridWidth);
    g.setColor(colorGridLight);
    for (int i = 0; i < cnt; i++) {
      g.drawLine(posx - i * vertGridWidth * vertLightGrid, posy1, posx - i * vertGridWidth * vertLightGrid, posy2);
    }

    int vertMinuteGridPix = vertMinuteGrid * perfMon.pixPerSecond;
    //draw minute grid
    gridShift = (int) ((((lastSnap.sctime) % (vertGridWidth * vertMinuteGridPix)) * perfMon.pixPerSecond) % (vertGridWidth * vertMinuteGridPix));
    posx = size.width - pad - gridShift;
    long sctime = lastSnap.sctime - gridShift / perfMon.pixPerSecond;
    cnt = (graphWidth + vertMinuteGridPix * vertGridWidth - 1) / (vertMinuteGridPix * vertGridWidth);
    g.setColor(colorGridMin);
    for (int i = 0; i < cnt; i++) {
      int xx = posx - (i * vertGridWidth * vertMinuteGridPix);
      g.drawLine(xx, posy1 - 2, xx, posy2 + 2);
      String strSctime = perfMon.gridFormat.format(new Date(sctime * 1000));
      sctime -= vertGridWidth * vertMinuteGridPix / perfMon.pixPerSecond;
      g.drawChars(strSctime.toCharArray(), 0, strSctime.length(),
          xx - fm.charsWidth(strSctime.toCharArray(), 0, strSctime.length()) / 2,
          size.height - pad - fm.getDescent());
    }

    //draw graph
    // prepare arrays for polyline
    if (snaps.size() > 1) {
      int msuShift = perfMon.showMsu ? 6 : 0;
      int maxheight = gmax * 5;
      int viewableGraph = graphWidth / perfMon.pixPerSecond;
      int idx = snaps.size() - 1;
      int posy = size.height - bottomSpace;
      int cntg = 0;
      posx = size.width - pad;
      for (int i = 0; i < viewableGraph && idx > 0; i++, cntg++) {
        posx -= perfMon.pixPerSecond;
        PerfSnap prevSnap = (PerfSnap) snaps.elementAt(idx);
        PerfSnap snap = (PerfSnap) snaps.elementAt(--idx);
        // draw submit graphs
        if (perfMon.viewMode == PerfMon.VIEWMODE_SEPARATE || (perfMon.viewMode == PerfMon.VIEWMODE_IO && perfMon.viewInputEnabled)) {
          if (perfMon.viewMode == PerfMon.VIEWMODE_IO || (perfMon.viewMode == PerfMon.VIEWMODE_SEPARATE && perfMon.viewSubmitErrEnabled))
            drawGraphLine(g, posy, posx,
                (int) snap.last[PerfSnap.IDX_SUBMITERR + msuShift],
                (int) prevSnap.last[PerfSnap.IDX_SUBMITERR + msuShift],
                0,
                0,
                maxheight, colorGraphSubmitErr);
          if (perfMon.viewMode == PerfMon.VIEWMODE_IO || (perfMon.viewMode == PerfMon.VIEWMODE_SEPARATE && perfMon.viewRetryEnabled))
            drawGraphLine(g, posy, posx,
                (int) snap.last[PerfSnap.IDX_RETRY + msuShift], (int) prevSnap.last[PerfSnap.IDX_RETRY + msuShift],
                (perfMon.viewMode == PerfMon.VIEWMODE_IO) ? (int) snap.last[PerfSnap.IDX_SUBMITERR + msuShift] : 0,
                (perfMon.viewMode == PerfMon.VIEWMODE_IO) ? (int) prevSnap.last[PerfSnap.IDX_SUBMITERR + msuShift] : 0,
                maxheight, colorGraphRetry);
          if (perfMon.viewMode == PerfMon.VIEWMODE_IO || (perfMon.viewMode == PerfMon.VIEWMODE_SEPARATE && perfMon.viewSubmitEnabled))
            drawGraphLine(g, posy, posx,
                (int) snap.last[PerfSnap.IDX_SUBMIT + msuShift], (int) prevSnap.last[PerfSnap.IDX_SUBMIT + msuShift],
                (perfMon.viewMode == PerfMon.VIEWMODE_IO) ? (int) (snap.last[PerfSnap.IDX_SUBMITERR + msuShift] + snap.last[PerfSnap.IDX_RETRY + msuShift]) : 0,
                (perfMon.viewMode == PerfMon.VIEWMODE_IO) ? (int) (prevSnap.last[PerfSnap.IDX_SUBMITERR + msuShift] + prevSnap.last[PerfSnap.IDX_RETRY + msuShift]) : 0,
                maxheight, colorGraphSubmit);
        }
        //draw deliver graphs
        if (perfMon.viewMode == PerfMon.VIEWMODE_SEPARATE || (perfMon.viewMode == PerfMon.VIEWMODE_IO && perfMon.viewOutputEnabled)) {
          if (perfMon.viewMode == PerfMon.VIEWMODE_IO || (perfMon.viewMode == PerfMon.VIEWMODE_SEPARATE && perfMon.viewDeliverErrEnabled))
            drawGraphLine(g, posy, posx,
                (int) snap.last[PerfSnap.IDX_DELIVERERR + msuShift], (int) prevSnap.last[PerfSnap.IDX_DELIVERERR + msuShift],
                0, 0,
                maxheight, colorGraphDeliverErr);
          if (perfMon.viewMode == PerfMon.VIEWMODE_IO || (perfMon.viewMode == PerfMon.VIEWMODE_SEPARATE && perfMon.viewTempErrEnabled))
            drawGraphLine(g, posy, posx,
                (int) snap.last[PerfSnap.IDX_TEMPERR + msuShift], (int) prevSnap.last[PerfSnap.IDX_TEMPERR + msuShift],
                (perfMon.viewMode == PerfMon.VIEWMODE_IO) ? (int) snap.last[PerfSnap.IDX_DELIVERERR + msuShift] : 0,
                (perfMon.viewMode == PerfMon.VIEWMODE_IO) ? (int) prevSnap.last[PerfSnap.IDX_DELIVERERR + msuShift] : 0,
                maxheight, colorGraphTempErr);
          if (perfMon.viewMode == PerfMon.VIEWMODE_IO || (perfMon.viewMode == PerfMon.VIEWMODE_SEPARATE && perfMon.viewDeliverEnabled))
            drawGraphLine(g, posy, posx,
                (int) snap.last[PerfSnap.IDX_DELIVER + msuShift], (int) prevSnap.last[PerfSnap.IDX_DELIVER + msuShift],
                (perfMon.viewMode == PerfMon.VIEWMODE_IO) ? (int) (snap.last[PerfSnap.IDX_TEMPERR + msuShift] + snap.last[PerfSnap.IDX_DELIVERERR + msuShift]) : 0,
                (perfMon.viewMode == PerfMon.VIEWMODE_IO) ? (int) (prevSnap.last[PerfSnap.IDX_TEMPERR + msuShift] + prevSnap.last[PerfSnap.IDX_DELIVERERR + msuShift]) : 0,
                maxheight, colorGraphDeliver);
        }
      }
    }

    gg.drawImage(offscreen, 0, 0, null);
    g.dispose();
  }

  void drawGraphLine(Graphics g, int y, int x, int snapVal, int prevSnapVal, int underGraphVal, int underGraphPrevVal, int maxheight, Color color) {
    if (snapVal == 0 && prevSnapVal == 0) return;
    g.setColor(color);
    g.drawLine(x, y - (maxheight * (snapVal + underGraphVal)) / perfMon.scale, x + perfMon.pixPerSecond, y - (maxheight * (prevSnapVal + underGraphPrevVal)) / perfMon.scale);
  }


  public Dimension getPreferredSize() {
    return prefsz;
  }

  public void update(Graphics gg) {
    paint(gg);
  }

}
