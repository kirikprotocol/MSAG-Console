package ru.novosoft.smsc.mcisme.backend;

import ru.novosoft.smsc.admin.service.Service;
import ru.novosoft.smsc.admin.service.ServiceInfo;
import ru.novosoft.smsc.admin.service.Type;
import ru.novosoft.smsc.admin.AdminException;

import java.util.HashMap;

/**
 * Created by IntelliJ IDEA.
 * User: makar
 * Date: 02.04.2004
 * Time: 16:15:41
 * To change this template use Options | File Templates.
 */
class MCISmeTransport extends Service
{
  private static final String SME_COMPONENT_ID = "MCISme";

  private static final String method_flushStatistics_ID = "flushStatistics";

  public MCISmeTransport(ServiceInfo info) throws AdminException {
    super(info);
  }

  public synchronized void flushStatistics() throws AdminException {
    call(SME_COMPONENT_ID, method_flushStatistics_ID, Type.Types[Type.StringType], new HashMap());
  }

}
