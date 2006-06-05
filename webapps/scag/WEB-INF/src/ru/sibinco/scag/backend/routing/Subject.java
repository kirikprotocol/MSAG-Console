/*
 * Copyright (c) 2005 SibInco Inc. All Rights Reserved.
 */
package ru.sibinco.scag.backend.routing;

import ru.sibinco.scag.backend.endpoints.svc.Svc;
import ru.sibinco.scag.backend.endpoints.SmppManager;
import ru.sibinco.scag.backend.endpoints.centers.Center;
import ru.sibinco.lib.backend.route.Mask;
import ru.sibinco.lib.backend.util.xml.Utils;
import ru.sibinco.lib.backend.util.StringEncoderDecoder;
import ru.sibinco.lib.SibincoException;
import org.w3c.dom.Element;
import org.w3c.dom.NodeList;

import java.util.Collection;
import java.io.PrintWriter;

/**
 * The <code>Subject</code> class represents
 * <p><p/>
 * Date: 21.10.2005
 * Time: 12:00:39
 *
 * @author &lt;a href="mailto:igor@sibinco.ru"&gt;Igor Klimenko&lt;/a&gt;
 */
public class Subject {

    private String name = null;
    private Svc svc = null;
    private Center center = null;
    private MaskList masks;
    private String notes = "";


    public Subject(Element subjElement, SmppManager smppManager) throws SibincoException {

        name = StringEncoderDecoder.encode(subjElement.getAttribute("id"));
        NodeList masksList = subjElement.getElementsByTagName("mask");
        masks = new MaskList();
        for (int i = 0; i < masksList.getLength(); i++) {
            Element maskElem = (Element) masksList.item(i);
            masks.add(new Mask(maskElem.getAttribute("value").trim()));
        }
        svc = (Svc) smppManager.getSvcs().get(subjElement.getAttribute("defSme"));
        center = (Center) smppManager.getCenters().get(subjElement.getAttribute("defSme"));
        if (svc == null && center == null)
            throw new SibincoException("Unknown default Sme \"" + subjElement.getAttribute("defSme") + "\" for subject \"" + name + "\"");

        NodeList notesList = subjElement.getElementsByTagName("notes");
        for (int i = 0; i < notesList.getLength(); i++) {
            notes += Utils.getNodeText(notesList.item(i));
        }
    }

    public Subject(String name, Object defaultSme, String[] masksStrings, String notes) throws SibincoException {
        if (name == null)
            throw new NullPointerException("Name is null");
        if (masksStrings == null)
            throw new NullPointerException("Masks is null");
        if (defaultSme == null)
            throw new NullPointerException("DefaultSme is null");
        this.name = name;

        if(defaultSme instanceof Svc){
            this.svc = (Svc)defaultSme;
        }else if(defaultSme instanceof Center){
            this.center = (Center)defaultSme;
        }
        this.masks = new MaskList(masksStrings);
        if (masks.size() == 0)
            throw new SibincoException("Masks is empty");
        this.notes = notes;
    }

    public Subject(String name, Collection masksStrings, Object defaultSme, String notes) throws SibincoException {
        this.notes = notes;
        if (name == null)
            throw new NullPointerException("Name is null");
        if (masksStrings == null)
            throw new NullPointerException("Masks is null");
        if (defaultSme == null)
            throw new NullPointerException("DefaultSme is null");

        this.name = name;
        masks = new MaskList(masksStrings);
        if (masks.size() == 0)
            throw new NullPointerException("Masks is empty");

        if(defaultSme instanceof Svc){
            this.svc = (Svc)defaultSme;
        }else if(defaultSme instanceof Center){
            this.center = (Center)defaultSme;
        }
    }

    public PrintWriter store(PrintWriter out) {
        out.println("  <subject_def id=\"" + StringEncoderDecoder.encode(getName()) + "\" defSme=\"" +
                StringEncoderDecoder.encode(getSvc() == null ? getCenter().getId() : getSvc().getId()) + "\">");
        if (notes != null)
            out.println("    <notes>" + notes + "</notes>");
        getMasks().store(out);
        out.println("  </subject_def>");
        return out;
    }

    public String getId() {
        return getName();
    }

    public void setId(final String id) {
        setName(id);
    }

    public String getName() {
        return name;
    }

    public void setName(final String name) {
        this.name = name;
    }

    public Svc getSvc() {
        return svc;
    }

    public void setSvc(final Svc svc) {
        this.svc = svc;
    }

    public Center getCenter() {
        return center;
    }

    public void setCenter(Center center) {
        this.center = center;
    }

    public MaskList getMasks() {
        return masks;
    }

    public void setMasks(final MaskList masks) {
        this.masks = masks;
    }

    public String getNotes() {
        return notes;
    }

    public void setNotes(final String notes) {
        this.notes = notes;
    }
}
