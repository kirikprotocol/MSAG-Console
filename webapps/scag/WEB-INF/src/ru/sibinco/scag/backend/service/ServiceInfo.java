/*
 * Copyright (c) 2006 SibInco Inc. All Rights Reserved.
 */

package ru.sibinco.scag.backend.service;

import ru.sibinco.scag.backend.endpoints.svc.Svc;
import ru.sibinco.scag.backend.endpoints.SmppManager;
import ru.sibinco.lib.SibincoException;

import java.io.File;
import java.util.Map;
import java.util.HashMap;

import org.w3c.dom.Element;
import org.w3c.dom.NodeList;

/**
 * The <code>ServiceInfo</code> class represents
 * <p><p/>
 * Date: 18.01.2006
 * Time: 15:37:28
 *
 * @author &lt;a href="mailto:igor@sibinco.ru"&gt;Igor Klimenko&lt;/a&gt;
 */
public class ServiceInfo {

    public static final byte STATUS_RUNNING = 0;
    public static final byte STATUS_STARTING = 1;
    public static final byte STATUS_STOPPING = 2;
    public static final byte STATUS_STOPPED = 3;
    public static final byte STATUS_UNKNOWN = 4;

    protected String id = "";
    protected String host = "";
    protected String args = "";
    protected long pid = 0;
    protected Svc svc = null;
    protected byte status = STATUS_UNKNOWN;
    private File serviceFolder;
    private boolean autostart;
    protected Map components = new HashMap();

    public ServiceInfo(final Element serviceElement, final String serviceHost,
                       final SmppManager smppManager, final String daemonServicesFolder) throws SibincoException {
        host = serviceHost;
        id = serviceElement.getAttribute("id");
        this.autostart = "true".equals(serviceElement.getAttribute("autostart"));
        args = serviceElement.getAttribute("args");

        if ("".equals(id)) {
            throw new SibincoException("service name or services system id not specified in response");
        }

        svc = (Svc) smppManager.getSvcs().get(id);
        setStatusStr(serviceElement.getAttribute("status"));
        final String pidStr = serviceElement.getAttribute("pid");
        this.pid = null != pidStr && 0 < pidStr.length() ? Long.decode(pidStr).longValue() : 0;
        this.serviceFolder = new File(daemonServicesFolder, id);
    }

    public ServiceInfo(final String id, String host, final String serviceFolder,
                       final String args, final boolean autostart, final long pid,
                       final Svc svc, final byte status) {
        this.id = id;
        this.host = host;
        this.serviceFolder = new File(serviceFolder);
        this.args = args;
        this.autostart = autostart;
        this.pid = pid;
        this.svc = svc;
        this.status = status;
    }

    public ServiceInfo(final String id, final String host, final String serviceFolder,
                       final String args, final boolean autostart, final Svc svc, final byte status) {
        this(id, host, serviceFolder, args, autostart, 0, svc, status);
    }

    public String getHost() {
        return host;
    }

    public String getId() {
        return id;
    }

    public String getArgs() {
        return args;
    }

    public Map getComponents() {
        return components;
    }

    public void setComponents(final Element response) {
        components.clear();
        final NodeList list = response.getElementsByTagName("component");
        for (int i = 0; i < list.getLength(); i++) {
            final Element compElem = (Element) list.item(i);
            final Component component = new Component(compElem);
            components.put(component.getName(), component);
        }
    }

    public long getPid() {
        return pid;
    }

    public void setPid(final long pid) {
        this.pid = pid;
        if (0 == pid && (STATUS_RUNNING == status || STATUS_STOPPING == status))
            status = STATUS_STOPPED;
        if (0 != pid && (STATUS_STARTING == status || STATUS_STOPPED == status))
            status = STATUS_RUNNING;
    }

    public Svc getSvc() {
        return svc;
    }

    public Svc setSvc(final Svc svc) {
        final Svc old_svc = this.svc;
        this.svc = svc;
        return old_svc;
    }

    public byte getStatus() {
        return status;
    }

    protected void setStatusStr(String statusStr) {
        if ("running".equalsIgnoreCase(statusStr)) {
            this.status = STATUS_RUNNING;
        } else if ("starting".equalsIgnoreCase(statusStr)) {
            this.status = STATUS_STARTING;
        } else if ("stopping".equalsIgnoreCase(statusStr)) {
            this.status = STATUS_STOPPING;
        } else if ("stopped".equalsIgnoreCase(statusStr)) {
            this.status = STATUS_STOPPED;
        } else
            this.status = STATUS_UNKNOWN;
    }

    public String getStatusStr() {
        switch (status) {
            case STATUS_RUNNING:
                return "running";
            case STATUS_STARTING:
                return "starting";
            case STATUS_STOPPING:
                return "stopping";
            case STATUS_STOPPED:
                return "stopped";
            case STATUS_UNKNOWN:
                return "unknown";
            default:
                return "unknown";
        }
    }

    public void setStatus(final byte status) {
        this.status = status;
    }

    public void setArgs(final String args) {
        this.args = args;
    }

    public File getServiceFolder() {
        return serviceFolder;
    }

    public boolean isAutostart() {
        return autostart;
    }

}
