/*
 * Copyright (c) 2006 SibInco Inc. All Rights Reserved.
 */

package ru.sibinco.scag.backend.routing.http;

import org.apache.log4j.Logger;
import org.w3c.dom.Element;
import org.w3c.dom.NodeList;
import ru.sibinco.lib.SibincoException;
import ru.sibinco.lib.backend.util.StringEncoderDecoder;

import java.io.PrintWriter;
import java.util.*;

/**
 * The <code>HttpSite</code> class represents
 */
public class HttpSite {

    private static Logger log = Logger.getLogger(HttpSite.class);

    private String name;
    private Map<String, Site> sites = new HashMap<String,Site>();


    public HttpSite(Element subjElement) throws SibincoException {
        name = StringEncoderDecoder.encode(subjElement.getAttribute("id"));
        log.debug("Try to initialize http site: "+name);
        NodeList sitekList = subjElement.getElementsByTagName("site");

        for (int i = 0; i < sitekList.getLength(); i++) {
            Site site = new Site();
            List<String> path = new ArrayList<String>();
            Element siteElem = (Element) sitekList.item(i);
            site.setHost(siteElem.getAttribute("host").trim());
            site.setPort(Integer.parseInt(siteElem.getAttribute("port").trim()));
            site.setDefaultSite(siteElem.getAttribute("default").equals("true"));
            NodeList pathList = siteElem.getElementsByTagName("path");
            for (int j = 0; j < pathList.getLength(); j++) {
                Element pathElement = (Element) pathList.item(j);
                path.add(pathElement.getAttribute("value").trim());
            }
            site.setPathLinks(path.toArray(new String[path.size()]));
            String id = site.getId();
            sites.put(id, site);
            log.debug("init "+site);
        }
        log.debug("Initialize http site: "+name);
    }

    public PrintWriter store(PrintWriter out) {
        out.println("  <site_subject_def id=\"" + StringEncoderDecoder.encode(getName()) + "\">");
        for (final Site site : sites.values()) {
            out.println("    <site host=\"" + StringEncoderDecoder.encode(site.getHost()) + "\"  port=\"" + site.getPort() + "\" default=\"" + site.isDefaultSite() + "\">");
            String[] pathLinks = site.getPathLinks();
            for (String pathLink : pathLinks) out.println("      <path value=\"" + pathLink + "\"/>");
            out.println("    </site>");
        }
        out.println("  </site_subject_def>");
        return out;
    }

    /*public HttpSite(String name, Map<String, Site> sites) {
        if (name == null)
            throw new NullPointerException("Name is null");
        this.name = name;
        this.sites = sites;
    }*/

    public HttpSite(String name)  {
        if (name == null)
            throw new NullPointerException("Name is null");
        this.name = name;
        this.sites = new HashMap<String, Site>();
    }

    public Site[] getArraySite() {
        List<Site> siteList = new ArrayList<Site>();
        for (final Site site : sites.values()) siteList.add(site);
        return siteList.toArray(new Site[siteList.size()]);
    }

    public String[] getSiteAsStr() {
        List<String> siteList = new ArrayList<String>();
        for (final Site site : sites.values()) siteList.add(site.getHost());
        return siteList.toArray(new String[siteList.size()]);
    }

    public String getName() {
        return name != null ? name.trim() : null;
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

    public Map<String,Site> getSites() {
        return sites;
    }
}
