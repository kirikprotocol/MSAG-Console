package ru.novosoft.smsc.util.applet;

import java.awt.*;
import java.util.StringTokenizer;
import java.util.Vector;

/**
 * A container with an optional label.<br>
 * If the label is set, the group shows a border.
 */
public class LabelGroup
    extends Container {

  /**
   * The centre alignment.
   * This can be used as a constant to render a background image centrally.
   */
  public static final int CENTER = 0;

  /**
   * The north alignment.
   */
  public static final int NORTH = 1;

  /**
   * The east alignment.
   */
  public static final int EAST = 2;

  /**
   * The south alignment.
   */
  public static final int SOUTH = 3;

  /**
   * The west alignment.
   */
  public static final int WEST = 4;

  /**
   * The northeast alignment.
   */
  public static final int NORTHEAST = 5;

  /**
   * The southeast alignment.
   */
  public static final int SOUTHEAST = 6;

  /**
   * The southwest alignment.
   */
  public static final int SOUTHWEST = 7;

  /**
   * The northwest alignment.
   */
  public static final int NORTHWEST = 8;
  /**
   * The alignment of the label.
   */
  protected int align;

  /**
   * The label.
   */
  protected String label;

  /**
   * The additional insets for this group.
   */
  protected Insets insets;

  /**
   * Constructs a group with no label or border.
   */
  public LabelGroup() {
    this(null, NORTHWEST);
  }

  /**
   * Constructs a group with the specified label and centre alignment.
   *
   * @param label the label to display
   */
  public LabelGroup(String label) {
    this(label, NORTHWEST);
  }

  /**
   * Constructs a bordered group with the specified label and alignment.
   *
   * @param label the label to display
   * @param align the alignment of the label (NORTH, NORTHEAST, NORTHWEST, SOUTH, SOUTHEAST, or SOUTHWEST)
   */
  public LabelGroup(String label, int align) {
    setAlign(align);
    this.label = label;
  }

  // -- Accessors --

  /**
   * Returns this group's label.
   *
   * @see #setLabel
   */
  public String getLabel() {
    return label;
  }

  /**
   * Sets this group's label.
   *
   * @param label the desired label
   * @see #getLabel
   */
  public void setLabel(String label) {
    if ((label == null && this.label != null) || (label != null && !label.equals(this.label))) {
      String old = this.label;
      this.label = label;
      invalidate();
      repaint();
    }
  }

  /**
   * Returns the label alignment.
   *
   * @see #setAlign
   */
  public int getAlign() {
    return align;
  }

  /**
   * Sets the label alignment.
   *
   * @param align the alignment of the label (NORTH, NORTHEAST, NORTHWEST, SOUTH, SOUTHEAST, or SOUTHWEST)
   * @see #getAlign
   */
  public void setAlign(int align) {
    switch (align) {
      case NORTH:
      case NORTHEAST:
      case NORTHWEST:
      case SOUTH:
      case SOUTHEAST:
      case SOUTHWEST:
        if (this.align != align) {
          int old = this.align;
          this.align = align;
          repaint();
        }
        break;
      default:
        throw new IllegalArgumentException("illegal alignment: " + align);
    }
  }

  /**
   * Sets the additional insets for this group.
   */
  public void setInsets(Insets insets) {
    if (insets == null || !insets.equals(this.insets)) {
      Insets old = this.insets;
      this.insets = insets;
    }
  }

  // -- Layout and painting --

  public Dimension getMinimumSize() {
    Insets insets = getInsets();
    Dimension sz = new Dimension(insets.left + insets.right, insets.top + insets.bottom);
    return super.getMinimumSize();
  }

  /**
   * Returns the insets of this group.
   */
  public Insets getInsets() {
    Insets i = (insets != null) ? insets : new Insets(0, 0, 0, 0);
    if (label != null) {
      Font font = getFont();
      FontMetrics fm = getFontMetrics(font);
      i.top += fm.getHeight() + 2;
      i.bottom += 6;
      i.left += 6;
      i.right += 6;
    } else {
      i.top += 6;
      i.bottom += 6;
      i.left += 6;
      i.right += 6;
    }
    return i;
  }

  /**
   * Paints this component.
   */
  public void paint(Graphics g) {
    if (label != null) {
      Font font = getFont();
      FontMetrics fm = getFontMetrics(font);
      Dimension size = getSize();
      Rectangle inside = new Rectangle(0, 0, size.width, size.height);
      Color cs = SystemColor.controlShadow;
      Color csl = SystemColor.controlLtHighlight;
      Color f = isEnabled() ? SystemColor.textText : SystemColor.textInactiveText;

      // discover the label position and extent
      int tw = fm.stringWidth(label) + 4, th = fm.getHeight();
      int ascent = fm.getAscent();
      Rectangle lr = new Rectangle(0, 0, (tw < inside.width - 16) ? tw : inside.width - 16, th);
      switch (align) {
        case SOUTHWEST:
          lr.y = inside.height - lr.height;
        case NORTHWEST:
          lr.x = 8;
          break;
        case SOUTH:
          lr.y = inside.height - lr.height;
        case NORTH:
          lr.x = (inside.width - lr.width) / 2;
          break;
        case SOUTHEAST:
          lr.y = inside.height - lr.height;
        case NORTHEAST:
          lr.x = inside.width - lr.width - 8;
          break;
      }

      // draw the border
      g.setColor(csl);
      g.drawLine(lr.x + lr.width + 1, (ascent / 2) + 2 + 1, inside.width - 2 + 1, (ascent / 2) + 2 + 1);
      g.drawLine(inside.width - 2 + 1, (ascent / 2) + 2 + 1, inside.width - 2 + 1, inside.height - 2 + 1);
      g.drawLine(inside.width - 2 + 1, inside.height - 2 + 1, 2 + 1, inside.height - 2 + 1);
      g.drawLine(2 + 1, inside.height - 2 + 1, 2 + 1, (ascent / 2) + 2 + 1);
      g.drawLine(2 + 1, (ascent / 2) + 2 + 1, lr.x + 1, (ascent / 2) + 2 + 1);

      // draw the border
      g.setColor(cs);
      g.drawLine(lr.x + lr.width, (ascent / 2) + 2, inside.width - 2, (ascent / 2) + 2);
      g.drawLine(inside.width - 2, (ascent / 2) + 2, inside.width - 2, inside.height - 2);
      g.drawLine(inside.width - 2, inside.height - 2, 2, inside.height - 2);
      g.drawLine(2, inside.height - 2, 2, (ascent / 2) + 2);
      g.drawLine(2, (ascent / 2) + 2, lr.x, (ascent / 2) + 2);

      // draw the label
      drawLabel(g, new Rectangle(lr.x + 2, lr.y, lr.width - 4, lr.height), fm, label, align, f);
    } else {
      Dimension size = getSize();
      Color cs = SystemColor.controlShadow;
      Color csl = SystemColor.controlLtHighlight;

      g.setColor(csl);
      g.drawRect(3, 3, size.width - 4, size.height - 4);
      g.setColor(cs);
      g.drawRect(2, 2, size.width - 4, size.height - 4);
    }
    super.paint(g); // paint children
  }

  /**
   * Returns the array of strings resulting from attempting to fit a string into a specified width,
   * according to the specified font metrics and splitting the original string at spaces.
   * This preserves line breaks.
   */
  protected static String[] getRows(FontMetrics fm, String label, int width) {
    String row = "";
    Vector rows = new Vector();
    for (StringTokenizer st = new StringTokenizer(label, " \n", true); st.hasMoreTokens(); ) {
      String token = st.nextToken();
      if (" ".equals(token)) {
      } else if ("\n".equals(token)) {
        rows.addElement(row);
        row = "";
      } else {
        String test = row + token;
        int rw = fm.stringWidth(test);
        if (rw > (width - 2) && row.length() > 0) {
          rows.addElement(row);
          row = "";
        }
        row = row + ((row.length() > 0) ? " " : "") + token;
      }
    }
    if (row.length() > 0)
      rows.addElement(row);
    String[] r = new String[rows.size()];
    rows.copyInto(r);
    return r;
  }

  /**
   * Draws a multiline label in the specified bounds rectangle.
   *
   * @param g     the graphics context
   * @param r     the bounding rectangle of the label
   * @param fm    the font metrics to apply to the string
   * @param label the label string
   * @param align the alignment of the label within the bounds
   * @param f     the colour of the text
   */
  protected static void drawLabel(Graphics g, Rectangle r, FontMetrics fm, String label, int align, Color f) {
    String[] rows = getRows(fm, label, r.width);
    int rh = fm.getHeight(), ascent = fm.getAscent();
    /*
     * Rectangle lr = new Rectangle(0, 0, 0, 0);
     * // set maxima for label rectangle (minima will be calculated)
     * if (align==CENTER || align==NORTH || align==SOUTH)
     * 	lr.x = r.width/2;
     * else if (align==EAST || align==NORTHEAST || align==SOUTHEAST)
     * 	lr.x = r.width;
     * else if (align==WEST || align==SOUTHWEST || align==NORTHWEST)
     * 	lr.x = 0;
     * if (align==CENTER || align==EAST || align==WEST)
     * 	lr.y = (r.height-(rows.length*rh))/2;
     * else if (align==NORTH || align==NORTHEAST || align==NORTHWEST)
     * 	lr.y = 0;
     * else if (align==SOUTH || align==SOUTHEAST || align==SOUTHWEST)
     * 	lr.y = r.height-(rows.length*rh);
     */
    g.setColor(f);
    for (int i = 0; i < rows.length; i++) {
      Rectangle rr = new Rectangle(0, 0, fm.stringWidth(rows[i]), rh);
      // set position of current row
      if (align == CENTER || align == NORTH || align == SOUTH)
        rr.x = (r.width - rr.width) / 2;
      else if (align == EAST || align == NORTHEAST || align == SOUTHEAST)
        rr.x = r.width - rr.width;
      else if (align == WEST || align == SOUTHWEST || align == NORTHWEST)
        rr.x = 0;
      if (align == CENTER || align == EAST || align == WEST)
        rr.y = (rh * i) + ((r.height - (rows.length * rh)) / 2);
      else if (align == NORTH || align == NORTHEAST || align == NORTHWEST)
        rr.y = (rh * i);
      else if (align == SOUTH || align == SOUTHEAST || align == SOUTHWEST)
        rr.y = (rh * i) + (r.height - (rows.length * rh));

      g.drawString(rows[i], r.x + rr.x, r.y + ascent + rr.y);
      /*
       * lr.x = Math.min(lr.x, rr.x);
       * lr.width = Math.max(lr.width, rr.width);
       * lr.y = Math.min(lr.y, rr.y);
       * lr.height += rr.height;
       */
    }
    /*
     * return lr;
     */
  }


  // -- Utility methods --

  protected String paramString() {
    StringBuilder buffer = new StringBuilder();
    buffer.append(super.paramString());
    if (label != null) {
      buffer.append(",label=");
      buffer.append(label);
      buffer.append(",align=");
      buffer.append(alignParamString(align));
    }
    return buffer.toString();
  }

  /**
   * Returns a description of alignment for debugging.
   */
  protected String alignParamString(int align) {
    switch (align) {
      case CENTER:
        return "center";
      case NORTH:
        return "north";
      case EAST:
        return "east";
      case SOUTH:
        return "south";
      case WEST:
        return "west";
      case NORTHEAST:
        return "northeast";
      case SOUTHEAST:
        return "southeast";
      case SOUTHWEST:
        return "southwest";
      case NORTHWEST:
        return "northwest";
    }
    return "<unknown>";
  }
}


