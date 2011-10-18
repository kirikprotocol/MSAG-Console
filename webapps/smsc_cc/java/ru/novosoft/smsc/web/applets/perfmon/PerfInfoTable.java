package ru.novosoft.smsc.web.applets.perfmon;

import ru.novosoft.smsc.admin.perfmon.PerfSnap;

import java.awt.*;
import java.util.ResourceBundle;


public class PerfInfoTable extends Canvas {

  static final int pad = 1;
  static final int grid = 1;
  static final int numHeads = 6;
  static final int numRows = 3;
  static final int colorShift = 16;

  static final Color bgColor = SystemColor.control;
  static final Color shadowColor = SystemColor.controlShadow;
  static final Color lightShadowColor = SystemColor.controlLtHighlight;
  static final Color textColor = SystemColor.textText;
  static Color[] headsColor = null;

  static {
    headsColor = new Color[numHeads];
    try {
      headsColor[0] = new Color(bgColor.getRed(), bgColor.getGreen(), bgColor.getBlue() + colorShift);
    } catch (Exception e) {
      headsColor[0] = bgColor;
    }
    try {
      headsColor[1] = new Color(bgColor.getRed() + colorShift, bgColor.getGreen() + colorShift, bgColor.getBlue() + colorShift);
    } catch (Exception e) {
      headsColor[1] = bgColor;
    }
    try {
      headsColor[2] = new Color(bgColor.getRed(), bgColor.getGreen() + colorShift, bgColor.getBlue() + colorShift);
    } catch (Exception e) {
      headsColor[2] = bgColor;
    }
    try {
      headsColor[3] = new Color(bgColor.getRed(), bgColor.getGreen() + colorShift, bgColor.getBlue());
    } catch (Exception e) {
      headsColor[3] = bgColor;
    }
    try {
      headsColor[4] = new Color(bgColor.getRed() + colorShift, bgColor.getGreen(), bgColor.getBlue());
    } catch (Exception e) {
      headsColor[4] = bgColor;
    }
    try {
      headsColor[5] = new Color(bgColor.getRed() + colorShift, bgColor.getGreen() + colorShift, bgColor.getBlue());
    } catch (Exception e) {
      headsColor[5] = bgColor;
    }
  }

  ;

  PerfSnap snap;
  Dimension prefSize;
  Image offscreen;
  int columnWidth;

  String heads[];
  String rows[];

  private PerfMon perfMon;

  public PerfInfoTable(PerfSnap snap, ResourceBundle bundle, PerfMon perfMon) {
    this.snap = new PerfSnap(snap);
    prefSize = new Dimension(100, 0);
    this.perfMon = perfMon;

    heads = new String[numHeads];
    for (int i = 0; i < numHeads; i++) {
      heads[i] = bundle.getString("ptabh." + i);
    }

    rows = new String[numRows];
    for (int i = 0; i < numRows; i++) {
      rows[i] = bundle.getString("ptabr." + i);
    }

    invalidate();
  }

  public synchronized void setSnap(PerfSnap snap) {
    this.snap = new PerfSnap(snap);
    this.repaint();
  }

  public void invalidate() {
    Font font = getFont();
    if (font != null) {
      FontMetrics fm = getFontMetrics(font);
      Dimension sz = getSize();
      prefSize.height = (fm.getHeight() + pad) * (rows.length + 1) + grid * rows.length;
      columnWidth = (sz.width - 2 * pad) / (numHeads + 1);
      setSize(sz.width, prefSize.height);
    }
    offscreen = null;
    super.invalidate();
  }

  public Dimension getPrefferedSize() {
    return prefSize;
  }

  public Dimension getMinimumSize() {
    return prefSize;
  }

  public synchronized void paint(Graphics gg) {
    Dimension sz = getSize();
    if (offscreen == null) {
      offscreen = createImage(sz.width, sz.height);
    }
    Graphics g = offscreen.getGraphics();
    g.setColor(bgColor);
    g.fillRect(0, 0, sz.width, sz.height);


    Font font = getFont();
    FontMetrics fm = getFontMetrics(font);

    // change background for rows
    {
      int y = fm.getHeight() + pad + grid;
      int height = sz.height - y - pad;
      int x = columnWidth + grid + pad;
      for (int i = 0; i < headsColor.length; i++) {
        g.setColor(headsColor[i]);
        g.fillRect(x, y, columnWidth, height);
        x += columnWidth;
      }
    }

    {
      // draw column headings
      g.setColor(textColor);
      int x = columnWidth;
      for (int i = 0; i < numHeads; i++) {
        g.drawChars(heads[i].toCharArray(), 0, heads[i].length(),
            x + (columnWidth - fm.charsWidth(heads[i].toCharArray(), 0, heads[i].length())) - pad,
            fm.getAscent() + pad);
        x += columnWidth;
      }
    }

    {
      // draw row headings
      g.setColor(textColor);
      int y = fm.getHeight() + pad + grid + fm.getAscent();
      for (int i = 0; i < numRows; i++) {
        g.drawChars(rows[i].toCharArray(), 0, rows[i].length(),
            columnWidth - fm.charsWidth(rows[i].toCharArray(), 0, rows[i].length()) - pad,
            y);
        y += fm.getHeight() + 2 * pad + grid;
      }
    }

    {
      // draw counters
      drawCounters(g, snap.last, sz, fm, 1);
      drawCounters(g, snap.avg, sz, fm, 2);
      drawCounters(g, snap.total, sz, fm, 3);
    }

    {
      // draw grids
      int y = fm.getHeight() + 2 * pad + grid;

/*            if( grid > 1 ) {
              g.setColor(lightShadowColor);
              for (int i = 1; i <= rows.length; i++) {
                  g.drawLine(0, i * y, sz.width, i * y);
              }
              for (int i = 1; i <= 3; i++) {
                  g.drawLine(i * x + 3, 0, i * x + 3, sz.height);
              }
            }
*/
      g.setColor(shadowColor);
      for (int i = 0; i < numRows; i++) {
        g.drawLine(pad, (i + 1) * y, sz.width - pad, (i + 1) * y);
      }
      for (int i = 0; i < numHeads; i++) {
        g.drawLine((i + 1) * columnWidth, pad, (i + 1) * columnWidth, sz.height - pad);
      }
    }

    gg.drawImage(offscreen, 0, 0, null);
    g.dispose();
  }

  void drawCounters(Graphics g, long counters[], Dimension sz, FontMetrics fm, int col) {
    g.setColor(textColor);
    int y = fm.getHeight() + 2 * pad + grid;
    int x = columnWidth;
    int msuShift = perfMon.showMsu ? 6 : 0;
    for (int i = 0; i < numHeads; i++) {
      char counter[] = String.valueOf(counters[i + msuShift]).toCharArray();
      g.drawChars(counter, 0, counter.length,
          (i + 2) * x - fm.charsWidth(counter, 0, counter.length) - pad,
          (col) * y + fm.getAscent() + pad);
    }
  }

  public void update(Graphics gg) {
    paint(gg);
  }


}
