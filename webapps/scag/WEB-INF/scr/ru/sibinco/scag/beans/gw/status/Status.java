package ru.sibinco.scag.beans.gw.status;

import ru.sibinco.lib.SibincoException;
import ru.sibinco.lib.backend.daemon.Daemon;
import ru.sibinco.lib.backend.service.ServiceInfo;
import ru.sibinco.scag.beans.SCAGBean;
import ru.sibinco.scag.beans.SCAGJspException;

import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;


/**
 * Created by igork Date: 27.07.2004 Time: 17:17:59
 */
public class Status extends SCAGBean
{
  private byte gwStatus = ServiceInfo.STATUS_UNKNOWN;

  public void process(final HttpServletRequest request, final HttpServletResponse response) throws SCAGJspException
  {
    super.process(request, response);
    final Daemon gwDaemon = appContext.getGwDaemon();
    try {
      gwDaemon.refreshServices(appContext.getGwSmeManager());
      final Object gwService = gwDaemon.getServiceInfo(appContext.getGateway().getId());
      if (gwService instanceof ServiceInfo) {
        final ServiceInfo info = (ServiceInfo) gwService;
        gwStatus = info.getStatus();
      }
    } catch (SibincoException e) {
      logger.error("Could not refresh services", e);
    } catch (NullPointerException e) {
      logger.error("Could not get GW daemon");
    }
  }

  public byte getGwStatus()
  {
    return gwStatus;
  }
}
