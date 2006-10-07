/*
 * Copyright (c) 2006 SibInco Inc. All Rights Reserved.
 */

package ru.sibinco.scag.backend.endpoints.svc;

import org.apache.log4j.Category;

import java.util.Map;

/**
 * The <code>SvcStatus</code> class represents of
 * <p/>
 * Date: 06.10.2006
 *
 * @author &lt;a href="mailto:igor@sibinco.ru"&gt;Igor Klimenko&lt;/a&gt;
 */
public class SvcStatus {

    private Category logger = Category.getInstance(Class.class);

    private String id;
    private String connStatus = "unknown";
    private String connHost = "";

    public SvcStatus(String id) {
        this.id = id;
    }

    public SvcStatus(String id, String connStatus, String connHost) {
        this.id = id;
        this.connStatus = connStatus;
        this.connHost = connHost;
    }

    public SvcStatus(final Map status) {
        id = (String) status.get("SystemId");
        connStatus = (String) status.get("Status");
        connHost = ((String) status.get("Host")).equals("Unknown") ? "" : (String) status.get("Host");
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

    public String getConnHost() {
        return connHost;
    }

    public void setConnHost(String connHost) {
        this.connHost = connHost;
    }
}
