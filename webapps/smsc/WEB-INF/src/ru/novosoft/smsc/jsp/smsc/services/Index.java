package ru.novosoft.smsc.jsp.smsc.services;

/*
 * Created by igork
 * Date: 23.10.2002
 * Time: 19:49:14
 */

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.Constants;
import ru.novosoft.smsc.admin.journal.Actions;
import ru.novosoft.smsc.admin.journal.SubjectTypes;
import ru.novosoft.smsc.admin.route.SmeStatus;
import ru.novosoft.smsc.admin.service.ServiceInfo;
import ru.novosoft.smsc.jsp.PageBean;
import ru.novosoft.smsc.jsp.SMSCErrors;

import javax.servlet.http.HttpServletRequest;
import java.util.*;


public class Index extends PageBean
{
  public static final int RESULT_VIEW = PRIVATE_RESULT;
  public static final int RESULT_VIEW_HOST = PRIVATE_RESULT + 1;
  public static final int RESULT_ADD = PRIVATE_RESULT + 2;
  public static final int RESULT_EDIT = PRIVATE_RESULT + 3;

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


  public int process(final HttpServletRequest request)
  {
    final int result = super.process(request);
    if (RESULT_OK != result)
      return result;

    if (null != mbAddService)
      return RESULT_ADD;
    else if (null != mbDelete) {
      if (request.isUserInRole("services"))
        return deleteServices();
      else
        return error(SMSCErrors.error.services.notAuthorizedForDeletingService);
    }
    else if (null != mbStartService)
      return startServices();
    else if (null != mbStopService)
      return stopServices();
    else if (null != mbView)
      return RESULT_VIEW;
    else if (null != mbViewHost)
      return RESULT_VIEW_HOST;
    else if (null != mbEdit)
      return RESULT_EDIT;
    else if (null != mbDisconnectServices)
      return disconnectServices();
    else
      return RESULT_OK;
  }

  public Collection getSmeIds()
  {
    if (null == hostsManager) {
      error(SMSCErrors.error.services.serviceManagerIsNull);
      return new LinkedList();
    }
    else {
      final List smeIds = hostsManager.getSmeIds();
		for (Iterator i = smeIds.iterator(); i.hasNext();)
		{
			final String smeName = (String) i.next();
			if (smeName.equals(Constants.SMSC_SME_ID))
			{smeIds.remove(smeName);}
		}
      return smeIds;
    }
  }

  public boolean isService(final String smeId)
  {
    return hostsManager.isService(smeId);
  }

  public boolean isServiceAdministrable(final String smeId)
  {
    return hostsManager.isServiceAdministrable(smeId);
  }

  /**
   * ********************* Command handlers ***************************
   */

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
    return 0 == errors.size() ? RESULT_DONE : RESULT_ERROR;
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
          if (!hostsManager.getServiceInfo(svcId).isOnline()) {
            hostsManager.startService(svcId);
          }
          else
            logger.debug("startServices: " + svcId + " is " + hostsManager.getServiceInfo(svcId).getStatusStr());
        } catch (AdminException e) {
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
        if (hostsManager.getServiceInfo(svcId).isOnline())
          hostsManager.shutdownService(svcId);
        else
          logger.debug("stopServices: " + svcId + " is " + hostsManager.getServiceInfo(svcId).getStatusStr());
      } catch (AdminException e) {
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
      appContext.getSmeManager().disconnectSmes(Arrays.asList(serviceIds));
    } catch (AdminException e) {
      result = error(SMSCErrors.error.hosts.couldntStopService, serviceId);
      logger.error("Couldn't disconnect services \"" + serviceIds + '"', e);
    }
    serviceIds = new String[0];
    return result;
  }


  public String getHost(final String sId)
  {
    try {
      return hostsManager.getServiceInfo(sId).getHost();
    } catch (Throwable e) {
      error(SMSCErrors.error.services.couldntGetServiceInfo, e);
      logger.error("Couldn't get service info for service \"" + sId + '"', e);
      return "";
    }
  }

  public SmeStatus getSmeStatus(final String id)
  {
    try {
      return appContext.getSmeManager().smeStatus(id);
    } catch (AdminException e) {
      logger.error("Couldn't get sme status for service \"" + id + "\", nested:" + e.getMessage());
      error(SMSCErrors.error.services.couldntGetServiceInfo, id);
      return null;
    }
  }

  public boolean isServiceDisabled(final String serviceId)
  {
    try {
      return appContext.getSmeManager().get(serviceId).isDisabled();
    } catch (AdminException e) {
      error(SMSCErrors.error.services.couldntGetServiceInfo, serviceId);
      return false;
    }
  }

  public boolean isServiceConnected(final String serviceId)
  {
    try {
      final SmeStatus smeStatus = appContext.getSmeManager().smeStatus(serviceId);
      if (null != smeStatus)
        return smeStatus.isConnected();
      else {
        logger.debug("SME ID \"" + serviceId + "\" not found");
        return false;
      }
    } catch (AdminException e) {
      error(SMSCErrors.error.services.couldntGetServiceInfo, serviceId);
      return false;
    }
  }

  public boolean isSmscAlive()
  {
    try {
      return hostsManager.getServiceInfo(Constants.SMSC_SME_ID).isOnline();
    } catch (AdminException e) {
      error(SMSCErrors.error.services.couldntGetServiceInfo, Constants.SMSC_SME_ID);
      return false;
    }
  }

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
}
