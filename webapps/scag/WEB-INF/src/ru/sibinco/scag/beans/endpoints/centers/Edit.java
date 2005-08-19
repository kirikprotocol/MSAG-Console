/*
 * Copyright (c) 2005 Your Corporation. All Rights Reserved.
 */
package ru.sibinco.scag.beans.endpoints.centers;

import ru.sibinco.scag.beans.EditBean;
import ru.sibinco.scag.beans.SCAGJspException;
import ru.sibinco.scag.backend.endpoints.svc.Svc;
import ru.sibinco.lib.backend.util.SortedList;

import java.util.Iterator;

/**
 * The <code>Edit</code> class represents
 * <p><p/>
 * Date: 15.07.2005
 * Time: 12:10:30
 *
 * @author &lt;a href="mailto:igor@sibinco.ru"&gt;Igor Klimenko&lt;/a&gt;
 */
public class Edit extends EditBean {

    private String id = null;
    private String password = null;
    private int timeout = 0;
    private byte mode = Svc.MODE_TRX;
    private String host = null;
    private int port = 0;
    private String altHost = null;
    private int altPort = 0;
    private boolean enabled = false;
    protected long transportId = 1;


    protected void load(String loadId) throws SCAGJspException {
        //To change body of implemented methods use File | Settings | File Templates.
    }

    protected void save() throws SCAGJspException {
        //To change body of implemented methods use File | Settings | File Templates.
    }


    public String[] getSmes() {
        final SortedList smes = new SortedList(appContext.getGwSmeManager().getSmes().keySet());
        for (Iterator i = appContext.getSmscsManager().getSmscs().keySet().iterator(); i.hasNext();) {
            final String smscId = (String) i.next();
            if (!smscId.equals(id))
                smes.remove(smscId);
        }
        return (String[]) smes.toArray(new String[0]);
    }

    public String getId() {
        return id;
    }

    public void setId(final String id) {
        this.id = id;
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

    public String getPassword() {
        return password;
    }

    public void setPassword(final String password) {
        this.password = password;
    }

    public String getAltHost() {
        return altHost;
    }

    public void setAltHost(final String altHost) {
        this.altHost = altHost;
    }

    public int getAltPort() {
        return altPort;
    }

    public void setAltPort(final int altPort) {
        this.altPort = altPort;
    }

    public int getTimeout() {
        return timeout;
    }

    public void setTimeout(final int timeout) {
        this.timeout = timeout;
    }

    public byte getMode() {
        return mode;
    }

    public void setMode(final byte mode) {
        this.mode = mode;
    }

    public boolean isEnabled() {
        return enabled;
    }

    public void setEnabled(final boolean enabled) {
        this.enabled = enabled;
    }

    public String[] getTranspotIds() {  //ToDo
        return new String[]{
                "1", "2", "3"
        };
    }

    public String[] getTranspotTitles() { //ToDo
        return new String[]{
                "SMPP", "WAP", "MMS"
        };
    }

    public long getTransportId() {
        return transportId;
    }

    public void setTransportId(long transportId) {
        this.transportId = transportId;
    }
}
