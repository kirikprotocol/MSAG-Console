package ru.novosoft.smsc.jsp.smsc.hosts;

/*
 * Created by igork
 * Date: 23.10.2002
 * Time: 21:16:20
 */

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.journal.Actions;
import ru.novosoft.smsc.admin.journal.SubjectTypes;
import ru.novosoft.smsc.jsp.SMSCErrors;
import ru.novosoft.smsc.jsp.smsc.SmscBean;

import javax.servlet.http.HttpServletRequest;
import java.util.Collection;
import java.util.LinkedList;
import java.util.List;

public class Index extends SmscBean
{
  public static final int RESULT_ADD = PRIVATE_RESULT;
  public static final int RESULT_VIEW = PRIVATE_RESULT + 1;

  protected String mbAdd = null;
  protected String mbView = null;
  protected String mbDelete = null;
  protected String hostName = null;
  protected String[] hostIds = new String[0];

  public int process(HttpServletRequest request)
  {
    int result = super.process(request);
    if (result != RESULT_OK)
      return result;

    if (mbAdd != null)
      return RESULT_ADD;
    else if (mbView != null)
      return RESULT_VIEW;
    else if (mbDelete != null)
      return deleteHost();
    else
      return RESULT_OK;
  }

  protected int deleteHost()
  {
    if (hostIds == null || hostIds.length == 0)
      return error(SMSCErrors.error.hosts.hostsNotSpecified);

    List notRemovedIds = new LinkedList();

    for (int i = 0; i < hostIds.length; i++) {
      String id = hostIds[i];

      try {
        hostsManager.removeHost(id);
        journalAppend(SubjectTypes.TYPE_host, id, Actions.ACTION_DEL);
        appContext.getStatuses().setHostsChanged(true);
      } catch (AdminException e) {
        logger.error("Couldn't remove host \"" + id + '"', e);
        notRemovedIds.add(id);
        error(SMSCErrors.error.hosts.couldntRemoveHost, id);
      }
    }

    hostIds = (String[]) notRemovedIds.toArray(new String[0]);

    if (errors.size() == 0)
      return RESULT_DONE;
    else
      return RESULT_ERROR;
  }

  public Collection getHostNames()
  {
    return hostsManager.getHostNames();
  }

  public int getHostPort(String hostName)
  {
    try {
      return hostsManager.getHostPort(hostName);
    } catch (AdminException e) {
      logger.error("Couldn't get host \"" + hostName + "\" port");
      return -1;
    }
  }

  public int getServicesTotal(String hostName)
  {
    try {
      return hostsManager.getCountServices(hostName);
    } catch (AdminException e) {
      logger.warn("host \"" + hostName + "\" not found", e);
    }
    return 0;
  }

  public int getServicesRunning(String hostName)
  {
    try {
      return hostsManager.getCountRunningServices(hostName);
    } catch (AdminException e) {
      logger.warn("host \"" + hostName + "\" not found", e);
    }
    return 0;
  }

  public String getMbAdd()
  {
    return mbAdd;
  }

  public void setMbAdd(String mbAdd)
  {
    this.mbAdd = mbAdd;
  }

  public String getMbView()
  {
    return mbView;
  }

  public void setMbView(String mbView)
  {
    this.mbView = mbView;
  }

  public String getHostName()
  {
    return hostName;
  }

  public void setHostName(String hostName)
  {
    this.hostName = hostName;
  }

  public String getMbDelete()
  {
    return mbDelete;
  }

  public void setMbDelete(String mbDelete)
  {
    this.mbDelete = mbDelete;
  }

  public String[] getHostIds()
  {
    return hostIds;
  }

  public void setHostIds(String[] hostIds)
  {
    this.hostIds = hostIds;
  }
}
