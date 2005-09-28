/*
 * Copyright (c) 2005 SibInco Inc. All Rights Reserved.
 */
package ru.sibinco.scag.beans.rules.rules;

import ru.sibinco.scag.beans.EditBean;
import ru.sibinco.scag.beans.SCAGJspException;
import ru.sibinco.scag.backend.transport.Transport;

/**
 * The <code>Edit</code> class represents
 * <p><p/>
 * Date: 26.09.2005
 * Time: 16:31:47
 *
 * @author &lt;a href="mailto:igor@sibinco.ru"&gt;Igor Klimenko&lt;/a&gt;
 */
public class Edit extends EditBean {

    private String mbNext = null;

    protected long transportId = 1;

    public String getId() {
        return null;
    }

    protected void load(final String loadId) throws SCAGJspException {

    }

    protected void save() throws SCAGJspException {

    }

    public String getMbNext() {
        return mbNext;
    }

    public void setMbNext(String mbNext) {
        this.mbNext = mbNext;
    }

    public long getTransportId() {
        return transportId;
    }

    public void setTransportId(long transportId) {
        this.transportId = transportId;
    }

    public String[] getTranspotIds() {
        return Transport.transpotIds;
    }

    public String[] getTranspotTitles() {
        return Transport.transpotTitles;
    }
}
