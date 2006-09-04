package ru.novosoft.smsc.mcisme.backend;

import ru.novosoft.smsc.admin.service.Service;
import ru.novosoft.smsc.admin.service.ServiceInfo;
import ru.novosoft.smsc.admin.service.Type;
import ru.novosoft.smsc.admin.AdminException;

import java.util.HashMap;
import java.util.StringTokenizer;
import java.util.*;

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
  private static final String method_getSchedItem_ID = "getSchedItem";
  private static final String method_getSchedItems_ID = "getSchedItems";

  public MCISmeTransport(ServiceInfo info, String host, int port) {
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

	private SchedItem decodeSchedItemString(String str) throws AdminException 
	{
		SchedItem item = new SchedItem();
		
		if(str.length() > 0)
		{
			int start = 0; int end = str.indexOf(",", start);
			item.schedTime = str.substring(start, end);
			start = end + 1; end = str.indexOf(",", start);
			item.address = str.substring(start, end);
			start = end + 1; end = str.indexOf(",", start);
			item.eventsCount = str.substring(start, end);
			start = end + 1; end = str.indexOf(";", start);
			item.lastError = str.substring(start, end);
		}
		return item;
	}
	private ArrayList decodeSchedItemsString(String str) throws AdminException 
	{
		ArrayList itemsList = new ArrayList();

		int pos = 0; int len = str.length();
		while(pos < len)
		{
			SchedItem item = new SchedItem();

			int start = pos; int end = str.indexOf(",", start);
			item.schedTime = str.substring(start, end);
			start = end + 1; end = str.indexOf(",", start);
			item.address = str.substring(start, end);
			start = end + 1; end = str.indexOf(",", start);
			item.eventsCount = str.substring(start, end);
			start = end + 1; end = str.indexOf(";", start);
			item.lastError = str.substring(start, end);
			itemsList.add(item);
			pos = end + 1;
		}		
		return itemsList;
	}
	
  public synchronized void flushStatistics() throws AdminException 
  {
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

	public synchronized SchedItem getSchedItem(String address) throws AdminException 
	{
		HashMap	arg = new HashMap();

		arg.put("Abonent", address);
		Object obj = call(SME_COMPONENT_ID, method_getSchedItem_ID, Type.Types[Type.StringType], arg);
		return (obj != null && obj instanceof String) ? decodeSchedItemString((String)obj):new SchedItem();
	}

	public synchronized ArrayList getSchedItems() throws AdminException 
	{
		Object obj = call(SME_COMPONENT_ID, method_getSchedItems_ID, Type.Types[Type.StringType], new HashMap());
		return (obj != null && obj instanceof String) ? decodeSchedItemsString((String)obj): new ArrayList();
	}

}
