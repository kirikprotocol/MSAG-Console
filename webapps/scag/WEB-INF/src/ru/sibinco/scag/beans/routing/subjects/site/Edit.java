/*
 * Copyright (c) 2006 SibInco Inc. All Rights Reserved.
 */

package ru.sibinco.scag.beans.routing.subjects.site;

import ru.sibinco.scag.beans.EditBean;
import ru.sibinco.scag.beans.SCAGJspException;
import ru.sibinco.scag.beans.CancelChildException;
import ru.sibinco.scag.backend.SCAGAppContext;
import ru.sibinco.scag.backend.transport.Transport;
import ru.sibinco.scag.backend.status.StatMessage;
import ru.sibinco.scag.backend.status.StatusManager;
import ru.sibinco.scag.backend.routing.http.HttpSite;
import ru.sibinco.scag.backend.routing.http.Site;
import ru.sibinco.scag.backend.routing.http.HttpRoutingManager;
import ru.sibinco.scag.Constants;
import ru.sibinco.lib.backend.util.Functions;
import ru.sibinco.lib.SibincoException;

import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;
import java.util.Map;

/**
 * The <code>Edit</code> class represents
 * <p><p/>
 * Date: 31.05.2006
 * Time: 12:33:20
 *
 * @author &lt;a href="mailto:igor@sibinco.ru"&gt;Igor Klimenko&lt;/a&gt;
 */
public class Edit extends EditBean {

    private String id;
    private String path = "";
    private String host;
    private int port = 0;

    private String parentId;
    private String[] pathLinks = new String[0];


    public String getId() {
        return id;
    }

    public void process(final HttpServletRequest request, final HttpServletResponse response) throws SCAGJspException {

        path = request.getContextPath();
        appContext = getAppContext();

        if (appContext == null) {
            appContext = (SCAGAppContext) request.getAttribute("appContext");
        }

        if (getMbCancel() != null) {
            throw new CancelChildException(path +
                    "/routing/subjects/edit.jsp?subjectType=" + HttpRoutingManager.HTTP_SITE_TYPE
                    + "&transportId=" + Transport.HTTP_TRANSPORT_ID + "&httpSiteId=" + getParentId());
        }

        if (getEditId() != null)
            id = getEditId();
        if (getMbSave() != null) {
            super.process(request, response);
            save();
        }
        load(id);
        super.process(request, response);
    }

    protected void load(final String loadId) throws SCAGJspException {
        if (!isAdd()) {
            final HttpSite httpSite = (HttpSite) appContext.getHttpRoutingManager().getSites().get(getParentId());
            Site site = (Site) httpSite.getSites().get(loadId);
            host = site.getHost();
            port = site.getPort();
            pathLinks = site.getPathLinks();
        }
    }

    protected void save() throws SCAGJspException {
        String messagetxt = "";
        pathLinks = Functions.trimStrings(pathLinks);
        final HttpSite httpSite = (HttpSite) appContext.getHttpRoutingManager().getSites().get(getParentId());
        final Map sites = httpSite.getSites();
        if (isAdd()) {
            if (sites.containsKey(host))
                throw new SCAGJspException(Constants.errors.routing.sites.HTTP_HOST_ALREADY_EXISTS, host);

            try {
                sites.put(host, new Site(host, port, pathLinks));
                messagetxt = "Added new host: '" + host + "'.";
            } catch (SibincoException e) {
                logger.debug("Could not create new site", e);
                throw new SCAGJspException(Constants.errors.routing.sites.COULD_NOT_CREATE, e);
            }
        } else {
            if (!getEditId().equals(host)) {
                if (sites.containsKey(host))
                    throw new SCAGJspException(Constants.errors.routing.sites.HTTP_HOST_ALREADY_EXISTS, host);
                sites.remove(getEditId());
                try {
                    sites.put(host, new Site(host, port, pathLinks));
                    messagetxt = "Added new host: '" + host + "'.";
                } catch (SibincoException e) {
                    logger.debug("Could not create new site", e);
                    throw new SCAGJspException(Constants.errors.routing.sites.COULD_NOT_CREATE, e);
                }
            } else {
                final Site site = (Site) sites.get(getEditId());
                site.setHost(host);
                site.setPort(port);
                site.setPathLinks(pathLinks);
                messagetxt = "Changed site: '" + host + "'.";
            }
        }
        StatMessage message = new StatMessage(getLoginedPrincipal().getName(), "Subject", messagetxt);
        appContext.getHttpRoutingManager().setRoutesChanged(true);
        appContext.getHttpRoutingManager().addStatMessages(message);
        StatusManager.getInstance().addStatMessages(message);
        throw new CancelChildException(path +
                "/routing/subjects/edit.jsp?subjectType=" + HttpRoutingManager.HTTP_SITE_TYPE
                + "&transportId=" + Transport.HTTP_TRANSPORT_ID + "&httpSiteId=" + getParentId());
        //throw new DoneException();

    }

    public String[] getPathLinks() {
        return pathLinks;
    }

    public void setPathLinks(String[] pathLinks) {
        this.pathLinks = pathLinks;
    }

    public void setId(String id) {
        this.id = id;
    }

    public String getPath() {
        return path;
    }

    public void setPath(String path) {
        this.path = path;
    }

    public String getParentId() {
        return parentId;
    }

    public void setParentId(String parentId) {
        this.parentId = parentId;
    }

    public String getHost() {
        return host;
    }

    public void setHost(final String host) {
        this.host = host;
    }

    public int getPort() {
        return port;
    }

    public void setPort(final int port) {
        this.port = port;
    }
}
