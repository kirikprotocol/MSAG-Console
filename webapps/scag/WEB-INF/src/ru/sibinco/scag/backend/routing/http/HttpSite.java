/*
 * Copyright (c) 2006 SibInco Inc. All Rights Reserved.
 */

package ru.sibinco.scag.backend.routing.http;

import org.w3c.dom.Element;
import org.w3c.dom.NodeList;
import ru.sibinco.lib.SibincoException;
import ru.sibinco.lib.backend.util.StringEncoderDecoder;

import java.util.Map;
import java.util.HashMap;
import java.util.ArrayList;
import java.util.List;

/**
 * The <code>HttpSite</code> class represents
 * <p><p/>
 * Date: 19.05.2006
 * Time: 14:04:57
 *
 * @author &lt;a href="mailto:igor@sibinco.ru"&gt;Igor Klimenko&lt;/a&gt;
 */
public class HttpSite {

    private String name;
    private Map sites = new HashMap();
    private String notes = "";

    public HttpSite(Element subjElement) throws SibincoException {

        name = StringEncoderDecoder.encode(subjElement.getAttribute("id"));
        NodeList maskList = subjElement.getElementsByTagName("site");

        for (int i = 0; i < maskList.getLength(); i++) {
            Site site = new Site();
            List path = new ArrayList();
            Element maskElem = (Element) maskList.item(i);
            site.setHost(maskElem.getAttribute("host").trim());
            site.setPort(Integer.parseInt(maskElem.getAttribute("port").trim()));
            NodeList pathList = maskElem.getElementsByTagName("path");
            for (int j = 0; j < pathList.getLength(); j++) {
                Element pathElement = (Element) pathList.item(j);
                path.add(pathElement.getAttribute("value").trim());
            }
            site.setPathLinks((String[]) path.toArray(new String[path.size()]));
            sites.put(site.getHost(), site);
        }
    }

    public HttpSite(String name, Map sites) throws SibincoException {
        if(name == null)
            throw new NullPointerException("Name is null");
        this.name = name;
        this.sites = sites;
    }

    public HttpSite(String name) throws SibincoException {
        if(name == null)
            throw new NullPointerException("Name is null");
        this.name = name;
        this.sites = new HashMap();
    }

    public String getName() {
        return name;
    }

    public void setName(String name) {
        this.name = name;
    }

    public String getId() {
        return getName();
    }

    public void setId(final String id) {
        setName(id);
    }

    public Map getSites() {
        return sites;
    }

    public String getNotes() {
        return notes;
    }

    public void setNotes(String notes) {
        this.notes = notes;
    }
}
