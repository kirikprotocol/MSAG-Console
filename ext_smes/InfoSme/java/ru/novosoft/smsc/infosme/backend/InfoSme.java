package ru.novosoft.smsc.infosme.backend;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.service.ServiceInfo;
import ru.novosoft.smsc.jsp.SMSCAppContext;
import ru.novosoft.smsc.infosme.backend.siebel.SiebelDataProvider;
import ru.novosoft.smsc.infosme.backend.siebel.SiebelTaskManager;
import ru.novosoft.smsc.infosme.backend.siebel.impl.SiebelDataProviderImpl;

import java.io.File;

/**
 * Created by IntelliJ IDEA.
 * User: igork
 * Date: 24.09.2003
 * Time: 17:20:59
 */
public class InfoSme extends InfoSmeTransport
{
  public InfoSme(ServiceInfo info, String host, int port) throws AdminException
  {
    super(info, host, port);
  }

  public void updateInfo(SMSCAppContext appContext) throws AdminException
  {
    setInfo(appContext.getHostsManager().getServiceInfo(getInfo().getId()));
  }
}
