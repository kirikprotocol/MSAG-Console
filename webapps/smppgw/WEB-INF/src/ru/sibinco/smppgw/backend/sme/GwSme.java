package ru.sibinco.smppgw.backend.sme;

import ru.sibinco.lib.backend.sme.Sme;


/**
 * Created by igork
 * Date: 22.03.2004
 * Time: 19:00:30
 */
public class GwSme extends Sme
{
  private Provider provider;
  private SmscInfo smscInfo;

  public GwSme(String id, int priority, byte type, int typeOfNumber, int numberingPlan, int interfaceVersion, String systemType, String password, String addrRange, int smeN, boolean wantAlias, boolean forceDC, int timeout, String receiptSchemeName, boolean disabled, byte mode, int proclimit, int schedlimit, Provider provider)
      throws NullPointerException
  {
    super(id, priority, type, typeOfNumber, numberingPlan, interfaceVersion, systemType, password, addrRange, smeN, wantAlias, forceDC, timeout, receiptSchemeName, disabled, mode, proclimit, schedlimit);
    this.provider = provider;
    this.smscInfo = null;
  }

  public GwSme(String id, int priority, byte type, int typeOfNumber, int numberingPlan, int interfaceVersion, String systemType, String password, String addrRange, int smeN, boolean wantAlias, boolean forceDC, int timeout, String receiptSchemeName, boolean disabled, byte mode, int proclimit, int schedlimit, SmscInfo smscInfo)
      throws NullPointerException
  {
    super(id, priority, type, typeOfNumber, numberingPlan, interfaceVersion, systemType, password, addrRange, smeN, wantAlias, forceDC, timeout, receiptSchemeName, disabled, mode, proclimit, schedlimit);
    this.provider = null;
    this.smscInfo = smscInfo;
  }

  public GwSme(Sme sme, SmscInfo smscInfo)
      throws NullPointerException
  {
    super(sme);
    this.provider = null;
    this.smscInfo = smscInfo;
  }

  public GwSme(Sme sme, Provider provider)
      throws NullPointerException
  {
    super(sme);
    this.provider = provider;
    this.smscInfo = null;
  }

  public boolean isSmsc()
  {
    return smscInfo != null;
  }

  public Provider getProvider()
  {
    return provider;
  }

  public void setProvider(Provider provider)
  {
    this.provider = provider;
    this.smscInfo = null;
  }

  public SmscInfo getSmscInfo()
  {
    return smscInfo;
  }

  public void setSmscInfo(SmscInfo smscInfo)
  {
    this.smscInfo = smscInfo;
    this.provider = null;
  }

  public long getProviderId()
  {
    if (provider != null)
      return provider.getId();
    else
      return -1;
  }

  public String getProviderName()
  {
    if (provider != null)
      return provider.getName();
    else
      return null;
  }

  public void setProviderName(String name)
  {
    if (provider != null)
      provider.setName(name);
  }

  public String getSmscHost()
  {
    if (smscInfo != null)
      return smscInfo.getHost();
    else
      return null;
  }

  public void setSmscHost(String host)
  {
    if (smscInfo != null)
      smscInfo.setHost(host);
  }

  public int getSmscPort()
  {
    if (smscInfo != null)
      return smscInfo.getPort();
    else
      return -1;
  }

  public void setSmscPort(int port)
  {
    if (smscInfo != null)
      smscInfo.setPort(port);
  }

  public String getSmscSystemId()
  {
    if (smscInfo != null)
      return smscInfo.getSystemId();
    else
      return null;
  }

  public void setSmscSystemId(String systemId)
  {
    if (smscInfo != null)
      smscInfo.setSystemId(systemId);
  }

  public String getSmscPassword()
  {
    if (smscInfo != null)
      return smscInfo.getPassword();
    else
      return null;
  }

  public void setSmscPassword(String password)
  {
    if (smscInfo != null)
      smscInfo.setPassword(password);
  }

  public int getSmscResponseTimeout()
  {
    if (smscInfo != null)
      return smscInfo.getResponseTimeout();
    else
      return -1;
  }

  public void setSmscResponseTimeout(int responseTimeout)
  {
    if (smscInfo != null)
      smscInfo.setResponseTimeout(responseTimeout);
  }

  public int getSmscUniqueMsgIdPrefix()
  {
    if (smscInfo != null)
      return smscInfo.getUniqueMsgIdPrefix();
    else
      return -1;
  }

  public void setSmscUniqueMsgIdPrefix(int uniqueMsgIdPrefix)
  {
    if (smscInfo != null)
      smscInfo.setUniqueMsgIdPrefix(uniqueMsgIdPrefix);
  }
}
