/*
 * Copyright (c) 2006 SibInco Inc. All Rights Reserved.
 */

package ru.sibinco.scag.beans.services;

import ru.sibinco.scag.beans.TabledBeanImpl;
import ru.sibinco.scag.beans.SCAGJspException;
import ru.sibinco.scag.Constants;

import java.util.Collection;
import java.util.List;
import java.util.ArrayList;
import java.io.IOException;

/**
 * The <code>Index</code> class represents
 * <p><p/>
 * Date: 06.02.2006
 * Time: 14:24:15
 *
 * @author &lt;a href="mailto:igor@sibinco.ru"&gt;Igor Klimenko&lt;/a&gt;
 */
public class Index extends TabledBeanImpl {

    protected Collection getDataSource() {
        return appContext.getServiceProviderManager().getServiceProviders().values();
    }

    protected void delete() throws SCAGJspException {
        final List toRemove = new ArrayList(checked.length);
        for (int i = 0; i < checked.length; i++) {
            final String serviceProviderIdStr = checked[i];
            final Long providerId = Long.decode(serviceProviderIdStr);
            toRemove.add(providerId);
        }
        appContext.getServiceProviderManager().getServiceProviders().keySet().removeAll(toRemove);
        try {
            appContext.getServiceProviderManager().store();
        } catch (IOException e) {
            logger.debug("Couldn't save config", e);
            throw new SCAGJspException(Constants.errors.status.COULDNT_SAVE_CONFIG, e);
        }
    }
}
