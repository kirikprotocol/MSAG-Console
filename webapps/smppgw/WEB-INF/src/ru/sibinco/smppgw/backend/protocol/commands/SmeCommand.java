package ru.sibinco.smppgw.backend.protocol.commands;

import ru.sibinco.lib.SibincoException;
import ru.sibinco.lib.backend.protocol.Command;
import ru.sibinco.lib.backend.sme.Sme;
import ru.sibinco.smppgw.backend.sme.GwSme;


/**
 * Created by igork Date: 01.09.2004 Time: 14:50:37
 */
public class SmeCommand extends Command
{
  public SmeCommand(final String commandName, final String systemId)
      throws SibincoException
  {
    super(commandName, systemId);
  }

  protected void fillParams(final GwSme gwSme)
  {
    createStringParam("id", gwSme.getId());
    createIntParam("priority", gwSme.getPriority());
    createStringParam("type", (Sme.SMPP == gwSme.getType() ? "smpp" : "ss7"));
    createIntParam("typeOfNumber", gwSme.getTypeOfNumber());
    createIntParam("numberingPlan", gwSme.getNumberingPlan());
    createIntParam("interfaceVersion", gwSme.getInterfaceVersion());
    createStringParam("systemType", gwSme.getSystemType());
    createStringParam("password", gwSme.getPassword());
    createStringParam("addrRange", gwSme.getAddrRange());
    createIntParam("timeout", gwSme.getTimeout());
    createStringParam("receiptSchemeName", gwSme.getReceiptSchemeName());
    createBoolParam("disabled", gwSme.isDisabled());
    createStringParam("mode", gwSme.getModeStr());
    createIntParam("proclimit", gwSme.getProclimit());
    createBoolParam("smsc", gwSme.isSmsc());
    createIntParam("providerId", gwSme.getProviderId());
  }
}
