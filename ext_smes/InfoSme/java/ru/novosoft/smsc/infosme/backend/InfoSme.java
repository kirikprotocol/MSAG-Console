package ru.novosoft.smsc.infosme.backend;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.service.ServiceInfo;

/**
 * Created by IntelliJ IDEA.
 * User: igork
 * Date: 24.09.2003
 * Time: 17:20:59
 */
public class InfoSme extends InfoSmeTransport
{
  public InfoSme(ServiceInfo info) throws AdminException
  {
    super(info);
  }

}
