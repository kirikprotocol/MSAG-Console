/*
 * Copyright (c) 2006 SibInco Inc. All Rights Reserved.
 */

package ru.sibinco.scag.backend.routing.http;

import org.w3c.dom.Element;
import org.w3c.dom.NodeList;
import ru.sibinco.lib.SibincoException;
import ru.sibinco.lib.backend.util.StringEncoderDecoder;

import java.io.PrintWriter;
import java.util.ArrayList;
import java.util.List;

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
    private String[] masks;

    public HttpSubject(Element subjElement) throws SibincoException {

        name = StringEncoderDecoder.encode(subjElement.getAttribute("id"));
        NodeList maskList = subjElement.getElementsByTagName("address");

        List masksList = new ArrayList();
        for (int i = 0; i < maskList.getLength(); i++) {
            Element maskElem = (Element) maskList.item(i);
            masksList.add(maskElem.getAttribute("value").trim());
        }
        masks = (String[]) masksList.toArray(new String[masksList.size()]);
        masksList.clear();
    }

    public HttpSubject(String name, String[] masksStrings) throws SibincoException {
        if (name == null)
            throw new NullPointerException("HttpSubject id is null ");
        this.name = name;
        this.masks = masksStrings;
    }

    public PrintWriter store(PrintWriter out) {
        out.println("    <subject_def id=\"" + StringEncoderDecoder.encode(getName()) + "\">");
        for (int i = 0; i < masks.length; i++) {
            out.println("        <address  value=\""+  masks[i] + "\"/>");
        }
        out.println("    </subject_def>");
        return out;
    }

    public String getName() {
        return name;
    }

    public void setName(final String name) {
        this.name = name;
    }

    public String[] getMasks() {
        return masks;
    }

    public void setMasks(final String[] masks) {
        this.masks = masks;
    }

    public String getId() {
        return getName();
    }

    public void setId(final String id) {
        setName(id);
    }
}
