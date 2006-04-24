/*
 * Copyright (c) 2006 SibInco Inc. All Rights Reserved.
 */

package ru.sibinco.scag.beans.services;

import ru.sibinco.scag.beans.TabledBeanImpl;
import ru.sibinco.scag.beans.SCAGJspException;
import ru.sibinco.scag.Constants;
import ru.sibinco.scag.backend.service.ServiceProvider;
import ru.sibinco.scag.backend.Scag;
import ru.sibinco.scag.backend.SCAGAppContext;
import ru.sibinco.scag.backend.status.StatMessage;
import ru.sibinco.scag.backend.status.StatusManager;
import ru.sibinco.scag.backend.daemon.Proxy;
import ru.sibinco.lib.SibincoException;
import ru.sibinco.lib.backend.users.User;

import java.util.Collection;
import java.util.List;
import java.util.ArrayList;
import java.util.Iterator;
import java.io.IOException;
import java.security.Principal;

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
            ServiceProvider serviceProvider = (ServiceProvider) appContext.getServiceProviderManager().getServiceProviders().get(providerId);
            for (Iterator iterator = serviceProvider.getServices().keySet().iterator(); iterator.hasNext();) {
                String serviceIdStr = (iterator.next()).toString();
                appContext.getRuleManager().removeRulesForService(serviceIdStr);
                serviceIds.add(serviceIdStr);
            }
        }
        final List toRemoveRoutes = appContext.getScagRoutingManager().getRoteIdsByServiceIds((String[]) serviceIds.toArray(new String[serviceIds.size()]));
        appContext.getScagRoutingManager().getRoutes().keySet().removeAll(toRemoveRoutes);
        appContext.getScagRoutingManager().setRoutesChanged(true);
        StatMessage message = new StatMessage(getUser(appContext).getLogin(), "Routes", "Deleted route(s): " + toRemoveRoutes.toString() + ".");
        appContext.getScagRoutingManager().addStatMessages(message);
        StatusManager.getInstance().addStatMessages(message);
        appContext.getServiceProviderManager().getServiceProviders().keySet().removeAll(toRemove);
        message = new StatMessage(getUser(appContext).getLogin(), "Routes", "Deleted service prvider(s): " + toRemove.toString() + ".");
        StatusManager.getInstance().addStatMessages(message);
        final Scag scag = appContext.getScag();
        try {
            scag.reloadServices();
        } catch (SibincoException e) {
            if (Proxy.STATUS_CONNECTED == scag.getStatus()) {
                throw new SCAGJspException(Constants.errors.serviceProviders.COULDNT_RELOAD_SERVICE_PROVIDER, e);
            }
        } finally {
            try {
                appContext.getServiceProviderManager().store();
            } catch (IOException e) {
                logger.debug("Couldn't save config", e);
            }
        }
    }

    private User getUser(SCAGAppContext appContext) throws SCAGJspException {
        Principal userPrincipal = super.getLoginedPrincipal();
        if (userPrincipal == null)
            throw new SCAGJspException(Constants.errors.users.USER_NOT_FOUND, "Failed to obtain user principal(s)");
        User user = (User) appContext.getUserManager().getUsers().get(userPrincipal.getName());
        if (user == null)
            throw new SCAGJspException(Constants.errors.users.USER_NOT_FOUND, "Failed to locate user '" + userPrincipal.getName() + "'");
        return user;
    }
}
