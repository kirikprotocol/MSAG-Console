/*
 * Copyright (c) 2006 SibInco Inc. All Rights Reserved.
 */

package ru.sibinco.scag.backend.routing.http;

import ru.sibinco.lib.backend.route.Mask;
import ru.sibinco.lib.backend.util.StringEncoderDecoder;
import ru.sibinco.lib.SibincoException;
import ru.sibinco.scag.backend.routing.MaskList;
import org.w3c.dom.Element;
import org.w3c.dom.NodeList;

import java.util.Collection;
import java.io.PrintWriter;

/**
 * The <code>HttpSubject</code> class represents
 * <p><p/>
 * Date: 06.05.2006
 * Time: 16:51:26
 *
 * @author &lt;a href="mailto:igor@sibinco.ru"&gt;Igor Klimenko&lt;/a&gt;
 */
public class HttpSubject{

    private String name = null;
    private MaskList masks;

    public HttpSubject(Element subjElement) throws SibincoException {

        name = StringEncoderDecoder.encode(subjElement.getAttribute("id"));
        NodeList maskList = subjElement.getElementsByTagName("address");
        masks = new MaskList();
        for (int i = 0; i < maskList.getLength(); i++) {
            Element maskElem = (Element) maskList.item(i);
            masks.add(new Mask(maskElem.getAttribute("value").trim()));
        }
    }

    public HttpSubject(String name, String[] masksStrings) throws SibincoException {
        if (name == null)
            throw new NullPointerException("HttpSubject id is null ");
        this.name = name;
        this.masks = new MaskList(masksStrings);
    }

    public HttpSubject(String name, Collection masksStrings) throws SibincoException {
        if (name == null)
            throw new NullPointerException("HttpSubject id is null ");
        this.name = name;
        this.masks = new MaskList(masksStrings);
    }

    public PrintWriter store(PrintWriter out) {
        out.println("<subject_def id=\"" + StringEncoderDecoder.encode(getName()) + "\">");
        getMasks().store(out, "address");
        out.println("  </subject_def>");
        return out;
    }

    public String getName() {
        return name;
    }

    public void setName(final String name) {
        this.name = name;
    }

    public MaskList getMasks() {
        return masks;
    }

    public void setMasks(final MaskList masks) {
        this.masks = masks;
    }

    public String getId() {
        return getName();
    }

    public void setId(final String id) {
        setName(id);
    }
}
