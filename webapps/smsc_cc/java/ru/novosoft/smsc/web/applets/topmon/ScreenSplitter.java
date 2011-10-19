package ru.novosoft.smsc.web.applets.topmon;

import ru.novosoft.smsc.util.applet.LabelGroup;

import java.awt.*;

/**
 * Created by: Serge Lugovoy
 * Date: 14.05.2004
 * Time: 15:50:04
 */
class ScreenSplitter extends Container {
  private static final Color colorBackground = SystemColor.control;
  private LabelGroup smeTopLabel;
  private LabelGroup errTopLabel;
  private int split = 0;
  private int shiftStep;

  public void invalidate() {
    super.invalidate();
    Font font = getFont();
    if (font != null) {
      FontMetrics fm = getFontMetrics(font);
      shiftStep = fm.getHeight();
    }
    offscreen = null;
    Dimension sz = getSize();
    if (split == 0) {
      Dimension errMinSz = errTopLabel.getMinimumSize();
      split = errMinSz.height + 10 * shiftStep;
    }
    System.out.println("Invalidate: " + sz.width + "/" + sz.height + " spl=" + split);
    smeTopLabel.setSize(sz.width, sz.height - split);
    smeTopLabel.setLocation(0, 0);
    smeTopLabel.invalidate();
    errTopLabel.setSize(sz.width, split);
    errTopLabel.setLocation(0, sz.height - split);
    errTopLabel.invalidate();
  }

  public void shiftUp() {
    Dimension sz = getSize();
    Dimension msz = smeTopLabel.getMinimumSize();
    if (sz.height - split < msz.height + 6 * shiftStep) return;
    split += shiftStep;
    System.out.println("Split = " + split);
    invalidate();
    validate();
    repaint();
  }

  public void shiftDown() {
    Dimension msz = errTopLabel.getMinimumSize();
    if (split < msz.height + 6 * shiftStep) return;
    split -= shiftStep;
    System.out.println("Split = " + split);
    invalidate();
    validate();
    repaint();
  }

  public void update(Graphics gg) {
    paint(gg);
  }

  private Image offscreen = null;

  public void paint(Graphics gg) {
    Dimension sz = getSize();
    if (sz.width == 0 || sz.height == 0) return;
    if (offscreen == null) {
      offscreen = createImage(sz.width, sz.height);
    }
    Graphics g = offscreen.getGraphics();
    g.setColor(colorBackground);
    g.fillRect(0, 0, sz.width, sz.height);

    super.paint(g);

    gg.drawImage(offscreen, 0, 0, null);
    g.dispose();
  }
}
