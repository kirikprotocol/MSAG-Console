package ru.novosoft.smsc.jsp.smsc.hosts;

/*
 * Created by igork
 * Date: 30.10.2002
 * Time: 2:32:11
 */

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.journal.Actions;
import ru.novosoft.smsc.admin.journal.SubjectTypes;
import ru.novosoft.smsc.jsp.SMSCErrors;
import ru.novosoft.smsc.jsp.smsc.SmscBean;

import javax.servlet.http.HttpServletRequest;

public class HostAdd extends SmscBean
{
  private String hostName = null;
  private int port = -1;
  private String hostServicesFolder = null;
  private String mbSave = null;
  private String mbCancel = null;

  public int process(HttpServletRequest request)
  {
    if (mbCancel != null)
      return RESULT_DONE;

    int result = super.process(request);
    if (result != RESULT_OK)
      return result;

    if (mbSave != null)
      return save();

    if (hostName == null)
      hostName = "";
    if (port <= 0)
      port = 6680;

    return RESULT_OK;
  }

  private int save()
  {
    if (hostName == null || hostName.length() == 0)
      return error(SMSCErrors.error.hosts.hostNotSpecified);

    if (hostsManager.getHostNames().contains(hostName))
      return error(SMSCErrors.error.hosts.hostAlreadyExist, hostName);

    if (port <= 0)
      return error(SMSCErrors.error.hosts.portNotSpecifiedOrIncorrect);

    if (hostServicesFolder == null || hostServicesFolder.length() == 0)
      return error(SMSCErrors.error.hosts.servicesFolderNotSpecified);

    try {
      hostsManager.addHost(hostName, port, hostServicesFolder);
      journalAppend(SubjectTypes.TYPE_host, hostName, Actions.ACTION_ADD);
      appContext.getStatuses().setHostsChanged(true);
    } catch (AdminException e) {
      logger.error("Couldn't add host", e);
      return error(SMSCErrors.error.hosts.couldntAddHost, hostName, e);
    }
    return RESULT_DONE;
  }

  public String getHostName()
  {
    return hostName;
  }

  public void setHostName(String hostName)
  {
    this.hostName = hostName;
  }

  public String getPort()
  {
    return Integer.toString(port);
  }

  public void setPort(String port)
  {
    try {
      this.port = Integer.decode(port).intValue();
    } catch (NumberFormatException e) {
      this.port = 0;
    }
  }

  public String getMbSave()
  {
    return mbSave;
  }

  public void setMbSave(String mbSave)
  {
    this.mbSave = mbSave;
  }

  public String getMbCancel()
  {
    return mbCancel;
  }

  public void setMbCancel(String mbCancel)
  {
    this.mbCancel = mbCancel;
  }

  public String getHostServicesFolder()
  {
    return hostServicesFolder;
  }

  public void setHostServicesFolder(String hostServicesFolder)
  {
    this.hostServicesFolder = hostServicesFolder;
  }
}
