package ru.novosoft.smsc.infosme.beans;

import ru.novosoft.smsc.jsp.SMSCAppContext;

import java.util.List;
import java.security.Principal;

/**
 * Created by igork
 * Date: Jul 31, 2003
 * Time: 3:43:24 PM
 */
public class Options extends InfoSmeBean {
  private String svcType = null;
  private int protocolId = 0;
  private int threadPoolMax = 0;
  private int threadPoolInit = 0;
  private String smscHost = null;
  private int smscPort = 0;
  private String smscSid = null;
  private int smscTimeout = 0;
  private String smscPassword = null;
  private boolean initialized = false;

  private String mbDone = null;
  private String mbCancel = null;

  protected int init(List errors)
  {
    int result = super.init(errors);
    if (result != RESULT_OK)
      return result;

    if (!initialized) {
      try {
        svcType = getConfig().getString("InfoSme.SvcType");
        protocolId = getConfig().getInt("InfoSme.ProtocolId");
        threadPoolMax = getConfig().getInt("InfoSme.ThreadPool.max");
        threadPoolInit = getConfig().getInt("InfoSme.ThreadPool.init");
        smscHost = getConfig().getString("InfoSme.SMSC.host");
        smscPort = getConfig().getInt("InfoSme.SMSC.port");
        smscSid = getConfig().getString("InfoSme.SMSC.sid");
        smscTimeout = getConfig().getInt("InfoSme.SMSC.timeout");
        smscPassword = getConfig().getString("InfoSme.SMSC.password");
      } catch (Exception e) {
        logger.error(e);
        return error(e.getMessage());
      }
    }
    return result;
  }

  public int process(SMSCAppContext appContext, List errors, Principal loginedPrincipal)
  {
    int result = super.process(appContext, errors, loginedPrincipal);
    if (result != RESULT_OK)
      return result;

    if (mbDone != null)
      return save();
    if (mbCancel != null)
      return RESULT_DONE;

    return result;
  }

  private int save()
  {
    getConfig().setString("InfoSme.SvcType", svcType);
    getConfig().setInt("InfoSme.ProtocolId", protocolId);
    getConfig().setInt("InfoSme.ThreadPool.max", threadPoolMax);
    getConfig().setInt("InfoSme.ThreadPool.init", threadPoolInit);
    getConfig().setString("InfoSme.SMSC.host", smscHost);
    getConfig().setInt("InfoSme.SMSC.port", smscPort);
    getConfig().setString("InfoSme.SMSC.sid", smscSid);
    getConfig().setInt("InfoSme.SMSC.timeout", smscTimeout);
    getConfig().setString("InfoSme.SMSC.password", smscPassword);
    return RESULT_DONE;
  }

  public String getSvcType()
  {
    return svcType;
  }

  public void setSvcType(String svcType)
  {
    this.svcType = svcType;
  }

  public int getProtocolIdInt()
  {
    return protocolId;
  }

  public void setProtocolIdInt(int protocolId)
  {
    this.protocolId = protocolId;
  }

  public String getProtocolId()
  {
    return String.valueOf(protocolId);
  }

  public void setProtocolId(String protocolId)
  {
    try {
      this.protocolId = Integer.decode(protocolId).intValue();
    } catch (NumberFormatException e) {
      logger.error("Invalid InfoSme.ProtocolId parameter value: \"" + protocolId + '"', e);
      this.protocolId = 0;
    }
  }

  public int getThreadPoolMaxInt()
  {
    return threadPoolMax;
  }

  public void setThreadPoolMaxInt(int threadPoolMax)
  {
    this.threadPoolMax = threadPoolMax;
  }

  public String getThreadPoolMax()
  {
    return String.valueOf(threadPoolMax);
  }

  public void setThreadPoolMax(String threadPoolMax)
  {
    try {
      this.threadPoolMax = Integer.decode(threadPoolMax).intValue();
    } catch (NumberFormatException e) {
      logger.error("Invalid InfoSme.ThreadPool.max parameter value: \"" + threadPoolMax + '"', e);
      this.threadPoolMax = 0;
    }
  }

  public int getThreadPoolInitInt()
  {
    return threadPoolInit;
  }

  public void setThreadPoolInitInt(int threadPoolInit)
  {
    this.threadPoolInit = threadPoolInit;
  }

  public String getThreadPoolInit()
  {
    return String.valueOf(threadPoolInit);
  }

  public void setThreadPoolInit(String threadPoolInit)
  {
    try {
      this.threadPoolInit = Integer.decode(threadPoolInit).intValue();
    } catch (NumberFormatException e) {
      logger.error("Invalid InfoSme.ThreadPool.init parameter value: \"" + threadPoolInit + '"', e);
      this.threadPoolInit = 0;
    }
  }

  public String getSmscHost()
  {
    return smscHost;
  }

  public void setSmscHost(String smscHost)
  {
    this.smscHost = smscHost;
  }

  public int getSmscPortInt()
  {
    return smscPort;
  }

  public void setSmscPortInt(int smscPort)
  {
    this.smscPort = smscPort;
  }

  public String getSmscPort()
  {
    return String.valueOf(smscPort);
  }

  public void setSmscPort(String smscPort)
  {
    try {
      this.smscPort = Integer.decode(smscPort).intValue();
    } catch (NumberFormatException e) {
      logger.error("Invalid InfoSme.SMSC.port parameter value: \"" + smscPort + '"', e);
      this.smscPort = 0;
    }
  }

  public String getSmscSid()
  {
    return smscSid;
  }

  public void setSmscSid(String smscSid)
  {
    this.smscSid = smscSid;
  }

  public int getSmscTimeoutInt()
  {
    return smscTimeout;
  }

  public void setSmscTimeoutInt(int smscTimeout)
  {
    this.smscTimeout = smscTimeout;
  }

  public String getSmscTimeout()
  {
    return String.valueOf(smscTimeout);
  }

  public void setSmscTimeout(String smscTimeout)
  {
    try {
      this.smscTimeout = Integer.valueOf(smscTimeout).intValue();
    } catch (NumberFormatException e) {
      logger.error("Invalid InfoSme.SMSC.timeout parameter value: \"" + smscTimeout + '"', e);
      this.smscTimeout = 0;
    }
  }

  public String getSmscPassword()
  {
    return smscPassword;
  }

  public void setSmscPassword(String smscPassword)
  {
    this.smscPassword = smscPassword;
  }

  public boolean isInitialized()
  {
    return initialized;
  }

  public void setInitialized(boolean initialized)
  {
    this.initialized = initialized;
  }

  public String getMbDone()
  {
    return mbDone;
  }

  public void setMbDone(String mbDone)
  {
    this.mbDone = mbDone;
  }

  public String getMbCancel()
  {
    return mbCancel;
  }

  public void setMbCancel(String mbCancel)
  {
    this.mbCancel = mbCancel;
  }
}
