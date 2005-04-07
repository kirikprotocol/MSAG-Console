package ru.sibinco.smppgw.backend.protocol.commands;

import ru.sibinco.lib.backend.protocol.Command;
import ru.sibinco.lib.SibincoException;
import ru.sibinco.smppgw.backend.sme.SmscInfo;

/**
 * Created by IntelliJ IDEA.
 * User: Andrey
 * Date: 07.04.2005
 * Time: 14:23:41
 * To change this template use File | Settings | File Templates.
 */
public class RegSmsc extends Command
{
  public RegSmsc(SmscInfo smscInfo) throws SibincoException

  {
    super("regSmsc", "file:///command_gw.dtd");
    fillParams(smscInfo);
  }
   protected void fillParams(final SmscInfo smscInfo)
  {
    createStringParam("id", smscInfo.getId());
    createStringParam("altHost", smscInfo.getAltHost());
    createIntParam("altPort", smscInfo.getAltPort());
    createStringParam("host", smscInfo.getHost());
    createStringParam("password", smscInfo.getPassword());
    createIntParam("port", smscInfo.getPort());
    createIntParam("responceTimeout", smscInfo.getResponseTimeout());
    createStringParam("systemId", smscInfo.getSystemId());
    createIntParam("uniqueMsgIdPrefix", smscInfo.getUniqueMsgIdPrefix());
  }
}
