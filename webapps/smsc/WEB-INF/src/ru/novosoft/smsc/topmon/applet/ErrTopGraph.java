package ru.novosoft.smsc.topmon.applet;

import ru.novosoft.smsc.topmon.TopSnap;

import java.awt.*;

/**
 * Created by: Serge Lugovoy
 * Date: 14.05.2004
 * Time: 15:28:05
 */
public class ErrTopGraph extends Canvas
{
  static final int pad = 2;
  private static final Color colorBackground = Color.black;

  Image offscreen;
  int bottomSpace = pad;
  int topSpace = pad;

  TopSnap snap;

  public ErrTopGraph(TopSnap snap)
  {
    super();
    this.snap = snap;
  }

  public void setSnap(TopSnap snap)
  {
    this.snap = snap;
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
    gg.drawImage(offscreen, 0, 0, null);
    g.dispose();
  }

  public void update(Graphics gg)
  {
    paint(gg);
  }

  Dimension prefsz = new Dimension(80, 200);

  public Dimension getPreferredSize()
  {
    return prefsz;
  }

  Dimension minsz = new Dimension(0, 0);

  public Dimension getMinimumSize()
  {
    return minsz;
  }
}
