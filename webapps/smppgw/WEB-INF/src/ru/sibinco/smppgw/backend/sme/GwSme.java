package ru.sibinco.smppgw.backend.sme;

import ru.sibinco.lib.backend.sme.Sme;

import java.io.PrintWriter;


/**
 * Created by igork Date: 22.03.2004 Time: 19:00:30
 */
public class GwSme extends Sme
{
  private Provider provider;
  private SmscInfo smscInfo;

  public GwSme(final String id, final int priority, final byte type, final int typeOfNumber, final int numberingPlan, final int interfaceVersion,
               final String systemType, final String password, final String addrRange, final int smeN, final boolean wantAlias, final boolean forceDC,
               final int timeout, final String receiptSchemeName, final boolean disabled, final byte mode, final int proclimit, final int schedlimit,
               final Provider provider)
      throws NullPointerException
  {
    super(id, priority, type, typeOfNumber, numberingPlan, interfaceVersion, systemType, password, addrRange, smeN, wantAlias, forceDC, timeout,
          receiptSchemeName, disabled, mode, proclimit, schedlimit);
    this.provider = provider;
    this.smscInfo = null;
  }

  public GwSme(final String id, final int priority, final byte type, final int typeOfNumber, final int numberingPlan, final int interfaceVersion,
               final String systemType, final String password, final String addrRange, final int smeN, final boolean wantAlias, final boolean forceDC,
               final int timeout, final String receiptSchemeName, final boolean disabled, final byte mode, final int proclimit, final int schedlimit,
               final SmscInfo smscInfo)
      throws NullPointerException
  {
    super(id, priority, type, typeOfNumber, numberingPlan, interfaceVersion, systemType, password, addrRange, smeN, wantAlias, forceDC, timeout,
          receiptSchemeName, disabled, mode, proclimit, schedlimit);
    this.provider = null;
    this.smscInfo = smscInfo;
  }

  public GwSme(final Sme sme, final SmscInfo smscInfo)
      throws NullPointerException
  {
    super(sme);
    this.provider = null;
    this.smscInfo = smscInfo;
  }

  public GwSme(final Sme sme, final Provider provider)
      throws NullPointerException
  {
    super(sme);
    this.provider = provider;
    this.smscInfo = null;
  }

  public boolean isSmsc()
  {
    return null != smscInfo;
  }

  protected PrintWriter storeBody(final PrintWriter out)
  {
    super.storeBody(out);
    out.println("    <param name=\"providerId\"          value=\"" + provider.getId() + "\"/>");
    return out;
  }

  public Provider getProvider()
  {
    return provider;
  }

  public void setProvider(final Provider provider)
  {
    this.provider = provider;
    this.smscInfo = null;
  }

  public SmscInfo getSmscInfo()
  {
    return smscInfo;
  }

  public void setSmscInfo(final SmscInfo smscInfo)
  {
    this.smscInfo = smscInfo;
    this.provider = null;
  }

  public long getProviderId()
  {
    if (null != provider)
      return provider.getId();
    else
      return -1;
  }

  public String getProviderName()
  {
    if (null != provider)
      return provider.getName();
    else
      return null;
  }

  public void setProviderName(final String name)
  {
    if (null != provider)
      provider.setName(name);
  }

  public String getSmscHost()
  {
    if (null != smscInfo)
      return smscInfo.getHost();
    else
      return null;
  }

  public void setSmscHost(final String host)
  {
    if (null != smscInfo)
      smscInfo.setHost(host);
  }

  public int getSmscPort()
  {
    if (null != smscInfo)
      return smscInfo.getPort();
    else
      return -1;
  }

  public void setSmscPort(final int port)
  {
    if (null != smscInfo)
      smscInfo.setPort(port);
  }

  public String getSmscSystemId()
  {
    if (null != smscInfo)
      return smscInfo.getSystemId();
    else
      return null;
  }

  public void setSmscSystemId(final String systemId)
  {
    if (null != smscInfo)
      smscInfo.setSystemId(systemId);
  }

  public String getSmscPassword()
  {
    if (null != smscInfo)
      return smscInfo.getPassword();
    else
      return null;
  }

  public void setSmscPassword(final String password)
  {
    if (null != smscInfo)
      smscInfo.setPassword(password);
  }

  public int getSmscResponseTimeout()
  {
    if (null != smscInfo)
      return smscInfo.getResponseTimeout();
    else
      return -1;
  }

  public void setSmscResponseTimeout(final int responseTimeout)
  {
    if (null != smscInfo)
      smscInfo.setResponseTimeout(responseTimeout);
  }

  public int getSmscUniqueMsgIdPrefix()
  {
    if (null != smscInfo)
      return smscInfo.getUniqueMsgIdPrefix();
    else
      return -1;
  }

  public void setSmscUniqueMsgIdPrefix(final int uniqueMsgIdPrefix)
  {
    if (null != smscInfo)
      smscInfo.setUniqueMsgIdPrefix(uniqueMsgIdPrefix);
  }
}
