/*
 * Copyright (c) 2006 SibInco Inc. All Rights Reserved.
 */

package ru.sibinco.scag.backend.routing.http;

import ru.sibinco.lib.backend.route.Mask;
import ru.sibinco.lib.backend.util.StringEncoderDecoder;
import ru.sibinco.lib.SibincoException;
import org.w3c.dom.Element;
import org.w3c.dom.NodeList;

import java.util.*;
import java.io.PrintWriter;


/**
 * The <code>Abonent</code> class represents
 * <p><p/>
 * <p/>
 * Date: 05.06.2006
 * Time: 13:57:08
 *
 * @author &lt;a href="mailto:igor@sibinco.ru"&gt;Igor Klimenko&lt;/a&gt;
 */
public class Abonent {

    private Map subjects = Collections.synchronizedMap(new HashMap());
    private Map address = Collections.synchronizedMap(new HashMap());

    public Abonent(Map subjects) {
        if (subjects == null)
            throw new NullPointerException("Http subject is null");
        this.subjects = subjects;
    }

    public Abonent() {
    }

    public Abonent(Element abonElement, Map subjs) throws SibincoException {
        NodeList listSubj = abonElement.getElementsByTagName("subject");
        NodeList listAddr = abonElement.getElementsByTagName("address");

        for (int i = 0; i < listSubj.getLength(); i++) {
            Element subjElem = (Element) listSubj.item(i);
            HttpSubject subject = (HttpSubject) subjs.get(subjElem.getAttribute("id"));
            if (subject == null) {
                throw new NullPointerException("Http subject is unknown: " + subjElem.getAttribute("id"));
            } else {
                subjects.put(subject.getName(), subject);
            }
        }

        for (int i = 0; i < listAddr.getLength(); i++) {
            Element addrElem = (Element) listAddr.item(i);
            Mask addressMask = new Mask(addrElem.getAttribute("value"));
            address.put(addressMask.getMask(), addressMask);
        }
    }

    public PrintWriter store(PrintWriter out) {
        out.println("    <abonents>");
        for (Iterator i = subjects.values().iterator(); i.hasNext();) {
            final HttpSubject subject = (HttpSubject) i.next();
            out.println("      <subject id=\"" + StringEncoderDecoder.encode(subject.getName()) + "\"/>");
        }
        for (Iterator i = address.values().iterator(); i.hasNext();) {
            final Mask addressMask = (Mask) i.next();
            out.println("      <address value=\"" + StringEncoderDecoder.encode(addressMask.getNormalizedMask()) + "\"/>");
        }
        out.println("    </abonents>");
        return out;
    }

    public Map getSubjects() {
        return subjects;
    }

    public Map getAddress() {
        return address;
    }

    public String[] getSubjectsAsStr() {
        List list = new ArrayList();
        for (Iterator i = subjects.values().iterator(); i.hasNext();) {
            final HttpSubject subject = (HttpSubject) i.next();
            list.add(subject.getName());
        }
        return (String[]) list.toArray(new String[list.size()]);
    }

    public String[] getAddressAsStr() {
        List list = new ArrayList();
        for (Iterator i = address.values().iterator(); i.hasNext();) {
            final Mask addressMask = (Mask) i.next();
            list.add(addressMask.getMask());
        }
        return (String[]) list.toArray(new String[list.size()]);
    }
}
