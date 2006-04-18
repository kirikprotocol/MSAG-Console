/*
 * Copyright (c) 2006 SibInco Inc. All Rights Reserved.
 */

package ru.sibinco.scag.beans.operators;

import ru.sibinco.scag.Constants;
import ru.sibinco.scag.backend.Scag;
import ru.sibinco.scag.backend.daemon.Proxy;
import ru.sibinco.scag.beans.SCAGJspException;
import ru.sibinco.scag.beans.TabledBeanImpl;
import ru.sibinco.lib.SibincoException;

import java.io.IOException;
import java.util.ArrayList;
import java.util.Collection;
import java.util.List;

/**
 * The <code>Index</code> class represents
 * <p><p/>
 * Date: 02.03.2006
 * Time: 13:52:21
 *
 * @author &lt;a href="mailto:igor@sibinco.ru"&gt;Igor Klimenko&lt;/a&gt;
 */
public class Index extends TabledBeanImpl {

    protected Collection getDataSource() {
        return appContext.getOperatorManager().getOperators().values();
    }

    protected void delete() throws SCAGJspException {
        final List toRemove = new ArrayList(checked.length);
        for (int i = 0; i < checked.length; i++) {
            final String operatorIdStr = checked[i];
            final Long operatorId = Long.decode(operatorIdStr);
            toRemove.add(operatorId);
        }
        appContext.getOperatorManager().getOperators().keySet().removeAll(toRemove);
        final Scag scag = appContext.getScag();
        try {
            scag.reloadOperators();
        } catch (SibincoException e) {
            if (Proxy.STATUS_CONNECTED == scag.getStatus()) {
                throw new SCAGJspException(Constants.errors.serviceProviders.COULDNT_RELOAD_SERVICE_PROVIDER, e);
            }
        } finally {
            try {
                appContext.getOperatorManager().store();
            } catch (IOException e) {
                logger.debug("Couldn't save config", e);
            }
        }
    }
}
