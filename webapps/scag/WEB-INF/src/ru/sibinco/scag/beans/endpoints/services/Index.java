/*
 * Copyright (c) 2005 SibInco Inc. All Rights Reserved.
 */
package ru.sibinco.scag.beans.endpoints.services;

import ru.sibinco.scag.beans.TabledBeanImpl;
import ru.sibinco.scag.beans.SCAGJspException;
import ru.sibinco.scag.backend.Gateway;
import ru.sibinco.scag.backend.endpoints.svc.Svc;
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
 * Time: 15:59:44
 *
 * @author &lt;a href="mailto:igor@sibinco.ru"&gt;Igor Klimenko&lt;/a&gt;
 */
public class Index extends TabledBeanImpl implements TabledBean {

    protected Collection getDataSource() {
        return appContext.getSmppManager().getSvcs().values();
    }

    protected void delete() throws SCAGJspException {
        final Gateway gateway = appContext.getGateway();
        final Map svcs = appContext.getSmppManager().getSvcs();
        for (Iterator iterator = checkedSet.iterator(); iterator.hasNext();) {
            final String svcId = (String) iterator.next();

            try {
                gateway.deleteSvc(svcId);
                svcs.remove(svcId);
            } catch (SibincoException e) {
                if (Proxy.StatusConnected == gateway.getStatus()) {
                    logger.error("Couldn't delete sme \"" + svcId + '"', e);
                    throw new SCAGJspException(Constants.errors.sme.COULDNT_DELETE, svcId, e);
                } else
                    svcs.remove(svcId);
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
