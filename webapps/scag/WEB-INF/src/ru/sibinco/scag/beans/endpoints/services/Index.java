/*
 * Copyright (c) 2005 SibInco Inc. All Rights Reserved.
 */
package ru.sibinco.scag.beans.endpoints.services;

import ru.sibinco.lib.bean.TabledBean;
import ru.sibinco.scag.beans.SCAGJspException;
import ru.sibinco.scag.beans.TabledBeanImpl;

import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;
import java.util.Collection;

/**
 * The <code>Index</code> class represents
 * <p><p/>
 * Date: 15.07.2005
 * Time: 15:59:44
 *
 * @author &lt;a href="mailto:igor@sibinco.ru"&gt;Igor Klimenko&lt;/a&gt;
 */
public class Index extends TabledBeanImpl implements TabledBean {

    private String mbDisconnect = null;

    public void process(HttpServletRequest request, HttpServletResponse response) throws SCAGJspException {
      super.process(request, response);
      if (mbDisconnect!=null && checked.length!=0) {
        appContext.getSmppManager().disconnectServices(getUserName(), checked, appContext);
      }
    }

    protected Collection getDataSource() {
        return appContext.getSmppManager().getSvcs().values();
    }

    protected void delete() throws SCAGJspException {
        appContext.getSmppManager().deleteServicePoints(getUserName(),
                checkedSet, appContext);
    }

    public void setMbDisconnect(String mbDisconnect) {
        this.mbDisconnect = mbDisconnect;
    }
}
