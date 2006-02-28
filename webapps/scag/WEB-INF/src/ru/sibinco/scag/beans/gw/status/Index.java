package ru.sibinco.scag.beans.gw.status;

import ru.sibinco.lib.SibincoException;
import ru.sibinco.lib.backend.util.config.Config;
import ru.sibinco.scag.Constants;
import ru.sibinco.scag.backend.daemon.Daemon;
import ru.sibinco.scag.backend.daemon.Proxy;
import ru.sibinco.scag.backend.daemon.ServiceInfo;
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

    private String mbApply;
    private String mbRestore;
    private String mbStart;
    private String mbStop;
    private String[] subj;
    private boolean gwRunning;
    private boolean gwStopped;

    protected Collection getDataSource() {
        return appContext.getStatusManager().getStatMessages().values();
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
        if (null != mbRestore)
            restore();
        else if (null != mbApply)
            apply();
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
    }

    private void start() throws SCAGJspException {
        try {
            appContext.getScagDaemon().startService(appContext.getScag().getId());
        } catch (SibincoException e) {
            logger.error("Could not start Scag", e);
            throw new SCAGJspException(Constants.errors.status.COULDNT_START_GATEWAY, e);
        }
    }

    private void restore() {
        if (null != subj && 0 < subj.length)
            for (int i = 0; i < subj.length; i++) {
                final String s = subj[i];
                if ("config".equals(s))
                    restoreConfig();
            }
    }

    private void restoreConfig() {
    }

    private void apply() throws SCAGJspException {
        if (null != subj && 0 < subj.length)
            for (int i = 0; i < subj.length; i++) {
                final String s = subj[i];
                if ("config".equals(s))
                    applyConfig();
                if ("routes".equals(s))
                    applyRoutes();
                if ("users".equals(s))
                    applyUsers();
                if ("billing".equals(s))
                    applyBilling();
            }
    }

    private void applyBilling() throws SCAGJspException {
        try {
            appContext.getBillingManager().save();
            appContext.getStatuses().setBillingChanged(false);
        } catch (Throwable e) {
            logger.debug("Couldn't apply Route billing rules", e);
            throw new SCAGJspException(Constants.errors.status.COULDNT_APPLY_BILLING, e);
        }
    }

    private void applyUsers() throws SCAGJspException {
        try {
            appContext.getUserManager().apply();
            appContext.getStatuses().setUsersChanged(false);
        } catch (Throwable e) {
            logger.debug("Couldn't apply users", e);
            throw new SCAGJspException(Constants.errors.status.COULDNT_APPLY_USERS, e);
        }
    }

    private void applyRoutes() throws SCAGJspException {
        try {
            appContext.getScagRoutingManager().apply();
            try {
                appContext.getScag().apply("routes");
            } catch (SibincoException e) {
                if (Proxy.STATUS_CONNECTED == appContext.getScag().getStatus()) {
                    logger.debug("Couldn't apply routes", e);
                    throw new SCAGJspException(Constants.errors.status.COULDNT_APPLY_ROUTES, e);
                }
            }
            appContext.getStatuses().setRoutesChanged(false);
        } catch (SibincoException e) {
            logger.debug("Couldn't apply routes", e);
            throw new SCAGJspException(Constants.errors.status.COULDNT_APPLY_ROUTES, e);
        }
    }

    private void applyConfig() throws SCAGJspException {
        try {
            appContext.getGwConfig().save();

            try {
                appContext.getScag().apply("config");
            } catch (SibincoException e) {
                if (Proxy.STATUS_CONNECTED == appContext.getScag().getStatus()) {
                    logger.debug("Couldn't apply config", e);
                    throw new SCAGJspException(Constants.errors.status.COULDNT_APPLY_CONFIG, e);
                }
            }
            appContext.getStatuses().setConfigChanged(false);
        } catch (SibincoException e) {
            logger.debug("Couldn't apply config", e);
            throw new SCAGJspException(Constants.errors.status.COULDNT_APPLY_CONFIG, e);
        } catch (Config.WrongParamTypeException e) {
            logger.debug("Couldn't save config", e);
            throw new SCAGJspException(Constants.errors.status.COULDNT_SAVE_CONFIG, e);
        } catch (IOException e) {
            logger.debug("Couldn't save config", e);
            throw new SCAGJspException(Constants.errors.status.COULDNT_SAVE_CONFIG, e);
        }
    }

    public String getMbApply() {
        return mbApply;
    }

    public void setMbApply(final String mbApply) {
        this.mbApply = mbApply;
    }

    public String getMbRestore() {
        return mbRestore;
    }

    public void setMbRestore(final String mbRestore) {
        this.mbRestore = mbRestore;
    }

    public String[] getSubj() {
        return subj;
    }

    public void setSubj(final String[] subj) {
        this.subj = subj;
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
}