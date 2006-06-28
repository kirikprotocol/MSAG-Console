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

    public String getId() {
        return host;
    }

    public Site() {
    }

    public Site(String host, int port, String[] pathLinks)  throws SibincoException {
        if(host == null)
            throw new NullPointerException("Host is null");
        this.host = host;
        this.port = port;
        this.pathLinks = pathLinks;
    }

    public Site(String host, int port)  throws SibincoException {
        if(host == null)
            throw new NullPointerException("Host is null");
        this.host = host;
        this.port = port;
    }

    public String getHost() {
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
}
