package ru.sibinco.smppgw.backend;

import ru.sibinco.lib.SibincoException;
import ru.sibinco.lib.backend.daemon.ServiceInfo;
import ru.sibinco.lib.backend.protocol.Proxy;
import ru.sibinco.lib.backend.protocol.Response;
import ru.sibinco.smppgw.backend.protocol.commands.Apply;
import ru.sibinco.smppgw.backend.protocol.commands.UpdateSmeInfo;
import ru.sibinco.smppgw.backend.sme.GwSme;


/**
 * Created by igork Date: 25.05.2004 Time: 15:50:34
 */
public class Gateway extends Proxy
{
  private final String id;

  public Gateway(final ServiceInfo gwServiceInfo)
  {
    super(gwServiceInfo.getHost(), gwServiceInfo.getPort());
    id = gwServiceInfo.getId();
  }

  public void apply(final String subject) throws SibincoException
  {
    final Response response = super.runCommand(new Apply(subject));
    if (Response.StatusOk != response.getStatus())
      throw new SibincoException("Couldn't apply, nested: " + response.getStatusString() + " \"" + response.getDataAsString() + '"');
  }

  public void updateSmeInfo(final GwSme gwSme) throws SibincoException
  {
    final Response response = super.runCommand(new UpdateSmeInfo(gwSme));
    if (Response.StatusOk != response.getStatus())
      throw new SibincoException("Couldn't apply, nested: " + response.getStatusString() + " \"" + response.getDataAsString() + '"');
  }

  public String getId()
  {
    return id;
  }
}
