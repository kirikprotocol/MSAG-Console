package ru.novosoft.smsc.mcisme.backend;

import ru.novosoft.smsc.admin.service.ServiceInfo;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.jsp.SMSCAppContext;

/**
 * Created by IntelliJ IDEA.
 * User: makar
 * Date: 02.04.2004
 * Time: 16:18:22
 * To change this template use Options | File Templates.
 */
public class MCISme extends MCISmeTransport
{
  public MCISme(final ServiceInfo info, String host, final int port) throws AdminException {
    super(info, host, port);
  }

  public void updateInfo(final SMSCAppContext appContext) throws AdminException {
    setInfo(appContext.getHostsManager().getServiceInfo(getInfo().getId()));
  }
}
