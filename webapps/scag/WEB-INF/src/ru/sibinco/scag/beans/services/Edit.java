package ru.sibinco.scag.beans.services;

import ru.sibinco.scag.Constants;
import ru.sibinco.scag.backend.SCAGAppContext;
import ru.sibinco.scag.backend.service.ServiceProvider;
import ru.sibinco.scag.backend.service.ServiceProvidersManager;
import ru.sibinco.scag.beans.*;
import ru.sibinco.scag.web.security.AuthFilter;
import ru.sibinco.scag.web.security.UserLoginData;

import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;
import javax.servlet.http.HttpSession;
import java.util.*;

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

        getFromSession(request);
        storeToSessionSP(request);

        if (appContext == null) {
            appContext = (SCAGAppContext) request.getAttribute(Constants.APP_CONTEXT);
        }

        HttpSession session = request.getSession();
        UserLoginData userLoginData = (UserLoginData) session.getAttribute(AuthFilter.USER_LOGIN_DATA);
        userLogin = userLoginData.getName();

        if (mbCancel != null && !mbCancel.isEmpty()) {
            storeToSessionGetFlagParent( request, Constants.GSP_TRUE );
            throw new CancelException();
        } else if (mbSave != null && !mbSave.isEmpty()) {
            storeToSessionGetFlagParent( request, Constants.GSP_TRUE );
            save();
        } else if (mbAddChild != null && !mbAddChild.isEmpty()) {
            throw new AddChildException("service", Long.toString(id));
        }
        load();
        if (mbDelete != null && !mbDelete.isEmpty()) {
          userName = userLoginData.getName();
        }
        if (!isAdd()) {
            super.process(request, response);
        }
    }

    void getFromSession( final HttpServletRequest request ){
        String get = null;
        logger.debug( "services/Edit:getFromSession():start:get_sp_services=" + request.getSession().getAttribute(Constants.GET_FROM_SESSION_START_POSITION_S) );
        if( request.getSession().getAttribute(Constants.GET_FROM_SESSION_START_POSITION_S) != null ){
            get = (String)request.getSession().getAttribute(Constants.GET_FROM_SESSION_START_POSITION_S);
        }
        if( get != null && get.equals(Constants.GSP_TRUE) ){
            request.getSession().setAttribute(Constants.GET_FROM_SESSION_START_POSITION_S, Constants.GSP_FALSE );
            logger.debug( "services/Edit:getFromSession():Edit:set top false get_sp_services=" +
                           request.getSession().getAttribute(Constants.GET_FROM_SESSION_START_POSITION_S) );
            int sp_services = 0;
            if( request.getSession().getAttribute(Constants.START_POSITION_S) != null ){
                logger.debug( "services/Edit:getFromSesion():sp_services=" + request.getSession().getAttribute( Constants.START_POSITION_S ) );
                Integer inte = (Integer)request.getSession().getAttribute(Constants.START_POSITION_S);
                sp_services = inte.intValue();
                logger.debug( "services/Edit:getFromSesion():sp_services=" + sp_services );
                if(sp_services!=-1){
                    startPosition = sp_services;
                }
            }
        }
    }


    void storeToSessionGetFlagParent( final HttpServletRequest request, String value ){
        request.getSession().setAttribute(Constants.GET_FROM_SESSION_START_POSITION_SI, value );
        logger.debug("services/Edit:storeToSessionGetFlag():"  +
                      request.getSession().getAttribute(Constants.GET_FROM_SESSION_START_POSITION_SI) );
    }

    void storeToSessionSP( final HttpServletRequest request ){
        request.getSession().setAttribute( Constants.START_POSITION_S, new Integer(startPosition) );
        logger.debug("services:process():Edit:sp_services=" + request.getSession().getAttribute(Constants.START_POSITION_S) );
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
                appContext.getRuleManager().removeRulesForService(getUserName(), serviceIdStr);
            }
            final List toRemoveSmppRoutes = appContext.getScagRoutingManager().getRoteIdsByServiceIds(checked);
            final List toRemoveHttpRoutes = appContext.getHttpRoutingManager().getRoteIdsByServiceIds(checked);
            if (toRemoveSmppRoutes.size() > 0) {
                appContext.getScagRoutingManager().deleteRoutes(getUserName(),
                        toRemoveSmppRoutes);
            }
            if (toRemoveHttpRoutes.size() > 0) {
                appContext.getHttpRoutingManager().deleteRoutes(getUserName(),
                        toRemoveSmppRoutes);
            }
            appContext.getServiceProviderManager().deleteServices(getUserName(),
                    toRemove, serviceProvider, appContext);
        }
    }

    protected void load() throws SCAGJspException {
        if (!isAdd() && getTabledItems() != null) {
            logger.info( "SERVICES:LOAD: " + getEditId() );
            final Map serviceProviders = appContext.getServiceProviderManager().getServiceProviders();
            final Long longLoadId = Long.decode(getEditId());
            if (!serviceProviders.containsKey(longLoadId)){
                logger.debug( "services/Edit: load(): longLoadId=" + longLoadId );
                throw new SCAGJspException(Constants.errors.serviceProviders.SERVICE_PROVIDER_NOT_FOUND, getEditId()+" ss");
            }
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
      if (logger.isDebugEnabled()) logger.debug("id: "+id);
      if (id == null || id.isEmpty()){
        this.id = -1;
        return;
      }

      Long decoded = Long.decode(id);
      if (decoded == null){
        this.id = -1;
        return;
      }

      this.id = decoded;
    }

    public String getName() {
        if (name != null) name = name.trim();
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
        if (logger.isDebugEnabled()) logger.debug("Set 'add' to '"+add+"'.");
        this.add = add;
    }

    public String getMbSave() {
        return mbSave;
    }

    public void setMbSave(String mbSave) {
        if (logger.isDebugEnabled()) logger.debug("Set 'mbSave' to '"+mbSave+"'.");
        this.mbSave = mbSave;
    }

    public String getMbCancel() {
        return mbCancel;
    }

    public void setMbCancel(String mbCancel) {
        if (logger.isDebugEnabled()) logger.debug("Set 'mbCancel' to '"+mbCancel+"'.");
        this.mbCancel = mbCancel;
    }

    public String getMbAddChild() {
        return mbAddChild;
    }

    public void setMbAddChild(String mbAddChild) {
        if (logger.isDebugEnabled()) logger.debug("Set 'mbAddChild' to '"+mbAddChild+"'.");
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
        if (logger.isDebugEnabled()) logger.debug("Set 'childEditId' to '"+childEitId+"'.");
        this.childEitId = childEitId;
    }


}