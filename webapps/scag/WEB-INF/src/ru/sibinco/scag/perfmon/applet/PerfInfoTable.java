package ru.sibinco.scag.perfmon.applet;

import ru.sibinco.scag.perfmon.PerfSnap;

import java.awt.*;

public class PerfInfoTable extends Canvas {

    static final int pad = 1;
    static final int grid = 1;
    static int numHeads = 5;
    static final int numRows = 3;
    static final int colorShift = 16;

    static final Color bgColor = SystemColor.control;
    static final Color shadowColor = SystemColor.controlShadow;
    static final Color lightShadowColor = SystemColor.controlLtHighlight;
    static final Color textColor = SystemColor.textText;
    //smpp
    static final Color headsColorSmpp[] = null;
    static {
        try {
            headsColorSmpp[0] = new Color(bgColor.getRed(),              bgColor.getGreen(),              bgColor.getBlue() + colorShift); // accepted
        } catch (Exception e) {
            headsColorSmpp[0] = bgColor;
        }
        try {
            headsColorSmpp[1] = new Color(bgColor.getRed() + colorShift, bgColor.getGreen() + colorShift, bgColor.getBlue() + colorShift); // rejected
        } catch (Exception e) {
            headsColorSmpp[1] = bgColor;
        }
        try {
            headsColorSmpp[2] = new Color(bgColor.getRed(),              bgColor.getGreen() + colorShift, bgColor.getBlue()); // delivered
        } catch (Exception e) {
            headsColorSmpp[2] = bgColor;
        }
        try {
            headsColorSmpp[3] = new Color(bgColor.getRed(),              bgColor.getGreen() + colorShift, bgColor.getBlue() + colorShift); // Gw rejected
        } catch (Exception e) {
            headsColorSmpp[3] = bgColor;
        }
        try {
            headsColorSmpp[4] = new Color(bgColor.getRed() + colorShift, bgColor.getGreen(),              bgColor.getBlue()); // Failed
        } catch (Exception e) {
            headsColorSmpp[4] = bgColor;
        }
//            new Color(bgColor.getRed(),              bgColor.getGreen(),              bgColor.getBlue() + colorShift); // accepted
//            new Color(bgColor.getRed() + colorShift, bgColor.getGreen() + colorShift, bgColor.getBlue() + colorShift), // rejected
//            new Color(bgColor.getRed(),              bgColor.getGreen() + colorShift, bgColor.getBlue()), // delivered
//            new Color(bgColor.getRed(),              bgColor.getGreen() + colorShift, bgColor.getBlue() + colorShift), // Gw rejected
//            new Color(bgColor.getRed() + colorShift, bgColor.getGreen(),              bgColor.getBlue()), // Failed
    };
    //http
    static final Color headsColorHttp[] = null;
    static{
        try {
            headsColorSmpp[0] = new Color(bgColor.getRed(),              bgColor.getGreen(),              bgColor.getBlue() + colorShift); // request
        } catch (Exception e) {
            headsColorSmpp[0] = bgColor;
        }
        try {
            headsColorSmpp[1] = new Color(bgColor.getRed() + colorShift, bgColor.getGreen() + colorShift, bgColor.getBlue() + colorShift); // request rejected
        } catch (Exception e) {
            headsColorSmpp[1] = bgColor;
        }
        try {
            headsColorSmpp[2] = new Color(bgColor.getRed(),              bgColor.getGreen() + colorShift, bgColor.getBlue() + colorShift); // response
        } catch (Exception e) {
            headsColorSmpp[2] = bgColor;
        }
        try {
            headsColorSmpp[3] = new Color(bgColor.getRed() + colorShift, bgColor.getGreen() + colorShift, bgColor.getBlue()); // response rejected
        } catch (Exception e) {
            headsColorSmpp[3] = bgColor;
        }
        try {
            headsColorSmpp[4] = new Color(bgColor.getRed(),              bgColor.getGreen() + colorShift, bgColor.getBlue()); // deliveredHTTP
        } catch (Exception e) {
            headsColorSmpp[4] = bgColor;
        }
        try {
            headsColorSmpp[5] = new Color(bgColor.getRed() + colorShift, bgColor.getGreen(),              bgColor.getBlue()); // FailedHTTP
        } catch (Exception e) {
            headsColorSmpp[5] = bgColor;
        }
//            new Color(bgColor.getRed(),              bgColor.getGreen(),              bgColor.getBlue() + colorShift), // request
//            new Color(bgColor.getRed() + colorShift, bgColor.getGreen() + colorShift, bgColor.getBlue() + colorShift), // request rejected
//            new Color(bgColor.getRed(),              bgColor.getGreen() + colorShift, bgColor.getBlue() + colorShift), // response
//            new Color(bgColor.getRed() + colorShift, bgColor.getGreen() + colorShift, bgColor.getBlue()), // response rejected
//            new Color(bgColor.getRed(),              bgColor.getGreen() + colorShift, bgColor.getBlue()), // deliveredHTTP
//            new Color(bgColor.getRed() + colorShift, bgColor.getGreen(),              bgColor.getBlue()) // FailedHTTP
    };

    static Color headsColor[];
    PerfSnap snap;
    Dimension prefSize;
    Image offscreen;
    int columnWidth;

    String heads[];
    String rows[];

    public PerfInfoTable(PerfSnap snap) {
        this.snap = new PerfSnap(snap);
        prefSize = new Dimension(100, 0);
        updateColumns();
    }

    public synchronized void setSnap(PerfSnap snap) {
        this.snap = new PerfSnap(snap);
        updateColumns();
        this.repaint();
    }

    public void updateColumns() {
      numHeads = (PerfMon.statMode.equals(PerfMon.smppStatMode)?5:6);
      headsColor = (PerfMon.statMode.equals(PerfMon.smppStatMode)?headsColorSmpp:headsColorHttp);
      heads = new String[numHeads];
      for (int i = 0; i < numHeads; i++) {
          heads[i] = PerfMon.localeText.getString("ptabh."+PerfMon.statMode+"."+i);
      }

      rows = new String[numRows];
      for (int i = 0; i < numRows; i++) {
          rows[i] = PerfMon.localeText.getString("ptabr." + i);
      }
      invalidate();
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
            if (PerfMon.statMode.equals(PerfMon.smppStatMode)) {
              drawCounters(g, snap.smppSnap.last, sz, fm, 1);
              drawCounters(g, snap.smppSnap.avg, sz, fm, 2);
              drawCounters(g, snap.smppSnap.total, sz, fm, 3);
            } else {
              drawCounters(g, snap.httpSnap.last, sz, fm, 1);
              drawCounters(g, snap.httpSnap.avg, sz, fm, 2);
              drawCounters(g, snap.httpSnap.total, sz, fm, 3);
            }
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
        for (int i = 0; i < counters.length; i++) {
            char counter[] = String.valueOf(counters[i]).toCharArray();
            g.drawChars(counter, 0, counter.length,
                    (i + 2) * x - fm.charsWidth(counter, 0, counter.length) - pad,
                    (col) * y + fm.getAscent() + pad);
        }
    }

    public void update(Graphics gg) {
        paint(gg);
    }

}
