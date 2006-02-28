package ru.sibinco.scag.beans.gw.status;

import ru.sibinco.lib.SibincoException;
import ru.sibinco.scag.beans.SCAGBean;
import ru.sibinco.scag.beans.SCAGJspException;
import ru.sibinco.scag.backend.daemon.ServiceInfo;
import ru.sibinco.scag.backend.daemon.Daemon;

import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;


/**
 * Created by igork Date: 27.07.2004 Time: 17:17:59
 */
public class Status extends SCAGBean {

    private byte scagStatus = ServiceInfo.STATUS_UNKNOWN;
    public void process(final HttpServletRequest request, final HttpServletResponse response) throws SCAGJspException {
        super.process(request, response);
        final Daemon scagDaemon = appContext.getScagDaemon();
        try {
            scagDaemon.refreshServices(appContext.getSmppManager());
            final Object gwService = scagDaemon.getServiceInfo(appContext.getScag().getId());
            if (gwService instanceof ServiceInfo) {
                final ServiceInfo info = (ServiceInfo) gwService;
                scagStatus = info.getStatus();
            }
        } catch (SibincoException e) {
            logger.error("Could not refresh services", e);
        } catch (NullPointerException e) {
            logger.error("Could not get SCAG daemon");
        }
    }

    public byte getScagStatus() {
        return scagStatus;
    }
}
