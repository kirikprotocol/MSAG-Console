package ru.sibinco.scag.perfmon.applet;

import ru.sibinco.scag.perfmon.PerfSnap;

import java.awt.*;

public class PerformanceBar extends Canvas {
  String bottomString;
  Image offscreen;
  int pixInGrid = 5;
  int numGrids = 1;
  int gridsInBlock = 1;
  int pad = 2;
  int bottomSpace = pad;
  int topSpace = pad;
  int graphWidth = 58;
  int barWidth = 46;
  int sepBarWidth = 2;
  int textwidth = 0;
  int numGraphs = 6;

  Dimension prefsz = new Dimension(88, 200);

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
      numGrids = (sz.height - bottomSpace - topSpace)/pixInGrid;
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
  Color colorBarDelivered = Color.green;
  Color colorBarGwRejected = Color.cyan;
  Color colorBarAccepted = Color.blue;
  Color colorBarRejected = Color.white;
  Color colorBarFailed = Color.red;

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

      int smallBarWidth = (barWidth - sepBarWidth*2)/3;

      // last rejected bar
      g.setColor(colorBarRejected);
      int barheight = (int) ((maxheight*snap.last[PerfSnap.IDX_REJECTED])/PerfMon.scale);
      g.fillRect(barposx, size.height - bottomSpace - spent - barheight, smallBarWidth, barheight);
      spent += barheight;

      // last accepted bar
      g.setColor(colorBarAccepted);
      barheight = (int) ((maxheight*snap.last[PerfSnap.IDX_ACCEPTED])/PerfMon.scale);
      g.fillRect(barposx, size.height - bottomSpace - spent - barheight, smallBarWidth, barheight);
      spent += barheight;

      barposx += smallBarWidth;
      g.setColor(colorBackground);
      g.fillRect(barposx, size.height - maxheight - bottomSpace, sepBarWidth, maxheight);
      barposx += sepBarWidth;
      spent = 0;

      //last Gw Rejected bar
      g.setColor(colorBarGwRejected);
      barheight = (int) ((maxheight*snap.last[PerfSnap.IDX_GW_REJECTED])/PerfMon.scale);
      g.fillRect(barposx, size.height - bottomSpace - spent - barheight, smallBarWidth, barheight);
      spent += barheight;

      // last delivered bar
      g.setColor(colorBarDelivered);
      barheight = (int) ((maxheight*snap.last[PerfSnap.IDX_DELIVERED])/PerfMon.scale);
      g.fillRect(barposx, size.height - bottomSpace - spent - barheight, smallBarWidth, barheight);
      spent += barheight;

      barposx += smallBarWidth;
      g.setColor(colorBackground);
      g.fillRect(barposx, size.height - maxheight - bottomSpace, sepBarWidth, maxheight);
      barposx += sepBarWidth;
      spent = 0;

      // last failed bar
      g.setColor(colorBarFailed);
      barheight = (int) ((maxheight*snap.last[PerfSnap.IDX_FAILED])/PerfMon.scale);
      g.fillRect(barposx, size.height - bottomSpace - spent - barheight, smallBarWidth, barheight);
      spent += barheight;

      barposx += smallBarWidth;
    } else {
      int smallBarWidth = (barWidth - sepBarWidth*(numGraphs-1))/numGraphs;

      int barheight = 0;

      // last submit ok bar
      g.setColor(colorBarAccepted);
      barheight = (int) ((maxheight*snap.last[PerfSnap.IDX_ACCEPTED])/PerfMon.scale);
      g.fillRect(barposx, size.height - bottomSpace - barheight, smallBarWidth, barheight);
      barposx += smallBarWidth;
      // separator bar background
      g.setColor(colorBackground);
      g.fillRect(barposx, size.height - maxheight - bottomSpace, sepBarWidth, maxheight);
      barposx += sepBarWidth;

      // last submit err bar
      g.setColor(colorBarRejected);
      barheight = (int) ((maxheight*snap.last[PerfSnap.IDX_REJECTED])/PerfMon.scale);
      g.fillRect(barposx, size.height - bottomSpace - barheight, smallBarWidth, barheight);
      barposx += smallBarWidth;
      // separator bar background
      g.setColor(colorBackground);
      g.fillRect(barposx, size.height - maxheight - bottomSpace, sepBarWidth, maxheight);
      barposx += sepBarWidth;


      // last deliver ok bar
      g.setColor(colorBarDelivered);
      barheight = (int) ((maxheight*snap.last[PerfSnap.IDX_DELIVERED])/PerfMon.scale);
      g.fillRect(barposx, size.height - bottomSpace - barheight, smallBarWidth, barheight);
      barposx += smallBarWidth;
      // separator bar background
      g.setColor(colorBackground);
      g.fillRect(barposx, size.height - maxheight - bottomSpace, sepBarWidth, maxheight);
      barposx += sepBarWidth;

      // last gw rejected  bar
      g.setColor(colorBarGwRejected);
      barheight = (int) ((maxheight*snap.last[PerfSnap.IDX_GW_REJECTED])/PerfMon.scale);
      g.fillRect(barposx, size.height - bottomSpace - barheight, smallBarWidth, barheight);
      barposx += smallBarWidth;
      // separator bar background
      g.setColor(colorBackground);
      g.fillRect(barposx, size.height - maxheight - bottomSpace, sepBarWidth, maxheight);
      barposx += sepBarWidth;

        // last retry bar
        g.setColor(colorBarFailed);
        barheight = (int) ((maxheight*snap.last[PerfSnap.IDX_FAILED])/PerfMon.scale);
        g.fillRect(barposx, size.height - bottomSpace - barheight, smallBarWidth, barheight);
        barposx += smallBarWidth;
        // separator bar background
        g.setColor(colorBackground);
        g.fillRect(barposx, size.height - maxheight - bottomSpace, sepBarWidth, maxheight);
        barposx += sepBarWidth;


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
