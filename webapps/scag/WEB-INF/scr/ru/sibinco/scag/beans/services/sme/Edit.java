package ru.sibinco.scag.beans.services.sme;

import ru.sibinco.lib.SibincoException;
import ru.sibinco.lib.backend.protocol.Proxy;
import ru.sibinco.lib.backend.sme.Sme;
import ru.sibinco.scag.Constants;
import ru.sibinco.scag.backend.Gateway;
import ru.sibinco.scag.backend.sme.*;
import ru.sibinco.scag.beans.*;

import java.util.*;


/**
 * Created by igork Date: 18.03.2004 Time: 16:42:01
 */
public class Edit extends EditBean
{
  protected String id = null;
  protected int priority = 0;
  protected byte type = Sme.SMPP;
  protected int typeOfNumber = 0;
  protected int numberingPlan = 0;
  protected int interfaceVersion = 0;
  protected String systemType = null;
  protected String password = null;
  protected String addrRange = null;
  protected int timeout = 0;
  protected String receiptSchemeName = null;
  protected boolean disabled = false;
  protected byte mode = Sme.MODE_TRX;
  protected int proclimit = 0;
  protected boolean smsc = false;
  protected long providerId = -1;


  public String getId()
  {
    return id;
  }

  protected void load(final String loadId) throws SCAGJspException
  {
    final GwSme sme = (GwSme) appContext.getGwSmeManager().getSmes().get(loadId);
    if (null == sme)
      throw new SCAGJspException(Constants.errors.sme.SME_NOT_FOUND, loadId);

    this.id = sme.getId();
    this.priority = sme.getPriority();
    this.type = sme.getType();
    this.typeOfNumber = sme.getTypeOfNumber();
    this.numberingPlan = sme.getNumberingPlan();
    this.interfaceVersion = sme.getInterfaceVersion();
    this.systemType = sme.getSystemType();
    this.password = sme.getPassword();
    this.addrRange = sme.getAddrRange();
    this.timeout = sme.getTimeout();
    this.receiptSchemeName = sme.getReceiptSchemeName();
    this.disabled = sme.isDisabled();
    this.mode = sme.getMode();
    this.proclimit = sme.getProclimit();
    if (sme.isSmsc()) {
      this.smsc = true;
      this.providerId = -1;
    } else {
      this.smsc = false;
      this.providerId = sme.getProviderId();
    }
  }

  protected void save() throws SCAGJspException
  {
    if (null == id || 0 == id.length() || !isAdd() && (null == getEditId() || 0 == getEditId().length()))
      throw new SCAGJspException(Constants.errors.sme.SME_ID_NOT_SPECIFIED);

    if (null == password)
      password = "";

    final Map smes = appContext.getGwSmeManager().getSmes();
    if (smes.containsKey(id) && (isAdd() || !id.equals(getEditId())))
      throw new SCAGJspException(Constants.errors.sme.SME_ALREADY_EXISTS, id);
    smes.remove(getEditId());
    final GwSme newGwSme;
    if (this.smsc) {
      newGwSme = new GwSme(id, priority, type, typeOfNumber, numberingPlan, interfaceVersion, systemType, password, addrRange, timeout, receiptSchemeName,
                           disabled, mode, proclimit, (SmscInfo) appContext.getSmscsManager().getSmscs().get(id));
      smes.put(id, newGwSme);
    } else {
      newGwSme = new GwSme(id, priority, type, typeOfNumber, numberingPlan, interfaceVersion, systemType, password, addrRange, timeout, receiptSchemeName,
                           disabled, mode, proclimit, (Provider) appContext.getProviderManager().getProviders().get(new Long(this.providerId)));
      smes.put(id, newGwSme);
    }
    final Gateway gateway = appContext.getGateway();
    try {
      if (isAdd())
        gateway.addSme(newGwSme);
      else
        gateway.updateSmeInfo(newGwSme);
      appContext.getGwSmeManager().store();
    } catch (SibincoException e) {
      if (Proxy.StatusConnected == gateway.getStatus()) {
        throw new SCAGJspException(Constants.errors.sme.COULDNT_APPLY, id, e);
      }
    }
    throw new DoneException();
  }


  public void setId(final String id)
  {
    this.id = id;
  }

  public int getPriority()
  {
    return priority;
  }

  public void setPriority(final int priority)
  {
    this.priority = priority;
  }

  public byte getType()
  {
    return type;
  }

  public void setType(final byte type)
  {
    this.type = type;
  }

  public int getTypeOfNumber()
  {
    return typeOfNumber;
  }

  public void setTypeOfNumber(final int typeOfNumber)
  {
    this.typeOfNumber = typeOfNumber;
  }

  public int getNumberingPlan()
  {
    return numberingPlan;
  }

  public void setNumberingPlan(final int numberingPlan)
  {
    this.numberingPlan = numberingPlan;
  }

  public int getInterfaceVersion()
  {
    return interfaceVersion;
  }

  protected void setInterfaceVersion(final int interfaceVersion)
  {
    this.interfaceVersion = interfaceVersion;
  }

  public String getSystemType()
  {
    return systemType;
  }

  public void setSystemType(final String systemType)
  {
    this.systemType = systemType;
  }

  public String getPassword()
  {
    return password;
  }

  public void setPassword(final String password)
  {
    this.password = password;
  }

  public String getAddrRange()
  {
    return addrRange;
  }

  public void setAddrRange(final String addrRange)
  {
    this.addrRange = addrRange;
  }

  public int getTimeout()
  {
    return timeout;
  }

  public void setTimeout(final int timeout)
  {
    this.timeout = timeout;
  }

  public String getReceiptSchemeName()
  {
    return receiptSchemeName;
  }

  public void setReceiptSchemeName(final String receiptSchemeName)
  {
    this.receiptSchemeName = receiptSchemeName;
  }

  public boolean isDisabled()
  {
    return disabled;
  }

  public void setDisabled(final boolean disabled)
  {
    this.disabled = disabled;
  }

  public byte getMode()
  {
    return mode;
  }

  public void setMode(final byte mode)
  {
    this.mode = mode;
  }

  public int getProclimit()
  {
    return proclimit;
  }

  public void setProclimit(final int proclimit)
  {
    this.proclimit = proclimit;
  }

  public boolean isSmsc()
  {
    return smsc;
  }

  public void setSmsc(final boolean smsc)
  {
    this.smsc = smsc;
  }

  public long getProviderId()
  {
    return providerId;
  }

  public void setProviderId(final long providerId)
  {
    this.providerId = providerId;
  }

  public String[] getProviderIds()
  {
    final Map providers = new TreeMap(appContext.getProviderManager().getProviders());
    final List result = new ArrayList(providers.size());
    for (Iterator i = providers.keySet().iterator(); i.hasNext();) {
      result.add(String.valueOf(((Long) i.next()).longValue()));
    }
    return (String[]) result.toArray(new String[0]);
  }

  public String[] getProviderTitles()
  {
    final Map providers = new TreeMap(appContext.getProviderManager().getProviders());
    final List result = new ArrayList(providers.size());
    for (Iterator i = providers.values().iterator(); i.hasNext();) {
      result.add(((Provider) i.next()).getName());
    }
    return (String[]) result.toArray(new String[0]);
  }
}
