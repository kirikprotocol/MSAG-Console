package ru.sibinco.scag.beans.gw.status;

import ru.sibinco.lib.SibincoException;
import ru.sibinco.lib.backend.util.config.Config;
import ru.sibinco.scag.Constants;
import ru.sibinco.scag.backend.daemon.Daemon;
import ru.sibinco.scag.backend.daemon.Proxy;
import ru.sibinco.scag.backend.daemon.ServiceInfo;
import ru.sibinco.scag.backend.status.StatMessage;
import ru.sibinco.scag.backend.status.StatusManager;
import ru.sibinco.scag.beans.SCAGJspException;
import ru.sibinco.scag.beans.TabledBeanImpl;

import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;
import java.io.IOException;
import java.util.Collection;


/**
 * Created by IntelliJ IDEA. User: igork Date: 03.03.2004 Time: 18:39:37
 */
public class Index extends TabledBeanImpl {

    private String mbStart;
    private String mbStop;
    private boolean gwRunning;
    private boolean gwStopped;

    protected Collection getDataSource() {
        return StatusManager.getInstance().getStatMessages();
    }

    public void process(final HttpServletRequest request, final HttpServletResponse response) throws SCAGJspException {

        super.process(request, response);
        final Daemon scagDaemon = appContext.getScagDaemon();
        try {
            scagDaemon.refreshServices(appContext.getSmppManager());
        } catch (SibincoException e) {
            logger.error("Could not refresh services", e);
            throw new SCAGJspException(Constants.errors.status.COULDNT_REFRESH_SERVICES);
        } catch (NullPointerException e) {
            logger.error("Could not get GW daemon");
            throw new SCAGJspException(Constants.errors.status.COULDNT_GET_DAEMON);
        }

        final ServiceInfo info = scagDaemon.getServiceInfo(appContext.getScag().getId());

        if (null != info) {
            final byte gwStatus = info.getStatus();
            gwRunning = ServiceInfo.STATUS_RUNNING == gwStatus;
            gwStopped = ServiceInfo.STATUS_STOPPED == gwStatus;
        }

        if (null != mbStart)
            start();
        if (null != mbStop)
            stop();
    }

    protected void delete() throws SCAGJspException {
        //To change body of implemented methods use File | Settings | File Templates.
    }

    private void stop() throws SCAGJspException {
        try {
            appContext.getScagDaemon().shutdownService(appContext.getScag().getId());
        } catch (SibincoException e) {
            logger.error("Could not stop Scag", e);
            throw new SCAGJspException(Constants.errors.status.COULDNT_STOP_GATEWAY, e);
        }
        StatusManager.getInstance().addStatMessages(new StatMessage(getLoginedPrincipal().getName(),
                "Status MSAG", "Service MSAG stoped"));
    }

    private void start() throws SCAGJspException {
        try {
            appContext.getScagDaemon().startService(appContext.getScag().getId());
        } catch (SibincoException e) {
            logger.error("Could not start Scag", e);
            throw new SCAGJspException(Constants.errors.status.COULDNT_START_GATEWAY, e);
        }
        StatusManager.getInstance().addStatMessages(new StatMessage(getLoginedPrincipal().getName(),
                "Status MSAG", "Service MSAG started"));
    }

    public boolean isConfigChanged() {
        return appContext.getStatuses().isConfigChanged();
    }

    public boolean isRoutesChanged() {
        return appContext.getStatuses().isRoutesChanged();
    }

    public boolean isBillingChanged() {
        return appContext.getStatuses().isBillingChanged();
    }

    public boolean isSmscsChanged() {
        return appContext.getStatuses().isSmscsChanged();
    }

    public boolean isUsersChanged() {
        return appContext.getStatuses().isUsersChanged();
    }

    public boolean isGwRunning() {
        return gwRunning;
    }

    public boolean isGwStopped() {
        return gwStopped;
    }

    public String getMbStart() {
        return mbStart;
    }

    public void setMbStart(final String mbStart) {
        this.mbStart = mbStart;
    }

    public String getMbStop() {
        return mbStop;
    }

    public void setMbStop(final String mbStop) {
        this.mbStop = mbStop;
    }

    protected String getDefaultSort() {
        return "-time";
    }
}