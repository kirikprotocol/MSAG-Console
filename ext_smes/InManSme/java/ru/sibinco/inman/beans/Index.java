package ru.sibinco.inman.beans;

import ru.novosoft.smsc.jsp.PageBean;
import ru.novosoft.smsc.util.config.Config;
import ru.novosoft.smsc.util.Functions;
import ru.novosoft.smsc.admin.service.ServiceInfo;
import ru.novosoft.smsc.admin.AdminException;
import ru.sibinco.inman.backend.InManSmeContext;
import ru.sibinco.inman.backend.InManSme;

import javax.servlet.http.HttpServletRequest;
import java.util.List;

/**
 * Created by IntelliJ IDEA.
 * User: makar
 * Date: 29.11.2005
 * Time: 11:53:08
 * To change this template use File | Settings | File Templates.
 */
public class Index extends PageBean
{
    private Config config = null;
    private boolean initialized = false;

    private InManSmeContext inManSmeContext = null;
    private InManSme inManSme = null;

    private String smeId   = "INMan";

    private String mbApply = null;
    private String mbReset = null;
    private String mbStart = null;
    private String mbStop  = null;

    private String ssfAddress       = "";
    private String ssn              = "";
    private String scfAddress       = "";
    private String host             = "";
    private String port             = "";
    private String billingDir       = "";
    private String billingInterval  = "";
    private String cdrMode          = "all";
    private String billMode         = "all";

    protected int init(List errors)
    {
      int result = super.init(errors);
      if (result != RESULT_OK)  return result;

      try
      {
        inManSmeContext = InManSmeContext.getInstance(appContext, smeId);
        inManSme = inManSmeContext.getInManSme();
        config = inManSmeContext.getConfig();

      }
      catch (Throwable e) {
        logger.error("Failed to init InMan", e);
        return error("inman.error.init", e);
      }

      return (!initialized) ? loadConfig():result;
    }

    public int process(HttpServletRequest request)
    {
        int result = super.process(request);
        if (result != RESULT_OK)  return result;

        try {
          smeId = Functions.getServiceId(request.getServletPath());
        } catch (AdminException e) {
          logger.error("Could not discover sme id", e);
          error("inman.error.sme_id", smeId, e);
        }

        try {
          getAppContext().getHostsManager().refreshServices();
          inManSme.updateInfo(appContext);
        } catch (AdminException e) {
          logger.error("Could not refresh InMan status", e);
          result = error("inman.error.status", e);
        }

        if (mbApply != null)  return apply();
        if (mbReset != null)  return reset();
        if (mbStart != null)  return start();
        if (mbStop != null)   return stop();

        return result;
    }

    private int loadConfig()
    {
        try
        {
            ssfAddress = config.getString("ssfAddress");
            ssn = Integer.toString(config.getInt("ssn"));
            scfAddress = config.getString("scfAddress");
            host = config.getString("host");
            port = Integer.toString(config.getInt("port"));
            billingDir = config.getString("billingDir");
            billingInterval = Integer.toString(config.getInt("billingInterval"));
            cdrMode = config.getString("cdrMode");
            billMode = config.getString("billMode");
        }
        catch (Exception ex) {
            logger.error("Failed to load config", ex);
            return error("inman.error.config_load", ex);
        }
        return RESULT_OK;
    }
    private int saveConfig()
    {
        try
        {
            config.setString("ssfAddress", ssfAddress);
            config.setInt("ssn", Integer.parseInt(ssn));
            config.setString("scfAddress", scfAddress);
            config.setString("host", host);
            config.setInt("port", Integer.parseInt(port));
            config.setString("billingDir", billingDir);
            config.setInt("billingInterval", Integer.parseInt(billingInterval));
            config.setString("cdrMode", cdrMode);
            config.setString("billMode", billMode);
            config.save();
        }
        catch (Exception ex) {
            logger.error("Failed to save config", ex);
            return error("inman.error.config_save", ex);
        }
        return RESULT_OK;
    }

    protected int apply()
    {
        int result = RESULT_OK;
        try {
          logger.debug("Apply ...");
          result = saveConfig();
        } catch (Throwable e) {
          logger.error("Failed to save InMan config", e);
          return error("inman.error.config_save", e);
        }
        return (result != RESULT_OK) ? result:
                message("inman.message.need_restart");
    }

    protected int reset()
    {
        int result = RESULT_OK;
        try {
          logger.debug("Reset ...");
          inManSmeContext.resetConfig();
          config = inManSmeContext.getConfig();
          result = loadConfig();
        } catch (Throwable e) {
          logger.debug("Couldn't reload InMan config", e);
          return error("inman.error.config_reload", e);
        }
        return result;
    }

    protected int start()
    {
        int result = RESULT_OK;
        try {
          getAppContext().getHostsManager().startService(getSmeId());
          try { Thread.sleep(5000); } catch (InterruptedException e) {}
        } catch (AdminException e) {
          logger.error("Could not start InMan", e);
          result = error("inman.error.start", e);
        }
        return result;
    }

    protected int stop()
    {
        int result = RESULT_OK;
        try {
          getAppContext().getHostsManager().shutdownService(getSmeId());
        } catch (AdminException e) {
          logger.error("Could not stop InMan", e);
          result = error("inman.error.stop", e);
        }
        return result;
    }


    public String getSmeId() {
      return smeId;
    }

    public boolean isInitialized() {
        return initialized;
    }
    public void setInitialized(boolean initialized) {
        this.initialized = initialized;
    }

    public byte getStatus()
    {
      try {
        return hostsManager.getServiceInfo(getSmeId()).getStatus();
      } catch (AdminException e) {
        return ServiceInfo.STATUS_UNKNOWN;
      }
    }

    public String getMbApply() {
        return mbApply;
    }
    public void setMbApply(String mbApply) {
        this.mbApply = mbApply;
    }
    public String getMbReset() {
        return mbReset;
    }
    public void setMbReset(String mbReset) {
        this.mbReset = mbReset;
    }
    public String getMbStart() {
        return mbStart;
    }
    public void setMbStart(String mbStart) {
        this.mbStart = mbStart;
    }
    public String getMbStop() {
        return mbStop;
    }
    public void setMbStop(String mbStop) {
        this.mbStop = mbStop;
    }

    public String getSsfAddress() {
        return ssfAddress;
    }
    public void setSsfAddress(String ssfAddress) {
        this.ssfAddress = ssfAddress;
    }

    public String getSsn() {
        return ssn;
    }
    public void setSsn(String ssn) {
        this.ssn = ssn;
    }

    public String getScfAddress() {
        return scfAddress;
    }
    public void setScfAddress(String scfAddress) {
        this.scfAddress = scfAddress;
    }

    public String getHost() {
        return host;
    }
    public void setHost(String host) {
        this.host = host;
    }

    public String getPort() {
        return port;
    }
    public void setPort(String port) {
        this.port = port;
    }

    public String getBillingDir() {
        return billingDir;
    }
    public void setBillingDir(String billingDir) {
        this.billingDir = billingDir;
    }

    public String getBillingInterval() {
        return billingInterval;
    }
    public void setBillingInterval(String billingInterval) {
        this.billingInterval = billingInterval;
    }

    public String getCdrMode() {
        return cdrMode;
    }
    public void setCdrMode(String cdrMode) {
        this.cdrMode = cdrMode;
    }

    public String getBillMode() {
        return billMode;
    }
    public void setBillMode(String billMode) {
        this.billMode = billMode;
    }
}
