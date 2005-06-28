package ru.sibinco.scag.beans.gw.smscs;

import ru.sibinco.lib.backend.util.SortedList;
import ru.sibinco.lib.backend.util.config.Config;
import ru.sibinco.lib.backend.protocol.Proxy;
import ru.sibinco.lib.SibincoException;
import ru.sibinco.scag.Constants;
import ru.sibinco.scag.backend.sme.GwSme;
import ru.sibinco.scag.backend.sme.SmscInfo;
import ru.sibinco.scag.backend.Gateway;
import ru.sibinco.scag.beans.*;

import java.util.Iterator;
import java.util.Map;


/**
 * Created by igork Date: 22.03.2004 Time: 20:04:20
 */
public class Edit extends EditBean
{
  private String id;
  private String host;
  private int port;
  private String systemId;
  private String password;
  private int responseTimeout;
  private int uniqueMsgIdPrefix;
  private String altHost;
  private int altPort;

  protected void load(final String loadId) throws SCAGJspException
  {
    final Map smscs = appContext.getSmscsManager().getSmscs();
    if (!smscs.containsKey(loadId))
      throw new SCAGJspException(Constants.errors.smscs.SMSC_NOT_FOUND, loadId);

    final SmscInfo info = (SmscInfo) smscs.get(loadId);
    this.id = info.getId();
    this.host = info.getHost();
    this.port = info.getPort();
    this.systemId = info.getSystemId();
    this.password = info.getPassword();
    this.responseTimeout = info.getResponseTimeout();
    this.uniqueMsgIdPrefix = info.getUniqueMsgIdPrefix();
    this.altHost = info.getAltHost();
    this.altPort = info.getAltPort();
  }

  protected void save() throws SCAGJspException
  {
    final Map smscs = getGlobalSmscs();
    final Map smes = getGlobalSmes();
    if (altHost==null) altHost="";
    final SmscInfo info = new SmscInfo(id, host, port, systemId, password, responseTimeout, uniqueMsgIdPrefix, altHost, altPort);
    if (isAdd()) {
      if (smes.containsKey(id)) {
        final GwSme sme = (GwSme) smes.get(id);
        sme.setSmscInfo(info);
      }
    } else {
      if (smes.containsKey(getEditId())) {
        final GwSme oldSme = (GwSme) smes.get(getEditId());
        oldSme.setSmscInfo(null);
      }
      if (smes.containsKey(id)) {
        final GwSme sme = (GwSme) smes.get(id);
        sme.setSmscInfo(info);
      }
    }
    smscs.put(id, info);
    appContext.getSmscsManager().storeSmscs(appContext.getGwConfig());
    regSmscs(info,isAdd());
    throw new DoneException();
  }
   private void regSmscs(SmscInfo info,boolean Add) throws SCAGJspException
  {
    final Gateway gateway=appContext.getGateway();
      try {
       if (!Add) gateway.modifySmsc(info);
       else gateway.regSmsc(info);
      } catch (SibincoException e) {
        e.printStackTrace();  //To change body of catch statement use File | Settings | File Templates.
       if (Proxy.StatusConnected == appContext.getGateway().getStatus()) {
          logger.debug("Couldn't register Service center", e);
          throw new SCAGJspException(Constants.errors.status.COULDNT_APPLY_SMSCS, e);
        }
      }
  }

  private Map getGlobalSmscs()
  {
    return appContext.getSmscsManager().getSmscs();
  }

  private Map getGlobalSmes()
  {
    return appContext.getGwSmeManager().getSmes();
  }

  public String[] getSmes()
  {
    final SortedList smes = new SortedList(appContext.getGwSmeManager().getSmes().keySet());
    for (Iterator i = appContext.getSmscsManager().getSmscs().keySet().iterator(); i.hasNext();) {
      final String smscId = (String) i.next();
      if (!smscId.equals(id))
        smes.remove(smscId);
    }
    return (String[]) smes.toArray(new String[0]);
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

  public String getAltHost()
  {
    return altHost;
  }

  public void setAltHost(final String altHost)
  {
    this.altHost = altHost;
  }

  public int getAltPort()
  {
    return altPort;
  }

  public void setAltPort(final int altPort)
  {
    this.altPort = altPort;
  }
}
