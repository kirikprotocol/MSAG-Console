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

  public SmscInfo(final String id, final String host, final int port, final String systemId, final String password, final int responseTimeout,
                  final int uniqueMsgIdPrefix)
  {
    this.id = id;
    this.host = host;
    this.port = port;
    this.systemId = systemId;
    this.password = password;
    this.responseTimeout = responseTimeout;
    this.uniqueMsgIdPrefix = uniqueMsgIdPrefix;
  }

  public SmscInfo(final Config gwConfig, final String smscSection) throws Config.WrongParamTypeException, Config.ParamNotFoundException
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

  public void setId(final String id)
  {
    this.id = id;
  }

  public String getHost()
  {
    return host;
  }

  public void setHost(final String host)
  {
    this.host = host;
  }

  public int getPort()
  {
    return port;
  }

  public void setPort(final int port)
  {
    this.port = port;
  }

  public String getSystemId()
  {
    return systemId;
  }

  public void setSystemId(final String systemId)
  {
    this.systemId = systemId;
  }

  public String getPassword()
  {
    return password;
  }

  public void setPassword(final String password)
  {
    this.password = password;
  }

  public int getResponseTimeout()
  {
    return responseTimeout;
  }

  public void setResponseTimeout(final int responseTimeout)
  {
    this.responseTimeout = responseTimeout;
  }

  public int getUniqueMsgIdPrefix()
  {
    return uniqueMsgIdPrefix;
  }

  public void setUniqueMsgIdPrefix(final int uniqueMsgIdPrefix)
  {
    this.uniqueMsgIdPrefix = uniqueMsgIdPrefix;
  }

  public void store(final Config gwConfig, final String sectionName)
  {
    final String fullName = sectionName + "." + id;
    gwConfig.setString(fullName + ".host", host);
    gwConfig.setInt(fullName + ".port", port);
    gwConfig.setString(fullName + ".systemId", systemId);
    gwConfig.setString(fullName + ".password", password);
    gwConfig.setInt(fullName + ".responseTimeout", responseTimeout);
    gwConfig.setInt(fullName + ".uniqueMsgIdPrefix", uniqueMsgIdPrefix);
  }
}
