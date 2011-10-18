package ru.novosoft.smsc.util.applet;

import java.awt.*;

// not blinking on repair label

public class AdvancedLabel extends Canvas {
  Image offscreen;

  public final static int LEFT = 0;
  public final static int CENTER = 1;
  public final static int RIGHT = 2;

  int align = LEFT;
  String text = null;
  int pad = 1;

  Dimension prefSize = new Dimension(0, 0);

  public AdvancedLabel(String text) {
    this.text = text;
  }

  public AdvancedLabel(String text, int align) {
    this.text = text;
    this.align = align;
  }

  public void setText(String text) {
    this.text = text;
    repaint();
  }

  public void invalidate() {
    if (text == null) {
      prefSize.width = prefSize.height = 0;
    } else {
      Font font = getFont();
      if (font != null) {
        FontMetrics fm = getFontMetrics(font);
        int textwidth = fm.charsWidth(text.toCharArray(), 0, text.length());
        int textheight = fm.getHeight();
        prefSize.width = 2 * pad + textwidth;
        prefSize.height = 2 * pad + textheight;
        Dimension sz = getSize();
        if (sz.height < prefSize.height)
          setSize(sz.width, prefSize.height);
      }
    }
    offscreen = null;

    super.invalidate();
  }

  public Dimension getPreferredSize() {
    return prefSize;
  }

  public void paint(Graphics gg) {
    Dimension size = getSize();
    if (offscreen == null) {
      offscreen = createImage(size.width, size.height);
    }
    Graphics g = offscreen.getGraphics();

    Font font = getFont();
    FontMetrics fm = getFontMetrics(font);

    g.setColor(getBackground());
    g.clearRect(0, 0, size.width, size.height);
    g.setFont(font);
    g.setColor(getForeground());
    if (align == LEFT) {
      g.drawChars(text.toCharArray(), 0, text.length(), pad, pad + fm.getAscent());
    } else if (align == CENTER) {
      g.drawChars(text.toCharArray(), 0, text.length(), pad + fm.charsWidth(text.toCharArray(), 0, text.length()) / 2, pad + fm.getAscent());
    } else {
      g.drawChars(text.toCharArray(), 0, text.length(), size.width - pad - fm.charsWidth(text.toCharArray(), 0, text.length()), pad + fm.getAscent());
    }

    gg.drawImage(offscreen, 0, 0, null);
    g.dispose();
  }

  public void update(Graphics gg) {
    paint(gg);
  }
}

