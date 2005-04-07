package ru.sibinco.smppgw.beans.gw.status;

import ru.sibinco.lib.SibincoException;
import ru.sibinco.lib.backend.daemon.Daemon;
import ru.sibinco.lib.backend.service.ServiceInfo;
import ru.sibinco.lib.backend.protocol.Proxy;
import ru.sibinco.lib.backend.util.config.Config;
import ru.sibinco.lib.backend.service.ServiceInfo;
import ru.sibinco.smppgw.Constants;
import ru.sibinco.smppgw.backend.Gateway;
import ru.sibinco.smppgw.beans.SmppgwBean;
import ru.sibinco.smppgw.beans.SmppgwJspException;

import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;
import java.io.IOException;


/**
 * Created by IntelliJ IDEA. User: igork Date: 03.03.2004 Time: 18:39:37
 */
public class Index extends SmppgwBean
{
  private String mbApply;
  private String mbRestore;
  private String mbStart;
  private String mbStop;
  private String[] subj;
  private boolean gwRunning;
  private boolean gwStopped;

  public void process(final HttpServletRequest request, final HttpServletResponse response) throws SmppgwJspException
  {
    super.process(request, response);
    final Daemon gwDaemon = appContext.getGwDaemon();
    try {
      gwDaemon.refreshServices(appContext.getGwSmeManager());
    } catch (SibincoException e) {
      logger.error("Could not refresh services", e);
      throw new SmppgwJspException(Constants.errors.status.COULDNT_REFRESH_SERVICES);
    } catch (NullPointerException e) {
      logger.error("Could not get GW daemon");
      throw new SmppgwJspException(Constants.errors.status.COULDNT_GET_DAEMON);
    }
    final ServiceInfo info = gwDaemon.getServiceInfo(appContext.getGateway().getId());
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

  private void stop() throws SmppgwJspException
  {
    try {
      appContext.getGwDaemon().shutdownService(appContext.getGateway().getId());
    } catch (SibincoException e) {
      logger.error("Could not stop Gateway", e);
      throw new SmppgwJspException(Constants.errors.status.COULDNT_STOP_GATEWAY, e);
    }
  }

  private void start() throws SmppgwJspException
  {
    try {
      appContext.getGwDaemon().startService(appContext.getGateway().getId());
    } catch (SibincoException e) {
      logger.error("Could not start Gateway", e);
      throw new SmppgwJspException(Constants.errors.status.COULDNT_START_GATEWAY, e);
    }
  }

  private void restore()
  {
    if (null != subj && 0 < subj.length)
      for (int i = 0; i < subj.length; i++) {
        final String s = subj[i];
        if ("config".equals(s))
          restoreConfig();
      }
  }

  private void restoreConfig()
  {
  }

  private void apply() throws SmppgwJspException
  {
    if (null != subj && 0 < subj.length)
      for (int i = 0; i < subj.length; i++) {
        final String s = subj[i];
        if ("config".equals(s))
          applyConfig();
        if ("routes".equals(s))
          applyRoutes();
        if ("providers".equals(s))
          applyProviders();
        if ("smscs".equals(s))
          applySmscs();
        if ("users".equals(s))
          applyUsers();
        if ("billing".equals(s))
          applyBilling();
      }
  }

  private void applyBilling() throws SmppgwJspException
  {
    try {
      appContext.getBillingManager().save();
      appContext.getStatuses().setBillingChanged(false);
    } catch (Throwable e) {
      logger.debug("Couldn't apply Route billing rules", e);
      throw new SmppgwJspException(Constants.errors.status.COULDNT_APPLY_BILLING, e);
    }
  }

  private void applySmscs() throws SmppgwJspException
  {
    try {
      final Config gwConfig = appContext.getGwConfig();
      Gateway gateway=appContext.getGateway();
      appContext.getSmscsManager().store(gwConfig,gateway);
      appContext.getGwConfig().save();
      try {
        appContext.getGateway().apply("smscs");
      } catch (SibincoException e) {
        if (Proxy.StatusConnected == appContext.getGateway().getStatus()) {
          logger.debug("Couldn't apply Service centers", e);
          throw new SmppgwJspException(Constants.errors.status.COULDNT_APPLY_SMSCS, e);
        }
      }
      appContext.getStatuses().setSmscsChanged(false);
    } catch (Throwable e) {
      logger.debug("Couldn't apply Service centers", e);
      throw new SmppgwJspException(Constants.errors.status.COULDNT_APPLY_SMSCS, e);
    }
  }

  private void applyProviders() throws SmppgwJspException
  {
    try {
      final Config gwConfig = appContext.getGwConfig();
      appContext.getProviderManager().store(gwConfig);
      appContext.getGwConfig().save();
      try {
        appContext.getGateway().apply("providers");
      } catch (SibincoException e) {
        if (Proxy.StatusConnected == appContext.getGateway().getStatus()) {
          logger.debug("Couldn't apply providers", e);
          throw new SmppgwJspException(Constants.errors.status.COULDNT_APPLY_PROVIDERS, e);
        }
      }
      appContext.getStatuses().setProvidersChanged(false);
    } catch (Throwable e) {
      logger.debug("Couldn't apply providers", e);
      throw new SmppgwJspException(Constants.errors.status.COULDNT_APPLY_PROVIDERS, e);
    }
  }

  private void applyUsers() throws SmppgwJspException
  {
    try {
      appContext.getUserManager().apply();
      appContext.getStatuses().setUsersChanged(false);
    } catch (Throwable e) {
      logger.debug("Couldn't apply users", e);
      throw new SmppgwJspException(Constants.errors.status.COULDNT_APPLY_USERS, e);
    }
  }

  private void applyRoutes() throws SmppgwJspException
  {
    try {
      appContext.getGwRoutingManager().apply();
      appContext.getGwSmeManager().store();
      try {
        appContext.getGateway().apply("routes");
      } catch (SibincoException e) {
        if (Proxy.StatusConnected == appContext.getGateway().getStatus()) {
          logger.debug("Couldn't apply routes", e);
          throw new SmppgwJspException(Constants.errors.status.COULDNT_APPLY_ROUTES, e);
        }
      }
      appContext.getStatuses().setRoutesChanged(false);
    } catch (SibincoException e) {
      logger.debug("Couldn't apply routes", e);
      throw new SmppgwJspException(Constants.errors.status.COULDNT_APPLY_ROUTES, e);
    }
  }

  private void applyConfig() throws SmppgwJspException
  {
    try {
      appContext.getSmscsManager().store(appContext.getGwConfig());
      appContext.getProviderManager().store(appContext.getGwConfig());
      appContext.getGwSmeManager().store();
      appContext.getGwConfig().save();

      try {
        appContext.getGateway().apply("config");
      } catch (SibincoException e) {
        if (Proxy.StatusConnected == appContext.getGateway().getStatus()) {
          logger.debug("Couldn't apply config", e);
          throw new SmppgwJspException(Constants.errors.status.COULDNT_APPLY_CONFIG, e);
        }
      }
      appContext.getStatuses().setConfigChanged(false);
    } catch (SibincoException e) {
      logger.debug("Couldn't apply config", e);
      throw new SmppgwJspException(Constants.errors.status.COULDNT_APPLY_CONFIG, e);
    } catch (Config.WrongParamTypeException e) {
      logger.debug("Couldn't save config", e);
      throw new SmppgwJspException(Constants.errors.status.COULDNT_SAVE_CONFIG, e);
    } catch (IOException e) {
      logger.debug("Couldn't save config", e);
      throw new SmppgwJspException(Constants.errors.status.COULDNT_SAVE_CONFIG, e);
    }
  }

  public String getMbApply()
  {
    return mbApply;
  }

  public void setMbApply(final String mbApply)
  {
    this.mbApply = mbApply;
  }

  public String getMbRestore()
  {
    return mbRestore;
  }

  public void setMbRestore(final String mbRestore)
  {
    this.mbRestore = mbRestore;
  }

  public String[] getSubj()
  {
    return subj;
  }

  public void setSubj(final String[] subj)
  {
    this.subj = subj;
  }

  public boolean isConfigChanged()
  {
    return appContext.getStatuses().isConfigChanged();
  }

  public boolean isRoutesChanged()
  {
    return appContext.getStatuses().isRoutesChanged();
  }

  public boolean isBillingChanged()
  {
    return appContext.getStatuses().isBillingChanged();
  }

  public boolean isProvidersChanged()
  {
    return appContext.getStatuses().isProvidersChanged();
  }

  public boolean isSmscsChanged()
  {
    return appContext.getStatuses().isSmscsChanged();
  }

  public boolean isUsersChanged()
  {
    return appContext.getStatuses().isUsersChanged();
  }

  public boolean isGwRunning()
  {
    return gwRunning;
  }

  public boolean isGwStopped()
  {
    return gwStopped;
  }

  public String getMbStart()
  {
    return mbStart;
  }

  public void setMbStart(final String mbStart)
  {
    this.mbStart = mbStart;
  }

  public String getMbStop()
  {
    return mbStop;
  }

  public void setMbStop(final String mbStop)
  {
    this.mbStop = mbStop;
  }
}