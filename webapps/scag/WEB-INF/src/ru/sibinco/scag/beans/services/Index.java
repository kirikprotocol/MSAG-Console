/*
 * Copyright (c) 2006 SibInco Inc. All Rights Reserved.
 */

package ru.sibinco.scag.beans.services;

import ru.sibinco.scag.backend.service.ServiceProvider;
import ru.sibinco.scag.beans.SCAGJspException;
import ru.sibinco.scag.beans.TabledBeanImpl;

import java.util.ArrayList;
import java.util.Collection;
import java.util.Iterator;
import java.util.List;

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
        final List serviceIds = new ArrayList();
        for (int i = 0; i < checked.length; i++) {
            final String serviceProviderIdStr = checked[i];
            final Long providerId = Long.decode(serviceProviderIdStr);
            toRemove.add(providerId);
            ServiceProvider serviceProvider = (ServiceProvider)
                    appContext.getServiceProviderManager().getServiceProviders().get(providerId);
            for (Iterator iterator = serviceProvider.getServices().keySet().iterator(); iterator.hasNext();) {
                String serviceIdStr = (iterator.next()).toString();
                appContext.getRuleManager().removeRulesForService(serviceIdStr);
                serviceIds.add(serviceIdStr);
            }
        }
        final List toRemoveRoutes = appContext.getScagRoutingManager().getRoteIdsByServiceIds(
                (String[]) serviceIds.toArray(new String[serviceIds.size()]));
        appContext.getScagRoutingManager().deleteRoutes(getLoginedPrincipal().getName(), toRemoveRoutes);
        appContext.getServiceProviderManager().deleteServiceProviders(getLoginedPrincipal().getName(), toRemove);
        appContext.getServiceProviderManager().reloadServices(appContext.getScag());
    }
}
