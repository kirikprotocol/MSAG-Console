/*
 * Copyright (c) 2006 SibInco Inc. All Rights Reserved.
 */

package ru.sibinco.scag.backend.endpoints.centers;

import org.apache.log4j.Category;

import java.util.Map;

/**
 * The <code>CenterStatus</code> class represents of
 * <p/>
 * Date: 06.10.2006
 * Time: 13:20:22
 *
 * @author &lt;a href="mailto:igor@sibinco.ru"&gt;Igor Klimenko&lt;/a&gt;
 */
public class CenterStatus {

    private Category logger = Category.getInstance(Class.class);

    private String id;
    private String connStatus = "unknown";
    private String connHostPort = "";

    public CenterStatus(String id) {
        this.id = id;
    }

    public CenterStatus(String id, String connected, String outAddress) {
        this.id = id;
        this.connStatus = connected;
        this.connHostPort = outAddress;
    }

    public CenterStatus(final Map status) {
        id = (String) status.get("SystemId");
        connStatus = (String) status.get("Status");
        connHostPort = status.get("Host") + ":" + status.get("Port");        
    }

    public String getId() {
        return id;
    }

    public void setId(String id) {
        this.id = id;
    }

    public String getConnStatus() {
        return connStatus;
    }

    public void setConnStatus(String connStatus) {
        this.connStatus = connStatus;
    }

    public String getConnHostPort() {
        return connHostPort;
    }

    public void setConnHostPort(String connHostPort) {
        this.connHostPort = connHostPort;
    }
}
