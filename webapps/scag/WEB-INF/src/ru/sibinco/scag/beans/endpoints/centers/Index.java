/*
 * Copyright (c) 2005 Your Corporation. All Rights Reserved.
 */
package ru.sibinco.scag.beans.endpoints.centers;

import ru.sibinco.scag.beans.TabledBeanImpl;
import ru.sibinco.scag.beans.SCAGJspException;
import ru.sibinco.scag.backend.Gateway;
import ru.sibinco.scag.backend.endpoints.centers.Center;
import ru.sibinco.scag.Constants;
import ru.sibinco.lib.bean.TabledBean;
import ru.sibinco.lib.SibincoException;
import ru.sibinco.lib.backend.protocol.Proxy;

import java.util.Collection;
import java.util.Map;
import java.util.Iterator;

/**
 * The <code>Index</code> class represents
 * <p><p/>
 * Date: 15.07.2005
 * Time: 12:08:56
 *
 * @author &lt;a href="mailto:igor@sibinco.ru"&gt;Igor Klimenko&lt;/a&gt;
 */
public class Index extends TabledBeanImpl implements TabledBean {


    protected Collection getDataSource() {
        return appContext.getSmppManager().getCenters().values();
    }

    protected void delete() throws SCAGJspException {
        final Gateway gateway = appContext.getGateway();
        final Map centers = appContext.getSmppManager().getCenters();
        for (Iterator iterator = checkedSet.iterator(); iterator.hasNext();) {
            final String centerId = (String) iterator.next();
            Center center = (Center) centers.get(centerId);
            try {
                gateway.deleteCenter(center);
                centers.remove(centerId);
            } catch (SibincoException e) {
                if (Proxy.StatusConnected == gateway.getStatus()) {
                    logger.error("Couldn't delete Smsc \"" + centerId + '"', e);
                    throw new SCAGJspException(Constants.errors.sme.COULDNT_DELETE, centerId, e);
                } else
                    centers.remove(centerId);
            } finally {
                try {
                    appContext.getSmppManager().store();
                } catch (SibincoException e) {
                    logger.error("Couldn't store smes ", e);
                }
            }
        }
    }
}
