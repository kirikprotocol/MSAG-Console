package ru.sibinco.smppgw.beans.services.services;

/**
 * Created by IntelliJ IDEA.
 * User: Andrey
 * Date: 22.03.2005
 * Time: 20:21:13
 * To change this template use File | Settings | File Templates.
 */

import ru.sibinco.smppgw.beans.TabledBeanImpl;
import ru.sibinco.smppgw.beans.SmppgwJspException;
import ru.sibinco.smppgw.Constants;
import ru.sibinco.smppgw.backend.sme.GwSmeManager;
import ru.sibinco.lib.bean.TabledBean;
import ru.sibinco.lib.backend.sme.SmeStatus;
import ru.sibinco.lib.SibincoException;

import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;
import java.util.*;
import java.io.IOException;
import java.net.URLEncoder;


public class Index extends TabledBeanImpl implements TabledBean
{
//  public static final int RESULT_VIEW = PRIVATE_RESULT;
//  public static final int RESULT_VIEW_HOST = PRIVATE_RESULT + 1;
//  public static final int RESULT_ADD = PRIVATE_RESULT + 2;
//  public static final int RESULT_EDIT = PRIVATE_RESULT + 3;

  protected String serviceId = null;
  protected String hostId = null;
  protected String[] serviceIds = new String[0];

  protected String mbAddService = null;
  protected String mbDelete = null;
  protected String mbStartService = null;
  protected String mbStopService = null;
  protected String mbView = null;
  protected String mbViewHost = null;
  protected String mbEdit = null;
  protected String mbDisconnectServices = null;
  private String CPATH = "/smppgw";
  protected GwSmeManager smeManager = null;

  public void process(final HttpServletRequest request,final HttpServletResponse response) throws SmppgwJspException
  {
    super.process(request, response);
    smeManager = appContext.getGwSmeManager();

    try {
      if (null != mbAddService)
      {}//   response.sendRedirect("serviceAdd.jsp" + (this.getHostId() != null ? ("?hostName=" + this.getHostId()) : ""));
      //return RESULT_ADD;
      else if (null != mbDelete) {
        if (request.isUserInRole("services"))
        {}//  deleteServices();
        else
         throw new SmppgwJspException(Constants.errors.services.notAuthorizedForDeletingService );
         // return error(Constants.errors.services.notAuthorizedForDeletingService);
      }
      else if (null != mbStartService)
      {}// startServices();
      else if (null != mbStopService)
      {}// stopServices();
      else if (null != mbView)
          response.sendRedirect(CPATH+"/esme_"+URLEncoder.encode(this.getServiceId())+"/index.jsp");//return RESULT_VIEW;
      else if (null != mbViewHost)
          response.sendRedirect(CPATH+"/hosts/hostView.jsp?hostName="+this.getHostId());// return RESULT_VIEW_HOST;
      else if (null != mbEdit)
          response.sendRedirect(CPATH+"/services/serviceEditSme.jsp?serviceId="+URLEncoder.encode(this.getServiceId()));// return RESULT_EDIT;
      else if (null != mbDisconnectServices)
      {}//  disconnectServices();
      else
        return ;
    } catch (IOException e) {
      e.printStackTrace();  //To change body of catch statement use File | Settings | File Templates.
    }
  }

  public Collection getSmeIds()
  {
    if (null == smeManager) {
      try {
        throw new SibincoException( "Service Manager is null!!!");
      } catch (SibincoException e) {
        e.printStackTrace();  //To change body of catch statement use File | Settings | File Templates.
      }
      //error("Service Manager is null!!!");
      return new LinkedList();
    }
    else {
      final List smeIds = smeManager.getSmeNames();
      smeIds.remove(Constants.SMPPGW_SME_ID);
      return smeIds;
    }
  }
 /*
  public boolean isService(final String smeId)
  {
    return hostsManager.isService(smeId);
  }

  public boolean isServiceAdministrable(final String smeId)
  {
    return hostsManager.isServiceAdministarble(smeId);
  }


  public String getHost(final String sId) throws SmppgwJspException
  {
   String Host="";
    try {
      Host= hostsManager.getServiceInfo(sId).getHost();
    } catch (Throwable e) {
      logger.error("Couldn't get service info for service \"" + sId + '"', e);
      throw new SmppgwJspException(Constants.errors.services.couldntGetServiceInfo, e);
    }
    return Host;
  }
 /*
  public byte getServiceStatus(final String serviceId)
  {
    if (hostsManager.isService(serviceId)) {
      try {
        return hostsManager.getServiceInfo(serviceId).getStatus();
      } catch (Throwable t) {
        logger.error("Couldn't get service info for service \"" + serviceId + '"', t);
       // throw new SmppgwJspException(Constants.errors.services.couldntGetServiceInfo, serviceId);
        return ServiceInfo.STATUS_UNKNOWN;
      }
    }
    else
      return ServiceInfo.STATUS_RUNNING;
  }
   */
  public SmeStatus getSmeStatus(final String id)
  {
    try {
      return appContext.getGateway().getSmeStatus(id);
    } catch (SibincoException e) {
      logger.error("Couldn't get sme status for service \"" + id + "\", nested:" + e.getMessage());
     // throw new SmppgwJspException(Constants.errors.services.couldntGetServiceInfo, id);
      return null;
    }
  }
  /*
  public boolean isServiceDisabled(final String serviceId)
  {
    try {
      return appContext.getSmeHostsManager().get(serviceId).isDisabled();
    } catch (SibincoException e) {
     // throw new SmppgwJspException(Constants.errors.services.couldntGetServiceInfo, serviceId);
      return false;
    }
  }
    */
  public boolean isServiceConnected(final String serviceId)
  {
    try {
      final SmeStatus smeStatus = appContext.getGateway().getSmeStatus(serviceId);
      if (null != smeStatus)
        return smeStatus.isConnected();
      else {
        logger.debug("SME ID \"" + serviceId + "\" not found");
        return false;
      }
    } catch (SibincoException e) {
     // throw new SmppgwJspException(Constants.errors.services.couldntGetServiceInfo, serviceId);
      return false;
    }
  }
 /*
  public boolean isSmppgwAlive()
  {
    try {
      return ServiceInfo.STATUS_RUNNING == hostsManager.getServiceInfo(Constants.SMPPGW_SME_ID).getStatus();
    } catch (SibincoException e) {
    // throw new SmppgwJspException(Constants.errors.services.couldntGetServiceInfo, Constants.SMPPGW_SME_ID);
      return false;
    }
  }  */
  /**
     * ********************* Command handlers ***************************
     */
   /*
    protected int deleteServices()
    {
      final List notRemoved = new LinkedList();
      for (int i = 0; i < serviceIds.length; i++) {
        final String id = serviceIds[i];
        try {
          if (hostsManager.isService(id)) {
            hostsManager.removeService(id);
            final String roleName = appContext.getWebXmlConfig().removeSecurityConstraint(id);
            appContext.getStatuses().setWebXmlChanged(true);
            journalAppend(SubjectTypes.TYPE_securityConstraint, id, Actions.ACTION_DEL);
            appContext.getStatuses().setWebXmlChanged(true);
            appContext.getUserManager().removeRole(roleName);
            journalAppend(SubjectTypes.TYPE_user, null, Actions.ACTION_MODIFY);
            try {
              logger.debug("Removed security constraint for service \"" + id + "\" with role \"" + roleName + "\", saving changes to WEB-INF/web.xml...");
              appContext.getWebXmlConfig().save();
              appContext.getJournal().clear(SubjectTypes.TYPE_securityConstraint);
              appContext.getStatuses().setWebXmlChanged(false);
            } catch (Throwable e) {
              error(SMSCErrors.error.services.couldntSaveWebXml, e);
              logger.error("Could not save WEB-INF/web.xml", e);
            }
          }
          else
            hostsManager.removeSme(id);

          journalAppend(SubjectTypes.TYPE_service, id, Actions.ACTION_DEL);
          //appContext.getStatuses().setServicesChanged(true);
        } catch (Throwable e) {
          error(SMSCErrors.error.services.coudntDeleteService, id);
          logger.error("Couldn't delete service \"" + id + '"', e);
          notRemoved.add(id);
        }
      }
      serviceIds = (String[]) notRemoved.toArray(new String[0]);
      return 0 == Constants.errors.size() ? RESULT_DONE : RESULT_ERROR;
    }

    protected int startServices()
    {
      logger.debug("startServices: " + (null != serviceIds ? serviceIds.length : 0));

      int result = RESULT_OK;
      if (0 == serviceIds.length)
        return warning(SMSCErrors.warning.hosts.noServicesSelected);

      final List notStartedIds = new LinkedList();

      for (int i = 0; i < serviceIds.length; i++) {
        final String svcId = serviceIds[i];
        if (hostsManager.isService(svcId)) {
          try {
            if (ServiceInfo.STATUS_STOPPED == hostsManager.getServic      eInfo(svcId).getStatus()) {
              hostsManager.startService(svcId);
            }
            else
              logger.debug("startServices: " + svcId + " is " + hostsManager.getServiceInfo(svcId).getStatusStr());
          } catch (SibincoException e) {
            notStartedIds.add(svcId);
            result = error(SMSCErrors.error.hosts.couldntStartService, svcId, e);
            logger.error("Couldn't start services \"" + svcId + '"', e);
            continue;
          }
        }
        else {
          notStartedIds.add(svcId);
          result = error(SMSCErrors.error.services.couldntStartInternalService, svcId);
        }
      }
      serviceIds = (String[]) notStartedIds.toArray(new String[0]);
      return result;
    }

    protected int stopServices()
    {
      logger.debug("stopServices: " + (null != serviceIds ? serviceIds.length : 0));

      int result = RESULT_OK;

      if (0 == serviceIds.length)
        return warning(SMSCErrors.warning.hosts.noServicesSelected);

      final List notStoppedIds = new LinkedList();

      for (int i = 0; i < serviceIds.length; i++) {
        final String svcId = serviceIds[i];
        try {
          if (ServiceInfo.STATUS_RUNNING == hostsManager.getServiceInfo(svcId).getStatus())
            hostsManager.shutdownService(svcId);
          else
            logger.debug("stopServices: " + svcId + " is " + hostsManager.getServiceInfo(svcId).getStatusStr());
        } catch (SibincoException e) {
          notStoppedIds.add(svcId);
          result = error(SMSCErrors.error.hosts.couldntStopService, svcId);
          logger.error("Couldn't stop services \"" + svcId + '"', e);
        }
      }
      serviceIds = (String[]) notStoppedIds.toArray(new String[0]);
      return result;
    }

    private int disconnectServices()
    {
      logger.debug("disconnectServices: " + (null != serviceIds ? serviceIds.length : 0));

      int result = RESULT_OK;

      if (0 == serviceIds.length)
        return warning(SMSCErrors.warning.hosts.noServicesSelected);

      try {
        appContext.getSmeHostsManager().disconnectSmes(Arrays.asList(serviceIds));
      } catch (SibincoException e) {
        result = error(SMSCErrors.error.hosts.couldntStopService, serviceId);
        logger.error("Couldn't disconnect services \"" + serviceIds + '"', e);
      }
      serviceIds = new String[0];
      return result;
    }
    */

  /**
   * ************************ Properties ******************************
   */


  public String getServiceId()
  {
    return serviceId;
  }

  public void setServiceId(final String serviceId)
  {
    this.serviceId = serviceId;
  }

  public String getHostId()
  {
    return hostId;
  }

  public void setHostId(final String hostId)
  {
    this.hostId = hostId;
  }

  public String[] getServiceIds()
  {
    return serviceIds;
  }

  public void setServiceIds(final String[] serviceIds)
  {
    this.serviceIds = serviceIds;
  }

  public String getMbAddService()
  {
    return mbAddService;
  }

  public void setMbAddService(final String mbAddService)
  {
    this.mbAddService = mbAddService;
  }

  public String getMbDelete()
  {
    return mbDelete;
  }

  public void setMbDelete(final String mbDelete)
  {
    this.mbDelete = mbDelete;
  }

  public String getMbStartService()
  {
    return mbStartService;
  }

  public void setMbStartService(final String mbStartService)
  {
    this.mbStartService = mbStartService;
  }

  public String getMbStopService()
  {
    return mbStopService;
  }

  public void setMbStopService(final String mbStopService)
  {
    this.mbStopService = mbStopService;
  }

  public String getMbView()
  {
    return mbView;
  }

  public void setMbView(final String mbView)
  {
    this.mbView = mbView;
  }

  public String getMbViewHost()
  {
    return mbViewHost;
  }

  public void setMbViewHost(final String mbViewHost)
  {
    this.mbViewHost = mbViewHost;
  }

  public String getMbEdit()
  {
    return mbEdit;
  }

  public void setMbEdit(final String mbEdit)
  {
    this.mbEdit = mbEdit;
  }

  public String getMbDisconnectServices()
  {
    return mbDisconnectServices;
  }

  public void setMbDisconnectServices(final String mbDisconnectServices)
  {
    this.mbDisconnectServices = mbDisconnectServices;
  }
   protected Collection getDataSource()
  {
    return new ArrayList();//appContext.getGwRoutingManager().getRoutes().values();
  }

  protected void delete()
  {
   // appContext.getGwRoutingManager().getRoutes().keySet().removeAll(checkedSet);
   // appContext.getStatuses().setRoutesChanged(true);
  }
}
