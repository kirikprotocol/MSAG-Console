/*
 * Copyright (c) 2006 SibInco Inc. All Rights Reserved.
 */

package ru.sibinco.scag.backend.routing.http;

import ru.sibinco.lib.backend.route.Mask;
import ru.sibinco.lib.backend.util.StringEncoderDecoder;
import ru.sibinco.lib.SibincoException;
import ru.sibinco.scag.backend.routing.http.placement.AbonentsPlacement;
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
    private List abonUsrPlace = new ArrayList();
    private int ton = 1;
    private int npi = 1;

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
        NodeList listAddrPref = abonElement.getElementsByTagName("address_prefix");

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
        if (listAddrPref != null) {
            Element addrPrefElement = (Element) listAddrPref.item(0);

            if (addrPrefElement != null) {
                setTon(Integer.parseInt(addrPrefElement.getAttribute("ton")));
                setNpi(Integer.parseInt(addrPrefElement.getAttribute("npi")));
            }
        }

        NodeList placementList = abonElement.getElementsByTagName("placement");
        if (placementList != null) {
            Element usrElement = (Element) placementList.item(0);
            if (usrElement != null) {
                NodeList usrPlaceList = usrElement.getElementsByTagName("usr_place");
                if (usrPlaceList != null) {
                    for (int i = 0; i < usrPlaceList.getLength(); i++) {
                        Element usrPlaceElement = (Element) usrPlaceList.item(i);
                        final AbonentsPlacement abonentsPlacement = new AbonentsPlacement(
                                usrPlaceElement.getAttribute("type"),
                                usrPlaceElement.getAttribute("name"),
                                Integer.parseInt(usrPlaceElement.getAttribute("priority")));
                        abonUsrPlace.add(abonentsPlacement);
                    }
                }
            }
        }
    }

    public PrintWriter store(PrintWriter out) {
        out.println("    <abonents>");
        out.println();
        out.println("        <address_prefix ton=\"" + getTon() + "\" npi=\"" + getNpi() + "\"/>");
        out.println();
        for (Iterator i = subjects.values().iterator(); i.hasNext();) {
            final HttpSubject subject = (HttpSubject) i.next();
            out.println("        <subject id=\"" + StringEncoderDecoder.encode(subject.getName()) + "\"/>");
        }
        out.println();
        for (Iterator i = address.values().iterator(); i.hasNext();) {
            final Mask addressMask = (Mask) i.next();
            out.println("        <address value=\"" + StringEncoderDecoder.encode(addressMask.getNormalizedMask()) + "\"/>");
        }
        out.println();
        out.println("        <placement>");
        for (Iterator i = abonUsrPlace.iterator(); i.hasNext();) {
            final AbonentsPlacement placement = (AbonentsPlacement) i.next();
            out.println(new StringBuffer().append("            <usr_place type=\"").
                    append(StringEncoderDecoder.encode(placement.getType())).append("\" ").
                    append(" name=\"").append(StringEncoderDecoder.encode(placement.getName())).
                    append("\" ").append(" priority=\"").append(placement.getPriority()).
                    append("\" />").toString());
        }
        out.println("        </placement>");
        out.println();
        out.println("    </abonents>");
        out.println();
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

    public int getTon() {
        return ton;
    }

    public void setTon(int ton) {
        this.ton = ton;
    }

    public int getNpi() {
        return npi;
    }

    public void setNpi(int npi) {
        this.npi = npi;
    }

    public List getAbonUsrPlace() {
        return abonUsrPlace;
    }
}
