/*
 * Copyright (c) 2005 Your Corporation. All Rights Reserved.
 */
package ru.sibinco.scag.beans.endpoints.centers;

import ru.sibinco.scag.beans.SCAGJspException;
import ru.sibinco.scag.beans.TabledBeanImpl;

import java.util.Collection;
import java.util.List;
import java.util.Map;

/**
 * The <code>Index</code> class represents
 * <p><p/>
 * Date: 15.07.2005
 * Time: 12:08:56
 *
 * @author &lt;a href="mailto:igor@sibinco.ru"&gt;Igor Klimenko&lt;/a&gt;
 */
public class Index extends TabledBeanImpl {


    protected Collection getDataSource() {
        return appContext.getSmppManager().getCenters().values();
    }

    protected void delete() throws SCAGJspException {
        appContext.getSmppManager().deleteCenters(getLoginedPrincipal().getName(),
                checkedSet, appContext);
    }

    public List getCenterIds() {
        return appContext.getSmppManager().getCenterNames();
    }

    public Map getCenterStatuses() {
        return appContext.getSmppManager().getCenterStatuses(appContext);
    }

}
