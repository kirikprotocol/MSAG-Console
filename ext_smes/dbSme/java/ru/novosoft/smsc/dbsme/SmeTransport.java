package ru.novosoft.smsc.dbsme;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.Constants;
import ru.novosoft.smsc.admin.service.*;
import ru.novosoft.smsc.jsp.SMSCAppContext;

import java.util.*;

/**
 * Created by IntelliJ IDEA.
 * User: igork
 * Date: 13.11.2003
 * Time: 16:13:03
 */
public class SmeTransport extends Service
{
  private Component smeComponent = null;
  private Method method_restart = null;
  private Method method_addJob = null;
  private Method method_removeJob = null;
  private Method method_changeJob = null;
  private Method method_setProviderEnabled = null;

  public SmeTransport(ServiceInfo info)
  {
    super(info);
  }

  protected void checkComponents()
  {
    super.checkComponents();
    smeComponent = (Component) getInfo().getComponents().get("DBSme");
    method_restart = (Method) smeComponent.getMethods().get("restart");
    method_addJob = (Method) smeComponent.getMethods().get("addJob");
    method_removeJob = (Method) smeComponent.getMethods().get("removeJob");
    method_changeJob = (Method) smeComponent.getMethods().get("changeJob");
    method_setProviderEnabled = (Method) smeComponent.getMethods().get("setProviderEnabled");
  }

  public synchronized void updateInfo(SMSCAppContext appContext) throws AdminException
  {
    setInfo(appContext.getHostsManager().getServiceInfo(Constants.DBSME_SME_ID));
  }

  public synchronized void restart() throws AdminException
  {
    refreshComponents();
    call(smeComponent, method_restart, Type.Types[Type.StringType], new HashMap());
  }

  public synchronized void addJob(String jobId, String providerId) throws AdminException
  {
    refreshComponents();
    Map params = new HashMap();
    params.put("jobId", jobId);
    params.put("providerId", providerId);
    call(smeComponent, method_addJob, Type.Types[Type.StringType], params);
  }

  public synchronized void removeJob(String jobId, String providerId) throws AdminException
  {
    refreshComponents();
    Map params = new HashMap();
    params.put("jobId", jobId);
    params.put("providerId", providerId);
    call(smeComponent, method_removeJob, Type.Types[Type.StringType], params);
  }

  public synchronized void changeJob(String jobId, String providerId) throws AdminException
  {
    refreshComponents();
    Map params = new HashMap();
    params.put("jobId", jobId);
    params.put("providerId", providerId);
    call(smeComponent, method_changeJob, Type.Types[Type.StringType], params);
  }

  public synchronized void setProviderEnabled(String providerId, boolean enabled) throws AdminException
  {
    refreshComponents();
    Map params = new HashMap();
    params.put("providerId", providerId);
    params.put("enabled", new Boolean(enabled));
    call(smeComponent, method_setProviderEnabled, Type.Types[Type.StringType], params);
  }
}
