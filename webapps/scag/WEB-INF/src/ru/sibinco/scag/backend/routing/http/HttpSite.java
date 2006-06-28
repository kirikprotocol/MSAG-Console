/*
 * Copyright (c) 2006 SibInco Inc. All Rights Reserved.
 */

package ru.sibinco.scag.backend.routing.http;

import org.w3c.dom.Element;
import org.w3c.dom.NodeList;
import ru.sibinco.lib.SibincoException;
import ru.sibinco.lib.backend.util.StringEncoderDecoder;

import java.util.*;
import java.io.PrintWriter;

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


    public HttpSite(Element subjElement) throws SibincoException {

        name = StringEncoderDecoder.encode(subjElement.getAttribute("id"));
        NodeList sitekList = subjElement.getElementsByTagName("site");

        for (int i = 0; i < sitekList.getLength(); i++) {
            Site site = new Site();
            List path = new ArrayList();
            Element siteElem = (Element) sitekList.item(i);
            site.setHost(siteElem.getAttribute("host").trim());
            site.setPort(Integer.parseInt(siteElem.getAttribute("port").trim()));
            NodeList pathList = siteElem.getElementsByTagName("path");
            for (int j = 0; j < pathList.getLength(); j++) {
                Element pathElement = (Element) pathList.item(j);
                path.add(pathElement.getAttribute("value").trim());
            }
            site.setPathLinks((String[]) path.toArray(new String[path.size()]));
            sites.put(site.getHost(), site);
        }
    }

    public PrintWriter store(PrintWriter out) {
        out.println("  <site_subject_def id=\"" + StringEncoderDecoder.encode(getName()) + "\">");
        for (Iterator iterator = sites.values().iterator(); iterator.hasNext();) {
            final Site site = (Site) iterator.next();
            out.println("    <site host=\"" + StringEncoderDecoder.encode(site.getHost()) + "\"  port=\"" + site.getPort() + "\" >");
            String[] pathLinks = site.getPathLinks();
            for (int i = 0; i < pathLinks.length; i++) {
                out.println("      <path value=\"" + pathLinks[i] + "\"/>");
            }
            out.println("    </site>");
        }
        out.println("  </site_subject_def>");
        return out;
    }

    public HttpSite(String name, Map sites) throws SibincoException {
        if (name == null)
            throw new NullPointerException("Name is null");
        this.name = name;
        this.sites = sites;
    }

    public HttpSite(String name) throws SibincoException {
        if (name == null)
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
}
