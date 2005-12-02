/*
 * Copyright (c) 2005 SibInco Inc. All Rights Reserved.
 */
package ru.sibinco.scag.backend.routing;

import ru.sibinco.scag.backend.endpoints.svc.Svc;
import ru.sibinco.scag.backend.endpoints.SmppManager;
import ru.sibinco.lib.backend.route.MaskList;
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
    private Svc defaultSme = null;
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
        defaultSme = (Svc) smppManager.getSvcs().get(subjElement.getAttribute("defSme"));
        if (defaultSme == null)
            throw new SibincoException("Unknown default Sme \"" + subjElement.getAttribute("defSme") + "\" for subject \"" + name + "\"");

        NodeList notesList = subjElement.getElementsByTagName("notes");
        for (int i = 0; i < notesList.getLength(); i++) {
            notes += Utils.getNodeText(notesList.item(i));
        }
    }

    public Subject(String name, Svc defaultSme, String[] masksStrings, String notes) throws SibincoException {
        if (name == null)
            throw new NullPointerException("Name is null");
        if (masksStrings == null)
            throw new NullPointerException("Masks is null");
        if (defaultSme == null)
            throw new NullPointerException("DefaultSme is null");
        this.name = name;

        this.defaultSme = defaultSme;
        this.masks = new MaskList(masksStrings);
        if (masks.size() == 0)
            throw new SibincoException("Masks is empty");
        this.notes = notes;
    }

    public Subject(String name, Collection masksStrings, Svc defaultSme, String notes) throws SibincoException {
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
        this.defaultSme = defaultSme;
    }

    public PrintWriter store(PrintWriter out) {
        out.println("  <subject_def id=\"" + StringEncoderDecoder.encode(getName()) + "\" defSme=\"" + StringEncoderDecoder.encode(getDefaultSme().getId()) + "\">");
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

    public Svc getDefaultSme() {
        return defaultSme;
    }

    public void setDefaultSme(final Svc defaultSme) {
        this.defaultSme = defaultSme;
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
