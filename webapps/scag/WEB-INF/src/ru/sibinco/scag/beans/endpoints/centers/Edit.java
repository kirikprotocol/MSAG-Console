/*
 * Copyright (c) 2005 Your Corporation. All Rights Reserved.
 */
package ru.sibinco.scag.beans.endpoints.centers;

import ru.sibinco.scag.beans.EditBean;
import ru.sibinco.scag.beans.SCAGJspException;
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
public class Edit extends EditBean{

  private String id;
  private String host;
  private int port;
  private String systemId;
  private String password;
  private int responseTimeout;
  private int uniqueMsgIdPrefix;
  private String altHost;
  private int altPort;
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

    public String getSystemId() {
        return systemId;
    }

    public void setSystemId(final String systemId) {
        this.systemId = systemId;
    }

    public String getPassword() {
        return password;
    }

    public void setPassword(final String password) {
        this.password = password;
    }

    public int getResponseTimeout() {
        return responseTimeout;
    }

    public void setResponseTimeout(final int responseTimeout) {
        this.responseTimeout = responseTimeout;
    }

    public int getUniqueMsgIdPrefix() {
        return uniqueMsgIdPrefix;
    }

    public void setUniqueMsgIdPrefix(final int uniqueMsgIdPrefix) {
        this.uniqueMsgIdPrefix = uniqueMsgIdPrefix;
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

    public String[] getTranspotIds() {  //ToDo
        return new String[]{
            "1",  "2", "3"
        };
    }

    public String[] getTranspotTitles() { //ToDo
        return new String[]{
            "SMPP",  "WAP", "MMS"
        };
    }

    public long getTransportId() {
        return transportId;
    }

    public void setTransportId(long transportId) {
        this.transportId = transportId;
    }
}
