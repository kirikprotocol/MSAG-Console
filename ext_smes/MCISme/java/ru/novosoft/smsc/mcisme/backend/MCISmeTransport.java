package ru.novosoft.smsc.mcisme.backend;

import ru.novosoft.smsc.admin.service.Service;
import ru.novosoft.smsc.admin.service.ServiceInfo;
import ru.novosoft.smsc.admin.service.Type;
import ru.novosoft.smsc.admin.AdminException;

import java.util.HashMap;
import java.util.StringTokenizer;

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

  private static final String method_getStatistics_ID       = "getStatistics";
  private static final String method_flushStatistics_ID     = "flushStatistics";
  private static final String method_getActiveTasksCount_ID = "getActiveTasksCount";

  public MCISmeTransport(ServiceInfo info) throws AdminException {
    super(info);
  }

  private CountersSet decodeStatisticsString(String str) throws AdminException
  {
    int elCount = 0; long el[] = new long[4];
    StringTokenizer st = new StringTokenizer(str, ",");
    try {
      while (st.hasMoreTokens() && elCount<4) {
        String value = st.nextToken();
        if (value == null) throw new Exception("Token value is null");
        el[elCount++] = Long.parseLong(value);
      }
      if (elCount != 4) new Exception("Invalid tokens count");
    } catch (Exception exc) {
      throw new AdminException("Invalid statistics string format. "+exc.getMessage());
    }
    return new CountersSet(el[0], el[1], el[2], el[3]);
  }

  public synchronized void flushStatistics() throws AdminException {
    call(SME_COMPONENT_ID, method_flushStatistics_ID, Type.Types[Type.StringType], new HashMap());
  }
  public synchronized CountersSet getStatistics() throws AdminException {
    Object obj = call(SME_COMPONENT_ID, method_getStatistics_ID, Type.Types[Type.StringType], new HashMap());
    return (obj != null && obj instanceof String) ? decodeStatisticsString((String)obj):new CountersSet();
  }
  public synchronized long getActiveTasksCount() throws AdminException {
    Object obj = call(SME_COMPONENT_ID, method_getActiveTasksCount_ID, Type.Types[Type.IntType], new HashMap());
    return (obj != null && obj instanceof Long) ? ((Long)obj).longValue():0;
  }

}
