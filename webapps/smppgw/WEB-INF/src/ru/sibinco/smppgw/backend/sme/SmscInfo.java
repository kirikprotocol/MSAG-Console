package ru.sibinco.smppgw.backend.sme;

import ru.sibinco.lib.backend.util.config.Config;


/**
 * Created by igork Date: 24.03.2004 Time: 17:58:57
 */
public class SmscInfo
{
  private String id;
  private String host;
  private int port;
  private String systemId;
  private String password;
  private int responseTimeout;
  private int uniqueMsgIdPrefix;

  public SmscInfo(String id, String host, int port, String systemId, String password, int responseTimeout, int uniqueMsgIdPrefix)
  {
    this.id = id;
    this.host = host;
    this.port = port;
    this.systemId = systemId;
    this.password = password;
    this.responseTimeout = responseTimeout;
    this.uniqueMsgIdPrefix = uniqueMsgIdPrefix;
  }

  public SmscInfo(Config gwConfig, String smscSection) throws Config.WrongParamTypeException, Config.ParamNotFoundException
  {
    this.id = smscSection.substring(smscSection.lastIndexOf('.') + 1);
    this.host = gwConfig.getString(smscSection + ".host");
    this.port = (int) gwConfig.getInt(smscSection + ".port");
    this.systemId = gwConfig.getString(smscSection + ".systemId");
    this.password = gwConfig.getString(smscSection + ".password");
    this.responseTimeout = (int) gwConfig.getInt(smscSection + ".responseTimeout");
    this.uniqueMsgIdPrefix = (int) gwConfig.getInt(smscSection + ".uniqueMsgIdPrefix");
  }

  public String getId()
  {
    return id;
  }

  public void setId(String id)
  {
    this.id = id;
  }

  public String getHost()
  {
    return host;
  }

  public void setHost(String host)
  {
    this.host = host;
  }

  public int getPort()
  {
    return port;
  }

  public void setPort(int port)
  {
    this.port = port;
  }

  public String getSystemId()
  {
    return systemId;
  }

  public void setSystemId(String systemId)
  {
    this.systemId = systemId;
  }

  public String getPassword()
  {
    return password;
  }

  public void setPassword(String password)
  {
    this.password = password;
  }

  public int getResponseTimeout()
  {
    return responseTimeout;
  }

  public void setResponseTimeout(int responseTimeout)
  {
    this.responseTimeout = responseTimeout;
  }

  public int getUniqueMsgIdPrefix()
  {
    return uniqueMsgIdPrefix;
  }

  public void setUniqueMsgIdPrefix(int uniqueMsgIdPrefix)
  {
    this.uniqueMsgIdPrefix = uniqueMsgIdPrefix;
  }
}
