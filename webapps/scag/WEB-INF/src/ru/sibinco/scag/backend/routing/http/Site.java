/*
 * Copyright (c) 2006 SibInco Inc. All Rights Reserved.
 */

package ru.sibinco.scag.backend.routing.http;

import ru.sibinco.lib.SibincoException;

/**
 * The <code>Site</code> class represents
 * <p><p/>
 * Date: 06.05.2006
 * Time: 16:51:26
 *
 * @author &lt;a href="mailto:igor@sibinco.ru"&gt;Igor Klimenko&lt;/a&gt;
 */
public class Site {
    private String host;
    private int port;
    private String[] pathLinks;
    private boolean defaultSite;

    public String getId() {
        return host+":"+port;
    }

    public Site() {
    }

    public Site(String host, int port, boolean defaultSite, String[] pathLinks) throws SibincoException {
        if(host == null)
            throw new NullPointerException("Host is null");
        this.host = host;
        this.port = port;
        this.defaultSite = defaultSite;
        this.pathLinks = pathLinks;
    }

    public Site(String host, int port, boolean defaultSite)  throws SibincoException {
        if(host == null)
            throw new NullPointerException("Host is null");
        this.host = host;
        this.port = port;
        this.defaultSite = defaultSite;
    }

    public String getHost() {
        if(host != null)host.trim();
        return host;
    }

    public void setHost(String host) {
        this.host = host;
    }

    public int getPort() {
        return port;
    }

    public void setPort(int port) {
        this.port = port;
    }

    public String[] getPathLinks() {
        return pathLinks;
    }

    public void setPathLinks(String[] pathLinks) {
        this.pathLinks = pathLinks;
    }

    public boolean isDefaultSite() {
        return defaultSite;
    }

    public void setDefaultSite(boolean defaultSite) {
        this.defaultSite = defaultSite;
    }

    public String toString(){
        return "Site{host="+host+", port="+port+", default="+defaultSite+", path="+pathLinks+"}";
    }
}
