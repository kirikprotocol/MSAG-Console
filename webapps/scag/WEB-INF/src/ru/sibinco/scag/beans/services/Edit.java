/*
 * Copyright (c) 2006 SibInco Inc. All Rights Reserved.
 */

package ru.sibinco.scag.beans.services;

import ru.sibinco.scag.Constants;
import ru.sibinco.scag.backend.SCAGAppContext;
import ru.sibinco.scag.backend.service.ServiceProvider;
import ru.sibinco.scag.backend.service.ServiceProvidersManager;
import ru.sibinco.scag.beans.*;

import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;
import java.util.*;

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
            appContext = (SCAGAppContext) request.getAttribute(Constants.APP_CONTEXT);
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
        if (appContext.getHttpRoutingManager().isRoutesChanged() || appContext.getScagRoutingManager().isRoutesChanged()){
            throw new SCAGJspException(Constants.errors.routing.routes.COULD_NOT_DELETE_SERVICE);
        } else {
            for (int i = 0; i < checked.length; i++) {
                final String serviceIdStr = checked[i];
                final Long serviceId = Long.decode(serviceIdStr);
                toRemove.add(serviceId);
                appContext.getRuleManager().removeRulesForService(getLoginedPrincipal().getName(), serviceIdStr);
            }
            final List toRemoveSmppRoutes = appContext.getScagRoutingManager().getRoteIdsByServiceIds(checked);
            final List toRemoveHttpRoutes = appContext.getHttpRoutingManager().getRoteIdsByServiceIds(checked);
            if (toRemoveSmppRoutes.size() > 0) {
                appContext.getScagRoutingManager().deleteRoutes(getLoginedPrincipal().getName(),
                        toRemoveSmppRoutes);
            }
            if (toRemoveHttpRoutes.size() > 0) {
                appContext.getHttpRoutingManager().deleteRoutes(getLoginedPrincipal().getName(),
                        toRemoveSmppRoutes);
            }
            appContext.getServiceProviderManager().deleteServices(getLoginedPrincipal().getName(),
                    toRemove, serviceProvider, appContext);
        }
    }

    protected void load() throws SCAGJspException {
        if (!isAdd() && getTabledItems() != null) {
            logger.info( "SERVICES:LOAD: " + getEditId() );
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
        ServiceProvider oldProvider = null;
        if( !serviceProvidersManager.checkForbiddenChars(name) ){
            logger.error( "Attempt to set illegal character into provider name - '" +
                    name + "' with '" + Constants.FORBIDDEN_CHARACTER + "'");
            throw new SCAGJspException( Constants.errors.CAN_NOT_SAVE_NAME_WITH_FORBIDDEN_CHARACTER,
                        Constants.FORBIDDEN_CHARACTER );
        }
        if( serviceProvidersManager.isUniqueProviderName(name, id) ){
            if( isAdd() ) {
                    id = serviceProvidersManager.createServiceProvider(userLogin, getName(), description);
            } else {
                oldProvider = serviceProvidersManager.updateServiceProvider(userLogin, id, getName(), description);
            }
        }else {
            logger.error( "services.Edit:save():provider - name not unique" );
            throw new SCAGJspException( Constants.errors.providers.CAN_NOT_SAVE_PROVIDER_NOT_UNIQUE_NAME, name );
        }
        appContext.getServiceProviderManager().reloadServices(appContext, isAdd(), id, oldProvider);
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
        if(name != null)name.trim();
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