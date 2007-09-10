/*
 * Copyright (c) 2006 SibInco Inc. All Rights Reserved.
 */

package ru.sibinco.scag.beans.services.service;

import ru.sibinco.lib.SibincoException;
import ru.sibinco.lib.backend.util.SortByPropertyComparator;
import ru.sibinco.lib.backend.util.SortedList;
import ru.sibinco.scag.Constants;
import ru.sibinco.scag.backend.SCAGAppContext;
import ru.sibinco.scag.backend.rules.RuleManager;
import ru.sibinco.scag.backend.service.Service;
import ru.sibinco.scag.backend.service.ServiceProvider;
import ru.sibinco.scag.backend.service.ServiceProvidersManager;
import ru.sibinco.scag.backend.transport.Transport;
import ru.sibinco.scag.beans.*;
import ru.sibinco.scag.beans.rules.RuleState;
import ru.sibinco.scag.util.Utils;

import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;
import javax.servlet.http.HttpSession;
import java.util.*;

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
    private String path = "";
    private String mbAddSmppRoute;
    private String mbAddHttpRoute;
    private Map routes = null;
    private Map serviceProviders;
    private String childEitId;
    private String mbDeleteHttpRoute;
    private String mbDefaultHttpRoute;
    private String unlockRuleSMPP = null;
    private String unlockRuleHTTP = null;
    private String unlockRuleMMS  = null;
    private String editRuleSMPP = null;
    private String editRuleHTTP = null;
    private String editRuleMMS  = null;

    public String getEditRuleSMPP() {
        return editRuleSMPP;
    }

    public String getEditRuleHTTP() {
        return editRuleHTTP;
    }

    public String getEditRuleMMS() {
        return editRuleMMS;
    }

    public void setEditRuleSMPP(String editRuleSMPP) {
        this.editRuleSMPP = editRuleSMPP;
    }

    public void setEditRuleHTTP(String editRuleHTTP) {
        this.editRuleHTTP = editRuleHTTP;
    }

    public void setEditRuleMMS(String editRuleMMS) {
        this.editRuleMMS = editRuleMMS;
    }

    public String getUnlockRuleSMPP() {
        return unlockRuleSMPP;
    }

    public String getUnlockRuleHTTP() {
        return unlockRuleHTTP;
    }

    public String getUnlockRuleMMS() {
        return unlockRuleMMS;
    }

    public void setId(long id) {
        logger.error("setId:'" + id +"'");
        this.id = id;
    }

    public void setUnlockRuleSMPP(String unblockRuleSMPP) {
        this.unlockRuleSMPP = unblockRuleSMPP;
    }

    public void setUnlockRuleHTTP(String unblockRuleHTTP) {
        this.unlockRuleHTTP = unblockRuleHTTP;
    }

    public void setUnlockRuleMMS(String unlockRuleMMS) {
        this.unlockRuleMMS = unlockRuleMMS;
    }

    private List httpRuteItems = new ArrayList();

    private int totalHttpSize = 0;

    protected Collection getDataSource() {
        return routes.values();
    }

    public void process(final HttpServletRequest request, final HttpServletResponse response) throws SCAGJspException {
        if (appContext == null) {
            appContext = (SCAGAppContext) request.getAttribute(Constants.APP_CONTEXT);
        }
        loginedPrincipal = request.getUserPrincipal();
        path = Utils.getPath(request);
        if (getMbCancel() != null) {
            String path = Utils.getPath(request);
            path = path.substring(0, (path.length() - (dirName.length() + 1))) + "edit.jsp?editId=" + (editChild ? getEditId() : getParentId());
            throw new CancelChildException(path);
        } else if (getMbSave() != null) {
            save();
        } else if (getMbAddSmppRoute() != null) {
            throw new AddChildException(request.getContextPath() + "/routing/routes", (!editChild ? getEditId() : getParentId()));
        } else if (getMbAddHttpRoute() != null) {
            throw new AddChildException(request.getContextPath() + "/routing/routes/http", (!editChild ? getEditId() : getParentId()));
        }
        if (mbDelete != null) {
            delete();
        }
        if (mbDeleteHttpRoute != null) {
            deleteHttpRoute();
        }
        Long servIdForRout;
        if( getEditId() != null ) {
            servIdForRout = (!editChild ? Long.decode(getEditId()) : Long.decode(getParentId()));
        } else {
            servIdForRout = Long.decode(getParentId());
        }
        HttpSession session = request.getSession();
        if (session.getAttribute(TabledBeanImpl.PAGE_SIZE) == null) {
            session.setAttribute(TabledBeanImpl.PAGE_SIZE, new Integer(25));
        } else {
            Integer pSize = (Integer) session.getAttribute(TabledBeanImpl.PAGE_SIZE);
            if (pSize.intValue() != pageSize && pageSize != 1) {
                session.setAttribute(TabledBeanImpl.PAGE_SIZE, new Integer(pageSize));
            }
        }
        pageSize = Integer.parseInt(String.valueOf(session.getAttribute(TabledBeanImpl.PAGE_SIZE)));
        serviceProviders = appContext.getServiceProviderManager().getServiceProviders();
        routes = appContext.getServiceProviderManager().getRoutesByServiceId(
                appContext.getScagRoutingManager().getRoutes(), servIdForRout);
        final SortedList results = new SortedList(getDataSource(), new SortByPropertyComparator(sort = (sort == null) ? "id" : sort));
        totalSize = results.size();
        if (totalSize > startPosition)
            tabledItems = results.subList(startPosition, Math.min(totalSize, startPosition + pageSize));
        else
            tabledItems = new LinkedList();

        final SortedList results2 = new SortedList(
                appContext.getServiceProviderManager().getHttpRoutesByServiceId(
                        appContext.getHttpRoutingManager().getRoutes(), servIdForRout).values(),
                new SortByPropertyComparator(sort));
        totalHttpSize = results2.size();
        if (totalHttpSize > startPosition)
            httpRuteItems = results2.subList(startPosition, Math.min(totalHttpSize, startPosition + pageSize));
        else
            httpRuteItems = new LinkedList();

        load();

        if (deleteRuleSMPP != null) {
            deleteRule(Transport.SMPP_TRANSPORT_NAME);
            System.out.println("!!!!!!deleteRuleSMPP!!!!!!!!!!!!");
        }
        if (deleteRuleHTTP != null) {
            deleteRule(Transport.HTTP_TRANSPORT_NAME);
        }
        if (deleteRuleMMS != null) {
            deleteRule(Transport.MMS_TRANSPORT_NAME);
        }
        if (mbDefaultHttpRoute!=null) setDefaultHttpRoute(new Long(id));
//unlock rule forcibly if clicked "unlock" button
        if( unlockRuleSMPP != null ) {
            logger.warn( "Edit:unlockRuleSMPP" );
            unlockRule( Transport.SMPP_TRANSPORT_NAME );
        } else if( unlockRuleHTTP != null ) {
            logger.warn( "Edit:unlockRuleHTTP" );
            unlockRule( Transport.HTTP_TRANSPORT_NAME );
        } else if( unlockRuleMMS != null )  {
            logger.warn( "Edit:unlockRuleMMS" );
            unlockRule( Transport.MMS_TRANSPORT_NAME );
        }
//set permission true if clicked "edit" button
        if( editRuleSMPP != null ) {
            logger.debug( "Edit:editRuleSMPP" );
            setPermissionRule( Transport.SMPP_TRANSPORT_NAME, true );
        } else if( editRuleHTTP != null ) {
            logger.debug( "Edit:editRuleHTTP" );
            setPermissionRule( Transport.HTTP_TRANSPORT_NAME, true );
        } else if( editRuleMMS != null ) {
            logger.debug( "Edit:editRuleMMS" );
            setPermissionRule( Transport.MMS_TRANSPORT_NAME, true );
        }

        if (getEditId() != null && !editChild) {
            super.process(request, response);
        }

    }

    void unlockRule( String transport ){
        RuleState ruleState = appContext.getRuleManager().getRuleState(Long.toString(id), transport);
        setPermissionRule( transport, false );
        ruleState.setLocked( false );
    }

    void setPermissionRule( String transport, boolean permission){
        RuleManager ruleManager = appContext.getRuleManager();
        if( transport.equals(Transport.SMPP_TRANSPORT_NAME) ){
            ruleManager.setSavePermissionSMPP( permission );
        }else if( transport.equals(Transport.HTTP_TRANSPORT_NAME) ){
            ruleManager.setSavePermissionHTTP( permission );
        }else if( transport.equals(Transport.MMS_TRANSPORT_NAME) ){
            ruleManager.setSavePermissionMMS( permission );
        }
    }

    private void setDefaultHttpRoute(Long serviceId) {
        appContext.getHttpRoutingManager().setDefaultHttpRoute(getLoginedPrincipal().getName(),checkedSet,serviceId);
    }

    private void deleteHttpRoute() {
       appContext.getHttpRoutingManager().deleteRoutes(getLoginedPrincipal().getName(), checkedSet);
    }


    protected void delete() throws SCAGJspException {
        appContext.getScagRoutingManager().deleteRoutes(getLoginedPrincipal().getName(), checkedSet);
    }

    protected void save() throws SCAGJspException {
        final ServiceProvidersManager serviceProvidersManager = appContext.getServiceProviderManager();
        Service oldService = null;
        Long serviceProviderId = null;
        if (description == null) description = "";

            if (getEditId() == null) {
                if( serviceProvidersManager.isUniqueServiceName( name, parentId )){
                    Service service = new Service(getName(), getDescription());
                    serviceProviderId = Long.decode(getParentId());
                    id = serviceProvidersManager.createService(getLoginedPrincipal().getName(), serviceProviderId.longValue(), service);
                }else{
                    logger.error( "services.Edit:save():service - name '" + name + "' is not unique" );
                    throw new SCAGJspException( Constants.errors.services.CAN_NOT_SAVE_SERVICE_NOT_UNIQUE_NAME, name );
                }
            } else {
                if (editChild) {
                    if( serviceProvidersManager.isUniqueServiceName(name, parentId) ){
                        serviceProviderId = Long.decode(getEditId());
                        ServiceProvider serviceProvider = (ServiceProvider) serviceProvidersManager.getServiceProviders().get(serviceProviderId);
                        Service service = (Service) serviceProvider.getServices().get(Long.decode(getParentId()));
                        oldService = service.copy();
                        service.setName(getName());
                        service.setDescription(getDescription());
                        serviceProvidersManager.updateService(getLoginedPrincipal().getName(), Long.decode(getEditId()).longValue(), service);
                    }else{
                        logger.error( "services.Edit:save():service - name '" + name + "' is not unique1" );
                        throw new SCAGJspException( Constants.errors.services.CAN_NOT_SAVE_SERVICE_NOT_UNIQUE_NAME, name );
                    }
                } else {
                    if( serviceProvidersManager.isUniqueServiceName( name, parentId )){
                        serviceProviderId = Long.decode(getParentId());
                        ServiceProvider serviceProvider = (ServiceProvider) serviceProvidersManager.getServiceProviders().get(serviceProviderId);
                        Service service = (Service) serviceProvider.getServices().get(Long.decode(getEditId()));
                        oldService = service.copy();
                        service.setName(getName());
                        service.setDescription(getDescription());
                        serviceProvidersManager.updateService(getLoginedPrincipal().getName(), Long.decode(getParentId()).longValue(), service);
                    }else{
                        logger.error( "services.Edit:save():service - name '" + name + "' is not unique2" );
                        throw new EditChildException(Long.toString(id), getParentId());
//                        throw new SCAGJspException( Constants.errors.services.CAN_NOT_SAVE_SERVICE_NOT_UNIQUE_NAME, name );
                    }
                }
            }

        appContext.getServiceProviderManager().reloadServices(appContext,(getEditId() == null)?true:false,id, serviceProviderId, oldService);
        if (id != -1) {
            throw new EditChildException(Long.toString(id), getParentId());
        } else {
            path = path.substring(0, (path.length() - (dirName.length() + 1))) + "edit.jsp?editId=" + (editChild ? getEditId() : getParentId());
            throw new CancelChildException(path);
        }
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

    private void deleteRule(String transport) throws SCAGJspException {
        RuleState ruleState = appContext.getRuleManager().getRuleStateAndLock(Long.toString(id), transport);
        if (ruleState.getLocked())
            throw new SCAGJspException(ru.sibinco.scag.Constants.errors.rules.COULD_NOT_REMOVE_RULE_IS_EDITING);
        try {
            appContext.getRuleManager().removeRule(Long.toString(id), transport, RuleManager.NON_TERM_MODE, getLoginedPrincipal().getName());
        } catch (SibincoException se) {
           logger.error("Couldn't remove rule",se);
           throw new SCAGJspException(Constants.errors.rules.COULD_NOT_REMOVE_RULE, se);
        } finally {
            appContext.getRuleManager().unlockRule(Long.toString(id), transport);
        }
    }

    public String getId() {
        return -1 == id ? null : String.valueOf(id);
    }

    public String getName() {
        if(name != null) name.trim();
        return name;
    }

    public void setName(String name) {
        logger.error( "setName:'" + name + "'" );
        this.name = name;
    }

    public String getDescription() {
        if(description != null) description.trim();
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
        logger.error( "setParentId:'" + parentId + "'" );
        this.parentId = parentId;
    }

    public boolean isEditChild() {
        return editChild;
    }

    public void setEditChild(boolean editChild) {
        this.editChild = editChild;
    }

    public String getMbAddSmppRoute() {
        return mbAddSmppRoute;
    }

    public void setMbAddSmppRoute(String mbAddSmppRoute) {
        this.mbAddSmppRoute = mbAddSmppRoute;
    }

    public String getChildEitId() {
        return childEitId;
    }

    public void setChildEitId(String childEitId) {
        this.childEitId = childEitId;
    }

    public List getHttpRuteItems() {
        return httpRuteItems;
    }

    public void setHttpRuteItems(List httpRuteItems) {
        this.httpRuteItems = httpRuteItems;
    }

    public String getMbAddHttpRoute() {
        return mbAddHttpRoute;
    }

    public void setMbAddHttpRoute(String mbAddHttpRoute) {
        this.mbAddHttpRoute = mbAddHttpRoute;
    }

    public int getTotalHttpSize() {
        return totalHttpSize;
    }

    public void setTotalHttpSize(int totalHttpSize) {
        this.totalHttpSize = totalHttpSize;
    }

    public String getMbDeleteHttpRoute() {
        return mbDeleteHttpRoute;
    }

    public void setMbDeleteHttpRoute(String mbDeleteHttpRoute) {
        this.mbDeleteHttpRoute = mbDeleteHttpRoute;
    }

    public String getMbDefaultHttpRoute() {
        return mbDefaultHttpRoute;
    }

    public void setMbDefaultHttpRoute(String mbDefaultHttpRoute) {
        this.mbDefaultHttpRoute = mbDefaultHttpRoute;
    }

}
