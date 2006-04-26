/*
 * Copyright (c) 2006 SibInco Inc. All Rights Reserved.
 */

package ru.sibinco.scag.beans.services;

import ru.sibinco.lib.SibincoException;
import ru.sibinco.scag.Constants;
import ru.sibinco.scag.backend.SCAGAppContext;
import ru.sibinco.scag.backend.Scag;
import ru.sibinco.scag.backend.daemon.Proxy;
import ru.sibinco.scag.backend.service.ServiceProvider;
import ru.sibinco.scag.backend.service.ServiceProvidersManager;
import ru.sibinco.scag.beans.*;

import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;
import java.io.IOException;
import java.util.ArrayList;
import java.util.Collection;
import java.util.List;
import java.util.Map;

/**
 * The <code>Edit</code> class represents
 * <p><p/>
 * Date: 06.02.2006
 * Time: 14:25:11
 *
 * @author &lt;a href="mailto:igor@sibinco.ru"&gt;Igor Klimenko&lt;/a&gt;
 */
public class Edit extends TabledEditBeanImpl {
    private long id = -1;
    private String name;
    private String description = "";
    private String editId = null;
    private boolean add = false;
    private boolean delete = false;
    private boolean addService = false;
    private String mbSave = null;
    private String mbCancel = null;
    private String mbAddChild;
    private String childEitId;
    private String userLogin;

    ServiceProvider serviceProvider;

    protected Collection getDataSource() {
        return serviceProvider.getServices().values();
    }

    public void process(final HttpServletRequest request, final HttpServletResponse response) throws SCAGJspException {
        if (appContext == null) {
            appContext = (SCAGAppContext) request.getAttribute("appContext");
        }
        userLogin = request.getUserPrincipal().getName();
        if (getMbCancel() != null) {
            throw new CancelException();
        } else if (getMbSave() != null) {
            save();
        } else if (getMbAddChild() != null) {
            throw new AddChildException("service", Long.toString(id));
        }
        load();
        if (mbDelete != null) {
            loginedPrincipal = request.getUserPrincipal();
        }
        if (!isAdd()) {
            super.process(request, response);
        }
    }


    protected void delete() throws SCAGJspException {
        final List toRemove = new ArrayList(checked.length);
        for (int i = 0; i < checked.length; i++) {
            final String serviceIdStr = checked[i];
            final Long serviceId = Long.decode(serviceIdStr);
            toRemove.add(serviceId);
            appContext.getRuleManager().removeRulesForService(serviceIdStr);
        }
        final List toRemoveRoutes = appContext.getScagRoutingManager().getRoteIdsByServiceIds(checked);
        appContext.getScagRoutingManager().deleteRoutes(getLoginedPrincipal().getName(),
                 toRemoveRoutes);
        appContext.getServiceProviderManager().deleteServices(getLoginedPrincipal().getName(),
                toRemove, serviceProvider);
        appContext.getServiceProviderManager().reloadServices(appContext.getScag());
    }

    protected void load() throws SCAGJspException {
        if (!isAdd() && getTabledItems() != null) {
            final Map serviceProviders = appContext.getServiceProviderManager().getServiceProviders();
            final Long longLoadId = Long.decode(getEditId());
            if (!serviceProviders.containsKey(longLoadId))
                throw new SCAGJspException(Constants.errors.serviceProviders.SERVICE_PROVIDER_NOT_FOUND, getEditId());
            serviceProvider = (ServiceProvider) serviceProviders.get(longLoadId);
            this.id = serviceProvider.getId().longValue();
            this.name = serviceProvider.getName();
            this.description = serviceProvider.getDescription();
            delete = isDelete();
        }
    }

    protected void save() throws SCAGJspException {
        final ServiceProvidersManager serviceProvidersManager = appContext.getServiceProviderManager();
        if (isAdd()) {
            id = serviceProvidersManager.createServiceProvider(userLogin, name, description);
        } else {
            serviceProvidersManager.updateServiceProvider(userLogin, id, name, description);
        }
        final Scag scag = appContext.getScag();
        try {
            scag.reloadServices();
        } catch (SibincoException e) {
            if (Proxy.STATUS_CONNECTED == scag.getStatus()) {
                throw new SCAGJspException(Constants.errors.serviceProviders.COULDNT_RELOAD_SERVICE_PROVIDER, Long.toString(id), e);
            }
        } finally {
            try {
                serviceProvidersManager.store();
            } catch (IOException e) {
                logger.debug("Couldn't save config", e);
            }
        }
        if (isAdd()) {
            throw new EditException(Long.toString(id));
        } else {
            throw new DoneException();
        }
    }

    public String getId() {
        return -1 == id ? null : String.valueOf(id);
    }

    public void setId(final String id) {
        this.id = Long.decode(id).longValue();
    }

    public String getName() {
        return name;
    }

    public void setName(final String name) {
        this.name = name;
    }

    public String getEditId() {
        return editId;
    }

    public void setEditId(String editId) {
        this.editId = editId;
    }

    public boolean isAdd() {
        return add;
    }

    public void setAdd(boolean add) {
        this.add = add;
    }

    public String getMbSave() {
        return mbSave;
    }

    public void setMbSave(String mbSave) {
        this.mbSave = mbSave;
    }

    public String getMbCancel() {
        return mbCancel;
    }

    public void setMbCancel(String mbCancel) {
        this.mbCancel = mbCancel;
    }

    public String getMbAddChild() {
        return mbAddChild;
    }

    public void setMbAddChild(String mbAddChild) {
        this.mbAddChild = mbAddChild;
    }

    public String getDescription() {
        return description;
    }

    public void setDescription(String description) {
        this.description = description;
    }

    public boolean isDelete() {
        return delete = (serviceProvider != null && serviceProvider.getServices() != null);
    }

    public boolean isAddService() {
        return addService = (serviceProvider != null);
    }

    public String getChildEitId() {
        return childEitId;
    }

    public void setChildEitId(String childEitId) {
        this.childEitId = childEitId;
    }


}