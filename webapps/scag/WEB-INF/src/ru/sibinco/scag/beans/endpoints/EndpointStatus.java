/*
 * Copyright (c) 2006 SibInco Inc. All Rights Reserved.
 */

package ru.sibinco.scag.beans.endpoints;

import ru.sibinco.scag.beans.SCAGBean;

import java.util.List;
import java.util.Map;

/**
 * The <code>EndpointStatus</code> class represents
 * <p><p/>
 * Date: 07.10.2006
 * Time: 18:27:18
 *
 * @author &lt;a href="mailto:igor@sibinco.ru"&gt;Igor Klimenko&lt;/a&gt;
 */
public class EndpointStatus extends SCAGBean {

    public List getCenterIds() {
        return appContext.getSmppManager().getCenterNames();
    }

    public Map getCenterStatuses() {
        return appContext.getSmppManager().getCenterStatuses(appContext);
    }

    public List getSvcIds() {
        return appContext.getSmppManager().getSvcsNames();
    }

    public Map getSvcStatuses() {
        return appContext.getSmppManager().getSvcsStatuses(appContext);
    }

}
