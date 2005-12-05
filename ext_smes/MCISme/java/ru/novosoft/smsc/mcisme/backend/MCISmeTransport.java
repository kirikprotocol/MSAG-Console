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

  private static final String method_flushStatistics_ID = "flushStatistics";
  private static final String method_getStatistics_ID = "getStatistics";
  private static final String method_getRuntime_ID = "getRuntime";

  public MCISmeTransport(ServiceInfo info, String host, int port) throws AdminException {
    super(info, port);
    this.host = host;
  }

  private long[] decode(String str, int valuesCount) throws AdminException
  {
    int elCount = 0; long el[] = new long[valuesCount];
    StringTokenizer st = new StringTokenizer(str, ",");
    try {
      while (st.hasMoreTokens() && elCount<valuesCount) {
        String value = st.nextToken();
        if (value == null) throw new Exception("Token value is null");
        el[elCount++] = Long.parseLong(value);
      }
      if (elCount != valuesCount) new Exception("Invalid tokens count");
    } catch (Exception exc) {
      throw new AdminException("Invalid protocol string format. "+exc.getMessage());
    }
    return el;
  }
  private CountersSet decodeStatisticsString(String str) throws AdminException {
    long el[] = decode(str, 4);
    return new CountersSet(el[0], el[1], el[2], el[3]);
  }
  private RuntimeSet decodeRuntimeString(String str) throws AdminException {
    long el[] = decode(str, 5);
    return new RuntimeSet(el[0], el[1], el[2], el[3], el[4]);
  }

  public synchronized void flushStatistics() throws AdminException {
    call(SME_COMPONENT_ID, method_flushStatistics_ID, Type.Types[Type.StringType], new HashMap());
  }
  public synchronized CountersSet getStatistics() throws AdminException {
    Object obj = call(SME_COMPONENT_ID, method_getStatistics_ID, Type.Types[Type.StringType], new HashMap());
    return (obj != null && obj instanceof String) ? decodeStatisticsString((String)obj):new CountersSet();
  }
  public synchronized RuntimeSet getRuntime() throws AdminException {
    Object obj = call(SME_COMPONENT_ID, method_getRuntime_ID, Type.Types[Type.StringType], new HashMap());
    return (obj != null && obj instanceof String) ? decodeRuntimeString((String)obj):new RuntimeSet();
  }

}
