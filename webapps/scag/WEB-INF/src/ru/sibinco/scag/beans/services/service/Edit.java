/*
 * Copyright (c) 2006 SibInco Inc. All Rights Reserved.
 */

package ru.sibinco.scag.beans.services.service;

import ru.sibinco.scag.Constants;
import ru.sibinco.scag.backend.SCAGAppContext;
import ru.sibinco.scag.backend.transport.Transport;
import ru.sibinco.scag.backend.service.Service;
import ru.sibinco.scag.backend.service.ServiceProvider;
import ru.sibinco.scag.backend.service.ServiceProvidersManager;
import ru.sibinco.scag.beans.CancelChildException;
import ru.sibinco.scag.beans.EditChildException;
import ru.sibinco.scag.beans.SCAGJspException;
import ru.sibinco.scag.beans.TabledEditBeanImpl;
import ru.sibinco.scag.beans.AddChildException;
import ru.sibinco.scag.util.Utils;
import ru.sibinco.lib.SibincoException;
import ru.sibinco.lib.StatusDisconnectedException;
import ru.sibinco.lib.backend.util.SortedList;
import ru.sibinco.lib.backend.util.SortByPropertyComparator;

import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;
import java.io.IOException;
import java.util.Collection;
import java.util.Map;
import java.util.LinkedList;

/**
 * The <code>Edit</code> class represents
 * <p><p/>
 * Date: 22.02.2006
 * Time: 15:15:03
 *
 * @author &lt;a href="mailto:igor@sibinco.ru"&gt;Igor Klimenko&lt;/a&gt;
 */
public class Edit extends TabledEditBeanImpl {

    private long id = -1;
    private String name;
    private String description;
    private boolean add = false;
    private String editId = null;
    private String mbSave = null;
    private String mbCancel = null;
    private String deleteRuleSMPP = null;
    private String deleteRuleHTTP = null;
    private String deleteRuleMMS = null;
    private String parentId;
    private String dirName = "service";
    private boolean editChild = false;
    private Map serviceRules = null;
    private String path = "";
    private String mbAddChild;
    private Map routes = null;
    private Map serviceProviders;
    private String childEitId;

    protected Collection getDataSource() {
        return routes.values();
    }

    public void process(final HttpServletRequest request, final HttpServletResponse response) throws SCAGJspException {
        if (appContext == null) {
            appContext = (SCAGAppContext) request.getAttribute("appContext");
        }
        path = Utils.getPath(request);
        if (getMbCancel() != null) {
            String path = Utils.getPath(request);
            path = path.substring(0, (path.length() - (dirName.length() + 1))) + "edit.jsp?editId=" + (editChild ? getEditId() : getParentId());
            throw new CancelChildException(path);
        } else if (getMbSave() != null) {
            save();
        } else if (getMbAddChild() != null) {
            throw new AddChildException(request.getContextPath() + "/routing/routes", (!editChild ? getEditId() : getParentId()));
        }if (mbDelete != null){
            delete();
        }
        serviceProviders = appContext.getServiceProviderManager().getServiceProviders();
        routes = appContext.getServiceProviderManager().getRoutesByServiceId(appContext.getScagRoutingManager().getRoutes(), Long.decode(getParentId()));
        final SortedList results = new SortedList(getDataSource(), new SortByPropertyComparator(sort=(sort==null)?"id":sort));
        totalSize = results.size();
        if (totalSize > startPosition)
            tabledItems = results.subList(startPosition, Math.min(totalSize, startPosition + pageSize));
        else
            tabledItems = new LinkedList();
        load();
        if (serviceRules == null) serviceRules = appContext.getRuleManager().getRules(new Long(id));
        if (deleteRuleSMPP != null) deleteRule(Transport.SMPP_TRANSPORT_NAME);
        if (deleteRuleHTTP != null) deleteRule(Transport.HTTP_TRANSPORT_NAME);
        if (deleteRuleMMS != null) deleteRule(Transport.MMS_TRANSPORT_NAME);
        if (getEditId() != null && !editChild) {
            super.process(request, response);
        }

    }

    protected void delete() throws SCAGJspException {
        appContext.getScagRoutingManager().getRoutes().keySet().removeAll(checkedSet);
        appContext.getStatuses().setRoutesChanged(true);
    }

    protected void load() throws SCAGJspException {
        if (!isAdd() && getTabledItems() != null) {

            if (editChild) {
                if (!serviceProviders.containsKey(Long.decode(getEditId())))
                    throw new SCAGJspException(Constants.errors.serviceProviders.SERVICE_PROVIDER_NOT_FOUND, getEditId());
                ServiceProvider serviceProvider = (ServiceProvider) serviceProviders.get(Long.decode(getEditId()));
                if (getParentId() != null) {
                    final Long longLoadId = Long.decode(getParentId());
                    Service service = (Service) serviceProvider.getServices().get(longLoadId);
                    if (service != null) {
                        this.id = service.getId().longValue();
                        this.name = service.getName();
                        this.description = service.getDescription();
                    }
                }
            } else {
                if (!serviceProviders.containsKey(Long.decode(getParentId())))
                    throw new SCAGJspException(Constants.errors.serviceProviders.SERVICE_PROVIDER_NOT_FOUND, getParentId());
                ServiceProvider serviceProvider = (ServiceProvider) serviceProviders.get(Long.decode(getParentId()));
                if (getEditId() != null) {
                    final Long longLoadId = Long.decode(getEditId());
                    Service service = (Service) serviceProvider.getServices().get(longLoadId);
                    if (service != null) {
                        this.id = service.getId().longValue();
                        this.name = service.getName();
                        this.description = service.getDescription();
                    }
                }
            }
        }
    }

    protected void save() throws SCAGJspException {
        final ServiceProvidersManager serviceProvidersManager = appContext.getServiceProviderManager();
        if (description==null) description = "";
        if (getEditId() == null) {
            Service service = new Service(name, description);
            id = serviceProvidersManager.createService(Long.decode(getParentId()).longValue(), service);
        } else {
            if (editChild) {
                ServiceProvider serviceProvider = (ServiceProvider) serviceProvidersManager.getServiceProviders().get(Long.decode(getEditId()));
                Service service = (Service) serviceProvider.getServices().get(Long.decode(getParentId()));
                service.setName(name);
                service.setDescription(description);
                serviceProvidersManager.updateService(Long.decode(getEditId()).longValue(), service);
            } else {
                ServiceProvider serviceProvider = (ServiceProvider) serviceProvidersManager.getServiceProviders().get(Long.decode(getParentId()));
                Service service = (Service) serviceProvider.getServices().get(Long.decode(getEditId()));
                service.setName(name);
                service.setDescription(description);
                serviceProvidersManager.updateService(Long.decode(getParentId()).longValue(), service);
            }
        }

        try {
            serviceProvidersManager.store();
        } catch (IOException e) {
            logger.debug("Couldn't save config", e);
            throw new SCAGJspException(Constants.errors.status.COULDNT_SAVE_CONFIG, e);
        }
        if (id != -1) {
            throw new EditChildException(Long.toString(id), getParentId());
        } else {
            path = path.substring(0, (path.length() - (dirName.length() + 1))) + "edit.jsp?editId=" + (editChild ? getEditId() : getParentId());
            throw new CancelChildException(path);
        }
    }

    private void deleteRule(String transport) throws SCAGJspException {
        try {
            appContext.getRuleManager().removeRule(Long.toString(id), transport);
            serviceRules.remove(transport);
        } catch (SibincoException se) {
            if (se instanceof StatusDisconnectedException)
                serviceRules.remove(transport);
            else {
              se.printStackTrace();/*PRINT ERROR ON THE SCREEN;*/
              throw new SCAGJspException(Constants.errors.rules.COULD_NOT_REMOVE_RULE, se);
            }
        }
    }

    public String getId() {
        return -1 == id ? null : String.valueOf(id);
    }

    public String getName() {
        return name;
    }

    public void setName(String name) {
        this.name = name;
    }

    public String getDescription() {
        return description;
    }

    public void setDescription(String description) {
        this.description = description;
    }

    public String getEditId() {
        return editId;
    }

    public void setEditId(String editId) {
        this.editId = editId;
    }

    public String getMbSave() {
        return mbSave;
    }

    public void setMbSave(String mbSave) {
        this.mbSave = mbSave;
    }

    public String getDeleteRuleSMPP() {
        return deleteRuleSMPP;
    }

    public void setDeleteRuleSMPP(String deleteRuleSMPP) {
        this.deleteRuleSMPP = deleteRuleSMPP;
    }

    public String getDeleteRuleHTTP() {
        return deleteRuleHTTP;
    }

    public void setDeleteRuleHTTP(String deleteRuleHTTP) {
        this.deleteRuleHTTP = deleteRuleHTTP;
    }

    public String getDeleteRuleMMS() {
        return deleteRuleMMS;
    }

    public void setDeleteRuleMMS(String deleteRuleMMS) {
        this.deleteRuleMMS = deleteRuleMMS;
    }

    public String getMbCancel() {
        return mbCancel;
    }

    public void setMbCancel(String mbCancel) {
        this.mbCancel = mbCancel;
    }

    public boolean isAdd() {
        return add;
    }

    public String getParentId() {
        return parentId;
    }

    public void setParentId(String parentId) {
        this.parentId = parentId;
    }

    public boolean isEditChild() {
        return editChild;
    }

    public void setEditChild(boolean editChild) {
        this.editChild = editChild;
    }

    public boolean isSmppRuleExists() {
        serviceRules = appContext.getRuleManager().getRules(new Long(id));
        return (serviceRules.get(Transport.SMPP_TRANSPORT_NAME)!=null);
    }

    public boolean isHttpRuleExists() {

       return (serviceRules.get(Transport.HTTP_TRANSPORT_NAME)!=null);

    }

    public boolean isMmsRuleExists() {

       return (serviceRules.get(Transport.MMS_TRANSPORT_NAME)!=null);

    }

    public String getMbAddChild() {
        return mbAddChild;
    }

    public void setMbAddChild(String mbAddChild) {
        this.mbAddChild = mbAddChild;
    }

    public String getChildEitId() {
        return childEitId;
    }

    public void setChildEitId(String childEitId) {
        this.childEitId = childEitId;
    }
}
