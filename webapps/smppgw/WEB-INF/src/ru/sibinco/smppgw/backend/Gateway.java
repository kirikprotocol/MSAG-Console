package ru.sibinco.smppgw.backend;

import ru.sibinco.lib.SibincoException;
import ru.sibinco.lib.backend.service.ServiceInfo;
import ru.sibinco.lib.backend.protocol.Proxy;
import ru.sibinco.lib.backend.protocol.Response;
import ru.sibinco.smppgw.backend.protocol.commands.*;
import ru.sibinco.smppgw.backend.sme.GwSme;


/**
 * Created by igork Date: 25.05.2004 Time: 15:50:34
 */
public class Gateway extends Proxy
{
  private final String id;

  public Gateway(final ServiceInfo gwServiceInfo, final int port)
  {
    super(gwServiceInfo.getHost(), port);
    id = gwServiceInfo.getId();
  }

  public String getId()
  {
    return id;
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
      throw new SibincoException("Couldn't update sme info, nested: " + response.getStatusString() + " \"" + response.getDataAsString() + '"');
  }

  public void deleteSme(final String smeId) throws SibincoException
  {
    final Response response = super.runCommand(new DeleteSme(smeId));
    if (Response.StatusOk != response.getStatus())
      throw new SibincoException("Couldn't delete sme, nested: " + response.getStatusString() + " \"" + response.getDataAsString() + '"');
  }

  public void addSme(final GwSme gwSme) throws SibincoException
  {
    final Response response = super.runCommand(new AddSme(gwSme));
    if (Response.StatusOk != response.getStatus())
      throw new SibincoException("Couldn't update sme info, nested: " + response.getStatusString() + " \"" + response.getDataAsString() + '"');
  }
}
