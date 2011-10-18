package ru.novosoft.smsc.web.applets.perfmon;

import ru.novosoft.smsc.admin.perfmon.PerfSnap;

import java.awt.*;
import java.util.ResourceBundle;

/**
 * author: Aleksandr Khalitov
 */
class PerformanceBar extends Canvas {
  private final String bottomString;
  private Image offscreen;
  private final int pixInGrid = 5;
  private int numGrids = 1;
  private int gridsInBlock = 1;
  private final int pad = 2;
  private int bottomSpace = pad;
  private int topSpace = pad;
  private final int graphWidth = 58;
  private final int barWidth = 46;
  private final int midBarWidth = 2;
  private final int sepBarWidth = 2;
  private int textwidth = 0;
  private final int numGraphs = 6;

  private final Dimension prefsz = new Dimension(88, 200);

  private PerfSnap snap;

  private final PerfMon perfMon;

  public PerformanceBar(PerfSnap snap, PerfMon perfMon, ResourceBundle bundle) {
    super();
    this.snap = snap;
    this.perfMon = perfMon;
    bottomString = bundle.getString("sms.per.s");
  }

  public synchronized void setSnap(PerfSnap snap) {
    this.snap = new PerfSnap(snap);
    this.repaint();
  }

  public void invalidate() {
    Font font = getFont();
    if (font != null) {
      FontMetrics fm = getFontMetrics(font);
      String scaleString = String.valueOf(perfMon.scale);
      textwidth = 3 * pad + fm.charsWidth(scaleString.toCharArray(), 0, scaleString.length());
      Dimension sz = getSize();

      bottomSpace = 2 * pad + fm.getDescent() + fm.getHeight();
      topSpace = pad + fm.getAscent();
      numGrids = (sz.height - bottomSpace - topSpace) / pixInGrid;
      gridsInBlock = (numGrids + 2) / perfMon.block;

      setSize(textwidth + graphWidth + 2 * pad, sz.height);
    }
    offscreen = null;
    super.invalidate();
  }

  private final Color colorText = Color.green;
  private final Color colorBackground = Color.black;
  private final Color colorGrid = new Color(0, 64, 0);
  private final Color colorGridLight = new Color(0, 128, 0);
  private final Color colorShadowBar = new Color(0, 96, 0);
  private final Color colorBarDeliver = Color.green;
  private final Color colorBarTempErr = Color.orange;
  private final Color colorBarDeliverErr = Color.red;
  private final Color colorBarSubmit = Color.blue;
  private final Color colorBarSubmitErr = Color.white;
  private final Color colorBarRetry = Color.cyan;

  public synchronized void paint(Graphics gg) {
    Dimension size = getSize();
    if (offscreen == null) {
      offscreen = createImage(size.width, size.height);
    }
    Graphics g = offscreen.getGraphics();

    Font font = getFont();
    FontMetrics fm = getFontMetrics(font);

    g.setColor(colorBackground);
    g.fillRect(0, 0, size.width, size.height);

    int gmax = (perfMon.block) * gridsInBlock;
    int barposx = pad + textwidth + (graphWidth - barWidth) / 2;

    int maxheight = gmax * pixInGrid;

    // bar background
    g.setColor(colorShadowBar);
    g.fillRect(barposx, size.height - maxheight - bottomSpace, barWidth, maxheight);
    int msuShift = perfMon.showMsu ? 6 : 0;
    if (perfMon.viewMode == PerfMon.VIEWMODE_IO) {
      int spent = 0;

      int halfBarWidth = (barWidth - midBarWidth) / 2;

      // last submit err bar
      g.setColor(colorBarSubmitErr);
      int barheight = (int) ((maxheight * snap.last[PerfSnap.IDX_SUBMITERR + msuShift]) / perfMon.scale);
      g.fillRect(barposx, size.height - bottomSpace - spent - barheight, halfBarWidth, barheight);
      spent += barheight;

      // last retry bar
      g.setColor(colorBarRetry);
      barheight = (int) ((maxheight * snap.last[PerfSnap.IDX_RETRY + msuShift]) / perfMon.scale);
      g.fillRect(barposx, size.height - bottomSpace - spent - barheight, halfBarWidth, barheight);
      spent += barheight;

      // last submit ok bar
      g.setColor(colorBarSubmit);
      barheight = (int) ((maxheight * snap.last[PerfSnap.IDX_SUBMIT + msuShift]) / perfMon.scale);
      g.fillRect(barposx, size.height - bottomSpace - spent - barheight, halfBarWidth, barheight);
      spent += barheight;

      spent = 0;

      // last deliver err bar
      g.setColor(colorBarDeliverErr);
      barheight = (int) ((maxheight * snap.last[PerfSnap.IDX_DELIVERERR + msuShift]) / perfMon.scale);
      g.fillRect(barposx + halfBarWidth + midBarWidth, size.height - bottomSpace - spent - barheight, halfBarWidth, barheight);
      spent += barheight;

      // last temp err bar
      g.setColor(colorBarTempErr);
      barheight = (int) ((maxheight * snap.last[PerfSnap.IDX_TEMPERR + msuShift]) / perfMon.scale);
      g.fillRect(barposx + halfBarWidth + midBarWidth, size.height - bottomSpace - spent - barheight, halfBarWidth, barheight);
      spent += barheight;

      // last deliver ok bar
      g.setColor(colorBarDeliver);
      barheight = (int) ((maxheight * snap.last[PerfSnap.IDX_DELIVER + msuShift]) / perfMon.scale);
      g.fillRect(barposx + halfBarWidth + midBarWidth, size.height - bottomSpace - spent - barheight, halfBarWidth, barheight);
      spent += barheight;

      // middle bar background
      g.setColor(colorBackground);
      barposx = pad + textwidth + (graphWidth - midBarWidth) / 2;
      g.fillRect(barposx, size.height - maxheight - bottomSpace, midBarWidth, maxheight);
    } else {
      int smallBarWidth = (barWidth - sepBarWidth * (numGraphs - 1)) / numGraphs;

      int barheight;

      // last submit ok bar
      g.setColor(colorBarSubmit);
      barheight = (int) ((maxheight * snap.last[PerfSnap.IDX_SUBMIT + msuShift]) / perfMon.scale);
      g.fillRect(barposx, size.height - bottomSpace - barheight, smallBarWidth, barheight);
      barposx += smallBarWidth;
      // separator bar background
      g.setColor(colorBackground);
      g.fillRect(barposx, size.height - maxheight - bottomSpace, sepBarWidth, maxheight);
      barposx += sepBarWidth;

      // last submit err bar
      g.setColor(colorBarSubmitErr);
      barheight = (int) ((maxheight * snap.last[PerfSnap.IDX_SUBMITERR + msuShift]) / perfMon.scale);
      g.fillRect(barposx, size.height - bottomSpace - barheight, smallBarWidth, barheight);
      barposx += smallBarWidth;
      // separator bar background
      g.setColor(colorBackground);
      g.fillRect(barposx, size.height - maxheight - bottomSpace, sepBarWidth, maxheight);
      barposx += sepBarWidth;

      // last retry bar
      g.setColor(colorBarRetry);
      barheight = (int) ((maxheight * snap.last[PerfSnap.IDX_RETRY + msuShift]) / perfMon.scale);
      g.fillRect(barposx, size.height - bottomSpace - barheight, smallBarWidth, barheight);
      barposx += smallBarWidth;
      // separator bar background
      g.setColor(colorBackground);
      g.fillRect(barposx, size.height - maxheight - bottomSpace, sepBarWidth, maxheight);
      barposx += sepBarWidth;

      // last deliver ok bar
      g.setColor(colorBarDeliver);
      barheight = (int) ((maxheight * snap.last[PerfSnap.IDX_DELIVER + msuShift]) / perfMon.scale);
      g.fillRect(barposx, size.height - bottomSpace - barheight, smallBarWidth, barheight);
      barposx += smallBarWidth;
      // separator bar background
      g.setColor(colorBackground);
      g.fillRect(barposx, size.height - maxheight - bottomSpace, sepBarWidth, maxheight);
      barposx += sepBarWidth;

      // last deliver err bar
      g.setColor(colorBarDeliverErr);
      barheight = (int) ((maxheight * snap.last[PerfSnap.IDX_DELIVERERR + msuShift]) / perfMon.scale);
      g.fillRect(barposx, size.height - bottomSpace - barheight, smallBarWidth, barheight);
      barposx += smallBarWidth;
      // separator bar background
      g.setColor(colorBackground);
      g.fillRect(barposx, size.height - maxheight - bottomSpace, sepBarWidth, maxheight);
      barposx += sepBarWidth;

      // last temp err bar
      g.setColor(colorBarTempErr);
      barheight = (int) ((maxheight * snap.last[PerfSnap.IDX_TEMPERR + msuShift]) / perfMon.scale);
      g.fillRect(barposx, size.height - bottomSpace - barheight, smallBarWidth, barheight);
      barposx += smallBarWidth;
      // separator bar background
      g.setColor(colorBackground);
      g.fillRect(barposx, size.height - maxheight - bottomSpace, sepBarWidth, maxheight);
      barposx += sepBarWidth;
    }

    int labcnt = 0;

    for (int i = 0; i <= gmax; i++) {
      int yy = size.height - bottomSpace - i * pixInGrid;
      if ((i % gridsInBlock) == 0) {
        g.setColor(colorText);
        String s = String.valueOf(labcnt);
        g.drawChars(s.toCharArray(), 0, s.length(),
            size.width - graphWidth - 3 * pad - fm.charsWidth(s.toCharArray(), 0, s.length()),
            yy + fm.getDescent());
        labcnt += perfMon.scale / perfMon.block;
        g.setColor(colorGridLight);
      } else {
        g.setColor(colorGrid);
      }
      g.drawLine(size.width - graphWidth - pad, yy, size.width - pad, yy);
    }

    g.setColor(colorText);
    g.drawChars(bottomString.toCharArray(), 0, bottomString.length(),
        (size.width - fm.charsWidth(bottomString.toCharArray(), 0, bottomString.length())) / 2,
        size.height - pad - fm.getDescent());

    gg.drawImage(offscreen, 0, 0, null);
    g.dispose();
  }


  public Dimension getPreferredSize() {
    return prefsz;
  }

  public void update(Graphics gg) {
    paint(gg);
  }

}
