package ru.novosoft.smsc.perfmon.applet;

import java.awt.*;

import ru.novosoft.smsc.perfmon.PerfSnap;

public class PerformanceBar extends Canvas {
  String bottomString;
  Image offscreen;
  int pixInGrid = 5;
  int numGrids = 1;
  int gridsInBlock = 1;
  int pad = 2;
  int bottomSpace = pad;
  int topSpace = pad;
  int graphWidth = 50;
  int barWidth = 38;
  int midBarWidth = 2;
  int sepBarWidth = 2;
  int textwidth = 0;

  Dimension prefsz = new Dimension(80, 200);

  PerfSnap snap;

  public PerformanceBar(PerfSnap snap) {
    super();
    this.snap = snap;
    bottomString = PerfMon.localeText.getString("sms.per.s");
  }

  public synchronized void setSnap(PerfSnap snap) {
    this.snap = new PerfSnap(snap);
    this.repaint();
  }

  public void invalidate() {
    Font font = getFont();
    if(font != null) {
      FontMetrics fm = getFontMetrics(font);
      String scaleString = String.valueOf(PerfMon.scale);
      textwidth = 3*pad + fm.charsWidth(scaleString.toCharArray(), 0, scaleString.length());
      Dimension sz = getSize();

      bottomSpace = 2*pad + fm.getDescent() + fm.getHeight();
      topSpace = pad + fm.getAscent();
      numGrids = (sz.height - bottomSpace - topSpace)/5;
      gridsInBlock = (numGrids + 2)/PerfMon.block;

      setSize(textwidth + graphWidth + 2*pad, sz.height);
    }
    offscreen = null;
    super.invalidate();
  }

  Color colorText = Color.green;
  Color colorBackground = Color.black;
  Color colorGrid = new Color(0, 64, 0);
  Color colorGridLight = new Color(0, 128, 0);
  Color colorShadowBar = new Color(0, 96, 0);
  Color colorBarDeliver = Color.green;
  Color colorBarDeliverErr = Color.red;
  Color colorBarSubmit = Color.blue;
  Color colorBarSubmitErr = Color.orange;
  Color colorBarRetry = Color.cyan;

  public synchronized void paint(Graphics gg) {
    Dimension size = getSize();
    if(offscreen == null) {
      offscreen = createImage(size.width, size.height);
    }
    Graphics g = offscreen.getGraphics();

    Font font = getFont();
    FontMetrics fm = getFontMetrics(font);

    g.setColor(colorBackground);
    g.fillRect(0, 0, size.width, size.height);

    int gmax = (PerfMon.block)*gridsInBlock;
    int barposx = pad + textwidth + (graphWidth - barWidth)/2;

    int maxheight = gmax*pixInGrid;

    // bar background
    g.setColor(colorShadowBar);
    g.fillRect(barposx, size.height - maxheight - bottomSpace, barWidth, maxheight);

    if( PerfMon.viewMode == PerfMon.VIEWMODE_IO ) {
      int spent = 0;

      int halfBarWidth = (barWidth - midBarWidth)/2;

      // last submit err bar
      g.setColor(colorBarSubmitErr);
      int barheight = (int) ((maxheight*snap.last[PerfSnap.IDX_SUBMITERR])/PerfMon.scale);
      g.fillRect(barposx, size.height - bottomSpace - spent - barheight, halfBarWidth, barheight);
      spent += barheight;

      // last retry bar
      g.setColor(colorBarRetry);
      barheight = (int) ((maxheight*snap.last[PerfSnap.IDX_RETRY])/PerfMon.scale);
      g.fillRect(barposx, size.height - bottomSpace - spent - barheight, halfBarWidth, barheight);
      spent += barheight;

      // last submit ok bar
      g.setColor(colorBarSubmit);
      barheight = (int) ((maxheight*snap.last[PerfSnap.IDX_SUBMIT])/PerfMon.scale);
      g.fillRect(barposx, size.height - bottomSpace - spent - barheight, halfBarWidth, barheight);
      spent += barheight;

      spent = 0;

      // last deliver err bar
      g.setColor(colorBarDeliverErr);
      barheight = (int) ((maxheight*snap.last[PerfSnap.IDX_DELIVERERR])/PerfMon.scale);
      g.fillRect(barposx + halfBarWidth + midBarWidth, size.height - bottomSpace - spent - barheight, halfBarWidth, barheight);
      spent += barheight;

      // last deliver ok bar
      g.setColor(colorBarDeliver);
      barheight = (int) ((maxheight*snap.last[PerfSnap.IDX_DELIVER])/PerfMon.scale);
      g.fillRect(barposx + halfBarWidth + midBarWidth, size.height - bottomSpace - spent - barheight, halfBarWidth, barheight);
      spent += barheight;

      // middle bar background
      g.setColor(colorBackground);
      barposx = pad + textwidth + (graphWidth - midBarWidth)/2;
      g.fillRect(barposx, size.height - maxheight - bottomSpace, midBarWidth, maxheight);
    } else {
      int smallBarWidth = (barWidth - sepBarWidth*4)/5;

      // last submit err bar
      g.setColor(colorBarSubmitErr);
      int barheight = (int) ((maxheight*snap.last[PerfSnap.IDX_SUBMITERR])/PerfMon.scale);
      g.fillRect(barposx, size.height - bottomSpace - barheight, smallBarWidth, barheight);
      barposx += smallBarWidth;
      // separator bar background
      g.setColor(colorBackground);
      g.fillRect(barposx, size.height - maxheight - bottomSpace, sepBarWidth, maxheight);
      barposx += sepBarWidth;

      // last retry bar
      g.setColor(colorBarRetry);
      barheight = (int) ((maxheight*snap.last[PerfSnap.IDX_RETRY])/PerfMon.scale);
      g.fillRect(barposx, size.height - bottomSpace - barheight, smallBarWidth, barheight);
      barposx += smallBarWidth;
      // separator bar background
      g.setColor(colorBackground);
      g.fillRect(barposx, size.height - maxheight - bottomSpace, sepBarWidth, maxheight);
      barposx += sepBarWidth;

      // last submit ok bar
      g.setColor(colorBarSubmit);
      barheight = (int) ((maxheight*snap.last[PerfSnap.IDX_SUBMIT])/PerfMon.scale);
      g.fillRect(barposx, size.height - bottomSpace - barheight, smallBarWidth, barheight);
      barposx += smallBarWidth;
      // separator bar background
      g.setColor(colorBackground);
      g.fillRect(barposx, size.height - maxheight - bottomSpace, sepBarWidth, maxheight);
      barposx += sepBarWidth;

      // last deliver err bar
      g.setColor(colorBarDeliverErr);
      barheight = (int) ((maxheight*snap.last[PerfSnap.IDX_DELIVERERR])/PerfMon.scale);
      g.fillRect(barposx, size.height - bottomSpace - barheight, smallBarWidth, barheight);
      barposx += smallBarWidth;
      // separator bar background
      g.setColor(colorBackground);
      g.fillRect(barposx, size.height - maxheight - bottomSpace, sepBarWidth, maxheight);
      barposx += sepBarWidth;

      // last deliver ok bar
      g.setColor(colorBarDeliver);
      barheight = (int) ((maxheight*snap.last[PerfSnap.IDX_DELIVER])/PerfMon.scale);
      g.fillRect(barposx, size.height - bottomSpace - barheight, smallBarWidth, barheight);
      barposx += smallBarWidth;
      // separator bar background
      g.setColor(colorBackground);
      g.fillRect(barposx, size.height - maxheight - bottomSpace, sepBarWidth, maxheight);
      barposx += sepBarWidth;

    }

    int labcnt = 0;

    for(int i = 0; i <= gmax; i++) {
      int yy = size.height - bottomSpace - i*pixInGrid;
      if((i%gridsInBlock) == 0) {
        g.setColor(colorText);
        String s = String.valueOf(labcnt);
        g.drawChars(s.toCharArray(), 0, s.length(),
                size.width - graphWidth - 3*pad - fm.charsWidth(s.toCharArray(), 0, s.length()),
                yy + fm.getDescent());
        labcnt += PerfMon.scale/PerfMon.block;
        g.setColor(colorGridLight);
      } else {
        g.setColor(colorGrid);
      }
      g.drawLine(size.width - graphWidth - pad, yy, size.width - pad, yy);
    }

    g.setColor(colorText);
    g.drawChars(bottomString.toCharArray(), 0, bottomString.length(),
            (size.width - fm.charsWidth(bottomString.toCharArray(), 0, bottomString.length()))/2,
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
