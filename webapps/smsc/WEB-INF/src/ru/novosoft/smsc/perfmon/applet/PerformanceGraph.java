package ru.novosoft.smsc.perfmon.applet;

import ru.novosoft.smsc.perfmon.PerfSnap;

import java.awt.*;
import java.util.Date;
import java.util.Vector;

public class PerformanceGraph extends Canvas
{
  static final int pixInGrid = 5;
  static final int pad = 2;
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
  private static final Color colorGraphTotal = Color.white;

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


  public PerformanceGraph(int vertLightGrid, int vertMinuteGrid, PerfSnap snap)
  {
    super();
    this.vertLightGrid = vertLightGrid;
    this.vertMinuteGrid = vertMinuteGrid;
    snaps.addElement(new PerfSnap(snap));
  }

  public void addSnap(PerfSnap snap)
  {
    if (snaps.size() == MAX_SNAP_LIST_SIZE) {
      snaps.removeElementAt(0);
    }
    snaps.addElement(new PerfSnap(snap));
    repaint();
  }

  public void invalidate()
  {
    Font font = getFont();
    if (font != null) {
      FontMetrics fm = getFontMetrics(font);
      Dimension sz = getSize();
      if (sz.width > 0 && sz.height > 0) {
        bottomSpace = 2 * pad + fm.getDescent() + fm.getHeight();
        topSpace = pad + fm.getAscent();
        numGrids = (sz.height - bottomSpace - topSpace) / pixInGrid;
        gridsInBlock = (numGrids + 2) / PerfMon.block;
        graphWidth = sz.width - 2 * pad;
        vertNumGrids = (graphWidth + vertGridWidth - 1) / vertGridWidth;
      }
    }
    offscreen = null;
    super.invalidate();
  }


  public void paint(Graphics gg)
  {
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

    int gmax = (PerfMon.block) * gridsInBlock;

    PerfSnap lastSnap = (PerfSnap) snaps.elementAt(snaps.size() - 1);
    int gridShift = (int)(((lastSnap.sctime % vertGridWidth) * PerfMon.pixPerSecond) % vertGridWidth);

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
      }
      else {
        g.setColor(colorGrid);
      }
      g.drawLine(pad, yy, size.width - pad, yy);
    }


    //draw lighter grid
    gridShift = (int)(((lastSnap.sctime % (vertGridWidth * vertLightGrid)) * PerfMon.pixPerSecond) % (vertGridWidth * vertLightGrid));
    posx = size.width - pad - gridShift;
    int cnt = (graphWidth + vertLightGrid * vertGridWidth - 1) / (vertLightGrid * vertGridWidth);
    g.setColor(colorGridLight);
    for (int i = 0; i < cnt; i++) {
      g.drawLine(posx - i * vertGridWidth * vertLightGrid, posy1, posx - i * vertGridWidth * vertLightGrid, posy2);
    }

    int vertMinuteGridPix = vertMinuteGrid * PerfMon.pixPerSecond;
    //draw minute grid
    gridShift = (int)((((lastSnap.sctime) % (vertGridWidth * vertMinuteGridPix)) * PerfMon.pixPerSecond) % (vertGridWidth * vertMinuteGridPix));
    posx = size.width - pad - gridShift;
    long sctime = lastSnap.sctime - gridShift / PerfMon.pixPerSecond;
    cnt = (graphWidth + vertMinuteGridPix * vertGridWidth - 1) / (vertMinuteGridPix * vertGridWidth);
    g.setColor(colorGridMin);
    for (int i = 0; i < cnt; i++) {
      int xx = posx - (i * vertGridWidth * vertMinuteGridPix);
      g.drawLine(xx, posy1 - 2, xx, posy2 + 2);
      String strSctime = PerfMon.gridFormat.format(new Date(sctime * 1000));
      sctime -= vertGridWidth * vertMinuteGridPix / PerfMon.pixPerSecond;
      g.drawChars(strSctime.toCharArray(), 0, strSctime.length(),
              xx - fm.charsWidth(strSctime.toCharArray(), 0, strSctime.length()) / 2,
              size.height - pad - fm.getDescent());
    }

    //draw graph
    // prepare arrays for polyline
    if (snaps.size() > 1) {
      int msuShift = PerfMon.showMsu?6:0;
      int maxheight = gmax * 5;
      int viewableGraph = graphWidth / PerfMon.pixPerSecond;
      int idx = snaps.size() - 1;
      int posy = size.height - bottomSpace;
      int cntg = 0;
      posx = size.width - pad;
      for (int i = 0; i < viewableGraph && idx > 0; i++, cntg++) {
        posx -= PerfMon.pixPerSecond;
        PerfSnap prevSnap = (PerfSnap) snaps.elementAt(idx);
        PerfSnap snap = (PerfSnap) snaps.elementAt(--idx);
        // draw submit graphs
        if (PerfMon.viewMode == PerfMon.VIEWMODE_SEPARATE || (PerfMon.viewMode == PerfMon.VIEWMODE_IO && PerfMon.viewInputEnabled)) {
          if (PerfMon.viewMode == PerfMon.VIEWMODE_IO || (PerfMon.viewMode == PerfMon.VIEWMODE_SEPARATE && PerfMon.viewSubmitErrEnabled))
            drawGraphLine(g, posy, posx,
                    (int) snap.last[PerfSnap.IDX_SUBMITERR+msuShift],
                    (int) prevSnap.last[PerfSnap.IDX_SUBMITERR+msuShift],
                    0,
                    0,
                    maxheight, colorGraphSubmitErr);
          if (PerfMon.viewMode == PerfMon.VIEWMODE_IO || (PerfMon.viewMode == PerfMon.VIEWMODE_SEPARATE && PerfMon.viewRetryEnabled))
            drawGraphLine(g, posy, posx,
                    (int) snap.last[PerfSnap.IDX_RETRY+msuShift], (int) prevSnap.last[PerfSnap.IDX_RETRY+msuShift],
                    (PerfMon.viewMode == PerfMon.VIEWMODE_IO) ? (int) snap.last[PerfSnap.IDX_SUBMITERR+msuShift] : 0,
                    (PerfMon.viewMode == PerfMon.VIEWMODE_IO) ? (int) prevSnap.last[PerfSnap.IDX_SUBMITERR+msuShift] : 0,
                    maxheight, colorGraphRetry);
          if (PerfMon.viewMode == PerfMon.VIEWMODE_IO || (PerfMon.viewMode == PerfMon.VIEWMODE_SEPARATE && PerfMon.viewSubmitEnabled))
            drawGraphLine(g, posy, posx,
                    (int) snap.last[PerfSnap.IDX_SUBMIT+msuShift], (int) prevSnap.last[PerfSnap.IDX_SUBMIT+msuShift],
                    (PerfMon.viewMode == PerfMon.VIEWMODE_IO) ? (int) (snap.last[PerfSnap.IDX_SUBMITERR+msuShift] + snap.last[PerfSnap.IDX_RETRY+msuShift]) : 0,
                    (PerfMon.viewMode == PerfMon.VIEWMODE_IO) ? (int) (prevSnap.last[PerfSnap.IDX_SUBMITERR+msuShift] + prevSnap.last[PerfSnap.IDX_RETRY+msuShift]) : 0,
                    maxheight, colorGraphSubmit);
        }
        //draw deliver graphs
        if (PerfMon.viewMode == PerfMon.VIEWMODE_SEPARATE || (PerfMon.viewMode == PerfMon.VIEWMODE_IO && PerfMon.viewOutputEnabled)) {
          if (PerfMon.viewMode == PerfMon.VIEWMODE_IO || (PerfMon.viewMode == PerfMon.VIEWMODE_SEPARATE && PerfMon.viewDeliverErrEnabled))
            drawGraphLine(g, posy, posx,
                    (int) snap.last[PerfSnap.IDX_DELIVERERR+msuShift], (int) prevSnap.last[PerfSnap.IDX_DELIVERERR+msuShift],
                    0, 0,
                    maxheight, colorGraphDeliverErr);
          if (PerfMon.viewMode == PerfMon.VIEWMODE_IO || (PerfMon.viewMode == PerfMon.VIEWMODE_SEPARATE && PerfMon.viewTempErrEnabled))
            drawGraphLine(g, posy, posx,
                    (int) snap.last[PerfSnap.IDX_TEMPERR+msuShift], (int) prevSnap.last[PerfSnap.IDX_TEMPERR+msuShift],
                    (PerfMon.viewMode == PerfMon.VIEWMODE_IO) ? (int) snap.last[PerfSnap.IDX_DELIVERERR+msuShift] : 0,
                    (PerfMon.viewMode == PerfMon.VIEWMODE_IO) ? (int) prevSnap.last[PerfSnap.IDX_DELIVERERR+msuShift] : 0,
                    maxheight, colorGraphTempErr);
          if (PerfMon.viewMode == PerfMon.VIEWMODE_IO || (PerfMon.viewMode == PerfMon.VIEWMODE_SEPARATE && PerfMon.viewDeliverEnabled))
            drawGraphLine(g, posy, posx,
                    (int) snap.last[PerfSnap.IDX_DELIVER+msuShift], (int) prevSnap.last[PerfSnap.IDX_DELIVER+msuShift],
                    (PerfMon.viewMode == PerfMon.VIEWMODE_IO) ? (int) (snap.last[PerfSnap.IDX_TEMPERR+msuShift] + snap.last[PerfSnap.IDX_DELIVERERR+msuShift]) : 0,
                    (PerfMon.viewMode == PerfMon.VIEWMODE_IO) ? (int) (prevSnap.last[PerfSnap.IDX_TEMPERR+msuShift] + prevSnap.last[PerfSnap.IDX_DELIVERERR+msuShift]) : 0,
                    maxheight, colorGraphDeliver);
        }
      }
    }

    gg.drawImage(offscreen, 0, 0, null);
    g.dispose();
  }

  protected void drawGraphLine(Graphics g, int y, int x, int snapVal, int prevSnapVal, int underGraphVal, int underGraphPrevVal, int maxheight, Color color)
  {
    if (snapVal == 0 && prevSnapVal == 0) return;
    g.setColor(color);
    g.drawLine(x, y - (maxheight * (snapVal + underGraphVal)) / PerfMon.scale, x + PerfMon.pixPerSecond, y - (maxheight * (prevSnapVal + underGraphPrevVal)) / PerfMon.scale);
  }


  public Dimension getPreferredSize()
  {
    return prefsz;
  }

  public void update(Graphics gg)
  {
    paint(gg);
  }

}
