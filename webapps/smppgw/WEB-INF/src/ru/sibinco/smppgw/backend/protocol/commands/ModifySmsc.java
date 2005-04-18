package ru.sibinco.smppgw.backend.protocol.commands;

import ru.sibinco.lib.backend.protocol.Command;
import ru.sibinco.lib.SibincoException;
import ru.sibinco.smppgw.backend.sme.SmscInfo;

/**
 * Created by IntelliJ IDEA.
 * User: Andrey
 * Date: 18.04.2005
 * Time: 19:16:09
 * To change this template use File | Settings | File Templates.
 */
public class ModifySmsc extends Command
{
  public ModifySmsc(SmscInfo smscInfo) throws SibincoException

  {
    super("modifySmsc", "file:///command_gw.dtd");
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
