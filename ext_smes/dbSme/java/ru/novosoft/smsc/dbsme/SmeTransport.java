package ru.novosoft.smsc.dbsme;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.service.*;
import ru.novosoft.smsc.jsp.SMSCAppContext;

import java.util.HashMap;
import java.util.Map;

/**
 * Created by IntelliJ IDEA.
 * User: igork
 * Date: 13.11.2003
 * Time: 16:13:03
 */
public class SmeTransport extends Service
{
  private static final String SME_COMPONENT_ID = "DBSme";
  private static final String method_restart_ID = "restart";
  private static final String method_addJob_ID = "addJob";
  private static final String method_removeJob_ID = "removeJob";
  private static final String method_changeJob_ID = "changeJob";
  private static final String method_setProviderEnabled_ID = "setProviderEnabled";

  public SmeTransport(ServiceInfo info, int port)
  {
    super(info, port);
  }

  public synchronized void updateInfo(SMSCAppContext appContext) throws AdminException
  {
    setInfo(appContext.getHostsManager().getServiceInfo(getInfo().getId()));
  }

  public synchronized void restart() throws AdminException
  {
    call(SME_COMPONENT_ID, method_restart_ID, Type.Types[Type.StringType], new HashMap());
  }

  public synchronized void addJob(String jobId, String providerId) throws AdminException
  {
    Map params = new HashMap();
    params.put("jobId", jobId);
    params.put("providerId", providerId);
    call(SME_COMPONENT_ID, method_addJob_ID, Type.Types[Type.StringType], params);
  }

  public synchronized void removeJob(String jobId, String providerId) throws AdminException
  {
    Map params = new HashMap();
    params.put("jobId", jobId);
    params.put("providerId", providerId);
    call(SME_COMPONENT_ID, method_removeJob_ID, Type.Types[Type.StringType], params);
  }

  public synchronized void changeJob(String jobId, String providerId) throws AdminException
  {
    Map params = new HashMap();
    params.put("jobId", jobId);
    params.put("providerId", providerId);
    call(SME_COMPONENT_ID, method_changeJob_ID, Type.Types[Type.StringType], params);
  }

  public synchronized void setProviderEnabled(String providerId, boolean enabled) throws AdminException
  {
    Map params = new HashMap();
    params.put("providerId", providerId);
    params.put("enabled", new Boolean(enabled));
    call(SME_COMPONENT_ID, method_setProviderEnabled_ID, Type.Types[Type.StringType], params);
  }
}
