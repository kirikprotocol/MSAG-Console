/*
 * Copyright (c) 2005 SibInco Inc. All Rights Reserved.
 */
package ru.sibinco.scag.backend.routing;

import ru.sibinco.lib.backend.route.Mask;
import ru.sibinco.lib.backend.route.MaskList;
import ru.sibinco.lib.backend.util.StringEncoderDecoder;
import ru.sibinco.lib.SibincoException;
import org.w3c.dom.Element;
import org.w3c.dom.NodeList;

import java.util.Map;
import java.io.PrintWriter;

/**
 * The <code>Source</code> class represents
 * <p><p/>
 * Date: 21.10.2005
 * Time: 12:00:08
 *
 * @author &lt;a href="mailto:igor@sibinco.ru"&gt;Igor Klimenko&lt;/a&gt;
 */
public class Source {

    Subject subject = null;
    Mask mask = null;

    public Source(Subject subject) {
        if (subject == null)
            throw new NullPointerException("Subject is null");
        this.subject = subject;
    }

    public Source(Element srcElement, Map subjects) throws SibincoException {
        NodeList list = srcElement.getElementsByTagName("subject");
        if (list.getLength() > 0) {
            Element subjElem = (Element) list.item(0);
            subject = (Subject) subjects.get(subjElem.getAttribute("id"));
            if (subject == null)
                throw new NullPointerException("Subject is unknown: " + subjElem.getAttribute("id"));
        } else {
            list = srcElement.getElementsByTagName("mask");
            Element maskElem = (Element) list.item(0);
            mask = new Mask(maskElem.getAttribute("value"));
        }
    }

    public PrintWriter store(PrintWriter out) {
        out.println("    <source>");
        if (isSubject())
            out.println("      <subject id=\"" + StringEncoderDecoder.encode(subject.getName()) + "\"/>");
        else
            out.println("      <mask value=\"" + StringEncoderDecoder.encode(mask.getNormalizedMask()) + "\"/>");
        out.println("    </source>");
        return out;
    }

    public Source(Mask mask) {
        if (mask == null)
            throw new NullPointerException("Mask is null");
        this.mask = mask;
    }

    public boolean isSubject() {
        return subject != null;
    }

    public String getName() {
        if (isSubject())
            return subject.getName();
        else
            return mask.getMask();
    }

    public MaskList getMask() {
        if (isSubject())
            return subject.getMasks();
        else
            return new MaskList(mask);
    }

    public String getDefaultSmeId() {
        if (subject != null)
            return subject.getDefaultSme().getId();
        else
            return null;
    }


}



