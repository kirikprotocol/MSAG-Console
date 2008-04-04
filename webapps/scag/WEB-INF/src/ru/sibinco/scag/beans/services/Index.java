/*
 * Copyright (c) 2006 SibInco Inc. All Rights Reserved.
 */

package ru.sibinco.scag.beans.services;

import ru.sibinco.scag.backend.service.ServiceProvider;
import ru.sibinco.scag.beans.SCAGJspException;
import ru.sibinco.scag.beans.TabledBeanImpl;
import ru.sibinco.scag.Constants;
import ru.sibinco.lib.SibincoException;

import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;
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

    public void process(HttpServletRequest request, HttpServletResponse response) throws SCAGJspException {

        getFromSession(request);
        request.getSession().setAttribute( Constants.START_POSITION_SI, new Integer(startPosition) );
        logger.debug("services:process():Edit:sp_services=" + request.getSession().getAttribute(Constants.START_POSITION_SI) );

        super.process( request, response);
    }

    void getFromSession( final HttpServletRequest request ){
        String get = null;
        logger.debug( "services:Edit:get_sp_services=" + request.getSession().getAttribute(Constants.GET_FROM_SESSION_START_POSITION_SI) );
        if( request.getSession().getAttribute(Constants.GET_FROM_SESSION_START_POSITION_SI) != null ){
            get = (String)request.getSession().getAttribute(Constants.GET_FROM_SESSION_START_POSITION_SI);
        }
        if( get != null && get.equals("true") ){
            logger.debug( "services:process():Edit:get_sp_services=" + get );
            request.getSession().setAttribute( Constants.GET_FROM_SESSION_START_POSITION_SI, "false" );
            int sp_services_i = 0;
            if( request.getSession().getAttribute(Constants.START_POSITION_SI) != null ){
                logger.debug( "services:process():Edit:get_sp_services=" + request.getSession().getAttribute( Constants.START_POSITION_SI ) );
                Integer inte = (Integer)request.getSession().getAttribute(Constants.START_POSITION_SI);
                sp_services_i = inte.intValue();
                logger.debug( "services:process():Edit:sp_services=" + sp_services_i );
                startPosition = sp_services_i;
            }
        }
    }

    protected void delete() throws SCAGJspException {
        final List toRemove = new ArrayList(checked.length);
        final List serviceIds = new ArrayList();
        if (appContext.getHttpRoutingManager().isRoutesChanged() || appContext.getScagRoutingManager().isRoutesChanged()){
            throw new SCAGJspException(Constants.errors.routing.routes.COULD_NOT_DELETE_SERVICE_PROVIDER);
        } else {
            for (int i = 0; i < checked.length; i++) {
                final String serviceProviderIdStr = checked[i];
                final Long providerId = Long.decode(serviceProviderIdStr);
                toRemove.add(providerId);
                ServiceProvider serviceProvider = (ServiceProvider)
                        appContext.getServiceProviderManager().getServiceProviders().get(providerId);
                for (Iterator iterator = serviceProvider.getServices().keySet().iterator(); iterator.hasNext();) {
                    String serviceIdStr = (iterator.next()).toString();
                    appContext.getRuleManager().removeRulesForService(getLoginedPrincipal().getName(), serviceIdStr);
                    serviceIds.add(serviceIdStr);
                }
            }
            final List toRemoveSmppRoutes = appContext.getScagRoutingManager().getRoteIdsByServiceIds(
                    (String[]) serviceIds.toArray(new String[serviceIds.size()]));
            if (toRemoveSmppRoutes.size() > 0) {
                appContext.getScagRoutingManager().deleteRoutes(getLoginedPrincipal().getName(), toRemoveSmppRoutes);
            }
            final List toRemoveHttpRoutes = appContext.getHttpRoutingManager().getRoteIdsByServiceIds(
                    (String[]) serviceIds.toArray(new String[serviceIds.size()]));
            if (toRemoveHttpRoutes.size() > 0) {
                appContext.getHttpRoutingManager().deleteRoutes(getLoginedPrincipal().getName(), toRemoveHttpRoutes);
            }
            appContext.getServiceProviderManager().deleteServiceProviders(getLoginedPrincipal().getName(), toRemove, appContext);

        }
    }
}
