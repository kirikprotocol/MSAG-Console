/*
 * Copyright (c) 2005 SibInco Inc. All Rights Reserved.
 */
package ru.sibinco.scag.svcmon.applet;

import ru.sibinco.lib.backend.applet.LabelGroup;

import java.awt.*;

/**
 * The <code>ScreenSplitter</code> class represents
 * <p><p/>
 * Date: 09.12.2005
 * Time: 17:10:19
 *
 * @author &lt;a href="mailto:igor@sibinco.ru"&gt;Igor Klimenko&lt;/a&gt;
 */
public class ScreenSplitter extends Container {

    private static final Color colorBackground = SystemColor.control;
    private LabelGroup smppTopLabel;
    Image offscreen = null;

    int split = 0;
    int shiftStep;

    public ScreenSplitter(LabelGroup smppTopLabel) {
        this.smppTopLabel = smppTopLabel;
        add(this.smppTopLabel);
    }

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
            //ToDo
        }
        //System.out.println("Invalidate: " + sz.width + "/" + sz.height + " spl=" + split);
        smppTopLabel.setSize(sz.width, sz.height - split);
        smppTopLabel.setLocation(0, 0);
        smppTopLabel.invalidate();
    }

    public void shiftUp() {

        Dimension sz = getSize();
        Dimension msz = smppTopLabel.getMinimumSize();
        if (sz.height - split < msz.height + 6 * shiftStep) return;
        split += shiftStep;
        //System.out.println("Split = " + split);
        invalidate();
        validate();
        repaint();
    }

    public void shiftDown() {
        // ToDo
        //System.out.println("Split = " + split);
        invalidate();
        validate();
        repaint();
    }

    public void update(Graphics gg) {
        paint(gg);
    }

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
