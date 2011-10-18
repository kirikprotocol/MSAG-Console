package ru.novosoft.smsc.web.applets.perfmon;

import ru.novosoft.smsc.admin.perfmon.PerfSnap;

import java.awt.*;
import java.util.ResourceBundle;

/**
 * author: Aleksandr Khalitov
 */
public class PerfQueueTable extends Canvas {
  static final int vpad = 1;
  static final int hpad = 5;
  static final int numCols = 4;

  static final Color bgColor = SystemColor.control;
  static final Color textColor = SystemColor.textText;
  PerfSnap snap;
  Dimension prefSize;
  int columnWidth;
  Image offscreen;
  String heads[];


  public PerfQueueTable(PerfSnap snap, ResourceBundle bundle) {
    this.snap = new PerfSnap(snap);
    prefSize = new Dimension(100, 0);
    heads = new String[numCols];
    for (int i = 0; i < numCols; i++) {
      heads[i] = bundle.getString("pqueh." + i);
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
      prefSize.height = (fm.getHeight() + 2 * vpad);
      columnWidth = (sz.width - (numCols + 1) * hpad) / (numCols);
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

  StringBuffer sbs = new StringBuffer(256);

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
    g.setColor(textColor);

    int x = hpad;
    int y = fm.getAscent() + vpad;

    sbs.setLength(0);
    sbs.append(heads[0]).append(' ').append(snap.queueSize);
    g.drawString(sbs.toString(), x, y);
    x += columnWidth;

    sbs.setLength(0);
    sbs.append(heads[1]).append(' ').append(snap.processingSize);
    g.drawString(sbs.toString(), x, y);
    x += columnWidth;

    sbs.setLength(0);
    sbs.append(heads[2]).append(' ').append(snap.schedulerSize);
    g.drawString(sbs.toString(), x, y);
    x += columnWidth;

    sbs.setLength(0);
    sbs.append(heads[3]).append(' ').append(snap.dpfSize);
    g.drawString(sbs.toString(), x, y);
    x += columnWidth;

    gg.drawImage(offscreen, 0, 0, null);
    g.dispose();
  }

  public void update(Graphics gg) {
    paint(gg);
  }
}
