/*
 * Copyright (c) 2006 SibInco Inc. All Rights Reserved.
 */

package ru.sibinco.scag.beans.services;

import ru.sibinco.scag.beans.SCAGJspException;
import ru.sibinco.scag.beans.DoneException;
import ru.sibinco.scag.beans.CancelException;
import ru.sibinco.scag.beans.EditException;
import ru.sibinco.scag.beans.AddChildException;
import ru.sibinco.scag.beans.TabledEditBeanImpl;
import ru.sibinco.scag.Constants;
import ru.sibinco.scag.backend.service.ServiceProvider;
import ru.sibinco.scag.backend.service.ServiceProvidersManager;
import ru.sibinco.scag.backend.SCAGAppContext;

import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;
import java.util.Map;
import java.util.Collection;
import java.util.List;
import java.util.ArrayList;
import java.io.IOException;

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
    private String description;
    private String editId = null;
    private boolean add = false;
    private boolean delete = false;
    private boolean addService = false;
    private String mbSave = null;
    private String mbCancel = null;
    private String mbAddChild;
    private String childEitId;


    ServiceProvider serviceProvider;

    protected Collection getDataSource() {
        return serviceProvider.getServices().values();
    }

    public void process(final HttpServletRequest request, final HttpServletResponse response) throws SCAGJspException {
        if (appContext == null) {
            appContext = (SCAGAppContext) request.getAttribute("appContext");
        }
        if (getMbCancel() != null) {
            throw new CancelException();
        } else if (getMbSave() != null) {
            save();
        } else if (getMbAddChild() != null) {
            throw new AddChildException("service", Long.toString(id));
        }
        load();
        if (!isAdd()) {
            super.process(request, response);
        }
    }

    protected void load(final String loadId) throws SCAGJspException {

    }

    protected void delete() throws SCAGJspException {
        final List toRemove = new ArrayList(checked.length);
        for (int i = 0; i < checked.length; i++) {
            final String serviceIdStr = checked[i];
            final Long serviceId = Long.decode(serviceIdStr);
            toRemove.add(serviceId);
            appContext.getRuleManager().removeRulesForService(serviceIdStr);
        }
        serviceProvider.getServices().keySet().removeAll(toRemove);
        try {
            appContext.getServiceProviderManager().store();
        } catch (IOException e) {
            logger.debug("Couldn't save config", e);
            throw new SCAGJspException(Constants.errors.status.COULDNT_SAVE_CONFIG, e);
        }
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
            id = serviceProvidersManager.createServiceProvider(name, description);
        } else {
            serviceProvidersManager.updateServiceProvider(id, name, description);
        }

        try {
            serviceProvidersManager.store();
        } catch (IOException e) {
            logger.debug("Couldn't save config", e);
            throw new SCAGJspException(Constants.errors.status.COULDNT_SAVE_CONFIG, e);
        }
        if (isAdd()) {
            throw new EditException(new Long(id).toString());
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