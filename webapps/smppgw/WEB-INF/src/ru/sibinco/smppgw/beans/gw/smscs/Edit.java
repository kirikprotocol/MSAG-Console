package ru.sibinco.smppgw.beans.gw.smscs;

import ru.sibinco.lib.backend.util.SortedList;
import ru.sibinco.smppgw.Constants;
import ru.sibinco.smppgw.backend.sme.GwSme;
import ru.sibinco.smppgw.backend.sme.SmscInfo;
import ru.sibinco.smppgw.beans.*;

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

  protected void load(final String loadId) throws SmppgwJspException
  {
    final Map smscs = appContext.getSmscsManager().getSmscs();
    if (!smscs.containsKey(loadId))
      throw new SmppgwJspException(Constants.errors.smscs.SMSC_NOT_FOUND, loadId);

    final SmscInfo info = (SmscInfo) smscs.get(loadId);
    this.id = info.getId();
    this.host = info.getHost();
    this.port = info.getPort();
    this.systemId = info.getSystemId();
    this.password = info.getPassword();
    this.responseTimeout = info.getResponseTimeout();
    this.uniqueMsgIdPrefix = info.getUniqueMsgIdPrefix();
  }

  protected void save() throws SmppgwJspException
  {
    final Map smscs = getGlobalSmscs();
    final Map smes = getGlobalSmes();
    final SmscInfo info = new SmscInfo(id, host, port, systemId, password, responseTimeout, uniqueMsgIdPrefix);
    smscs.put(id, info);
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
    appContext.getStatuses().setSmscsChanged(true);
    throw new DoneException();
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
}
