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
 * <p><p/>
 * Date: 19.05.2006
 * Time: 14:04:57
 *
 * @author &lt;a href="mailto:igor@sibinco.ru"&gt;Igor Klimenko&lt;/a&gt;
 */
public class HttpSite {

    private static Logger log = Logger.getLogger(HttpSite.class);

    private String name;
    private Map sites = new HashMap();


    public HttpSite(Element subjElement) throws SibincoException {
        name = StringEncoderDecoder.encode(subjElement.getAttribute("id"));
        log.debug("Try to initialize http site: "+name);
        NodeList sitekList = subjElement.getElementsByTagName("site");

        for (int i = 0; i < sitekList.getLength(); i++) {
            Site site = new Site();
            List path = new ArrayList();
            Element siteElem = (Element) sitekList.item(i);
            site.setHost(siteElem.getAttribute("host").trim());
            site.setPort(Integer.parseInt(siteElem.getAttribute("port").trim()));
            site.setDefaultSite(siteElem.getAttribute("default").equals("true"));
            NodeList pathList = siteElem.getElementsByTagName("path");
            for (int j = 0; j < pathList.getLength(); j++) {
                Element pathElement = (Element) pathList.item(j);
                path.add(pathElement.getAttribute("value").trim());
            }
            site.setPathLinks((String[]) path.toArray(new String[path.size()]));
            String id = site.getId();
            sites.put(id, site);
            log.debug("init "+site);
        }
        log.debug("Initialize http site: "+name);
    }

    public PrintWriter store(PrintWriter out) {
        out.println("  <site_subject_def id=\"" + StringEncoderDecoder.encode(getName()) + "\">");
        for (Iterator iterator = sites.values().iterator(); iterator.hasNext();) {
            final Site site = (Site) iterator.next();
            out.println("    <site host=\"" + StringEncoderDecoder.encode(site.getHost()) + "\"  port=\"" + site.getPort() + "\" default=\""+site.isDefaultSite()+"\">");
            String[] pathLinks = site.getPathLinks();
            for (int i = 0; i < pathLinks.length; i++) {
                out.println("      <path value=\"" + pathLinks[i] + "\"/>");
            }
            out.println("    </site>");
        }
        out.println("  </site_subject_def>");
        return out;
    }

    public HttpSite(String name, Map sites) {
        if (name == null)
            throw new NullPointerException("Name is null");
        this.name = name;
        this.sites = sites;
    }

    public HttpSite(String name)  {
        if (name == null)
            throw new NullPointerException("Name is null");
        this.name = name;
        this.sites = new HashMap();
    }

    public Site[] getArraySite() {
        List siteList = new ArrayList();
        for (Iterator i = sites.values().iterator(); i.hasNext();) {
            final Site site = (Site) i.next();
            siteList.add(site);
        }
        return (Site[]) siteList.toArray(new Site[siteList.size()]);
    }

    public String[] getSiteAsStr() {
        List siteList = new ArrayList();
        for (Iterator i = sites.values().iterator(); i.hasNext();) {
            final Site site = (Site) i.next();
            siteList.add(site.getHost());
        }
        return (String[]) siteList.toArray(new String[siteList.size()]);
    }

    public String getName() {
        if(name != null)name.trim();
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
