/*
 * Created by igork
 * Date: 23.10.2002
 * Time: 22:50:27
 */
package ru.novosoft.smsc.jsp.smsc.hosts;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.Constants;
import ru.novosoft.smsc.admin.journal.Actions;
import ru.novosoft.smsc.admin.journal.SubjectTypes;
import ru.novosoft.smsc.admin.service.ServiceInfo;
import ru.novosoft.smsc.jsp.SMSCErrors;
import ru.novosoft.smsc.jsp.smsc.SmscBean;

import javax.servlet.http.HttpServletRequest;
import java.util.*;


public class HostView extends SmscBean
{
  public static final int RESULT_VIEW = PRIVATE_RESULT;
  public static final int RESULT_EDIT = PRIVATE_RESULT + 1;
  public static final int RESULT_ADD_SERVICE = PRIVATE_RESULT + 2;
  public static final int RESULT_EDIT_SERVICE = PRIVATE_RESULT + 3;

  protected String hostName = null;
  protected String[] serviceIds = new String[0];
  protected String serviceId = null;

  protected Map services = null;

  protected String mbView = null;
  protected String mbEdit = null;
  protected String mbCancel = null;
  protected String mbAddService = null;
  protected String mbDelete = null;
  protected String mbStartService = null;
  protected String mbStopService = null;
  protected String mbEditService = null;

  protected int init(final List errors)
  {
    final int result = super.init(errors);
    if (RESULT_OK != result)
      return result;

    try {
      services = hostsManager.getServices(hostName);
      services.remove(Constants.SMSC_SME_ID);
    } catch (AdminException e) {
      services = new HashMap();
      return warning(SMSCErrors.warning.hosts.listFailed, hostName);
    }

    return RESULT_OK;
  }

  public int process(final HttpServletRequest request)
  {
    final int result = super.process(request);
    if (RESULT_OK != result)
      return result;

    if (null == serviceIds)
      serviceIds = new String[0];

    if (null != mbAddService) {
      return RESULT_ADD_SERVICE;
    } else if (null != mbCancel) {
      return RESULT_DONE;
    } else if (null != mbEdit) {
      return RESULT_EDIT;
    } else if (null != mbDelete) {
      return deleteServices();
    } else if (null != mbStartService) {
      return startServices();
    } else if (null != mbStopService) {
      return stopServices();
    } else if (null != mbEditService) {
      return RESULT_EDIT_SERVICE;
    } else if (null != mbView) {
      return RESULT_VIEW;
    } else
      return RESULT_OK;
  }

  private int deleteServices()
  {
    if (0 == serviceIds.length)
      return RESULT_OK;

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
        } else
          hostsManager.removeSme(id);
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
    int result = RESULT_OK;
    if (0 == serviceIds.length)
      return warning(SMSCErrors.warning.hosts.noServicesSelected);

    final List notStartedIds = new LinkedList();

    for (int i = 0; i < serviceIds.length; i++) {
      final ServiceInfo s = (ServiceInfo) services.get(serviceIds[i]);
      if (null == s)
        result = error(SMSCErrors.error.hosts.serviceNotFound, serviceIds[i]);
      else if (ServiceInfo.STATUS_STOPPED == s.getStatus()) {
        try {
          hostsManager.startService(serviceIds[i]);
        } catch (AdminException e) {
          notStartedIds.add(serviceIds[i]);
          result = error(SMSCErrors.error.hosts.couldntStartService, serviceIds[i], e);
          logger.error("Couldn't start services \"" + serviceIds[i] + '"', e);
          continue;
        }
      }
    }
    serviceIds = (String[]) notStartedIds.toArray(new String[0]);
    return result;
  }

  protected int stopServices()
  {
    int result = RESULT_OK;

    if (0 == serviceIds.length)
      return warning(SMSCErrors.warning.hosts.noServicesSelected);

    final List notStoppedIds = new LinkedList();

    for (int i = 0; i < serviceIds.length; i++) {
      final ServiceInfo s = (ServiceInfo) services.get(serviceIds[i]);
      if (null == s)
        result = error(SMSCErrors.error.hosts.serviceNotFound, serviceIds[i]);
      else if (ServiceInfo.STATUS_RUNNING == s.getStatus()) {
        try {
          hostsManager.shutdownService(serviceIds[i]);
        } catch (AdminException e) {
          notStoppedIds.add(serviceIds[i]);
          result = error(SMSCErrors.error.hosts.couldntStopService, serviceIds[i]);
          logger.error("Couldn't stop services \"" + serviceIds[i] + '"', e);
        }
      }
    }
    serviceIds = (String[]) notStoppedIds.toArray(new String[0]);
    return result;
  }

  public int getServicesTotal()
  {
    if (null != hostName && 0 < hostName.length()) {
      try {
        return hostsManager.getCountServices(hostName);
      } catch (AdminException e) {
        logger.warn("host \"" + hostName + "\" not found", e);
      }
    }
    return 0;
  }

  public int getServicesRunning()
  {
    if (null != hostName && 0 < hostName.length()) {
      try {
        return hostsManager.getCountRunningServices(hostName);
      } catch (AdminException e) {
        logger.warn("host \"" + hostName + "\" not found", e);
      }
    }
    return 0;
  }


  public Collection getServices()
  {
    return services.values();
  }

  public int getPort()
  {
    try {
      return hostsManager.getHostPort(hostName);
    } catch (AdminException e) {
      logger.error("Couldn't get port for host \"" + hostName + "\"", e);
      return -1;
    }
  }

  public String getHostName()
  {
    return hostName;
  }

  public void setHostName(final String hostName)
  {
    this.hostName = hostName;
  }

  public String[] getServiceIds()
  {
    return serviceIds;
  }

  public void setServiceIds(final String[] serviceIds)
  {
    this.serviceIds = serviceIds;
  }

  public String getMbView()
  {
    return mbView;
  }

  public void setMbView(final String mbView)
  {
    this.mbView = mbView;
  }

  public String getMbEdit()
  {
    return mbEdit;
  }

  public void setMbEdit(final String mbEdit)
  {
    this.mbEdit = mbEdit;
  }

  public String getMbCancel()
  {
    return mbCancel;
  }

  public void setMbCancel(final String mbCancel)
  {
    this.mbCancel = mbCancel;
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

  public String getServiceId()
  {
    return serviceId;
  }

  public void setServiceId(final String serviceId)
  {
    this.serviceId = serviceId;
  }

  public String getMbEditService()
  {
    return mbEditService;
  }

  public void setMbEditService(final String mbEditService)
  {
    this.mbEditService = mbEditService;
  }
}
