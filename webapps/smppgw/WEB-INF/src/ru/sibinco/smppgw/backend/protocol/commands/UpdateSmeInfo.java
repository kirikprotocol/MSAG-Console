package ru.sibinco.smppgw.backend.protocol.commands;

import ru.sibinco.lib.backend.protocol.Command;
import ru.sibinco.lib.backend.sme.Sme;
import ru.sibinco.lib.SibincoException;
import ru.sibinco.smppgw.backend.sme.GwSme;


/**
 * Created by igork Date: 20.08.2004 Time: 17:13:35
 */
public class UpdateSmeInfo extends Command
{
  public UpdateSmeInfo(final GwSme gwSme)
      throws SibincoException
  {
    super("updateSmeInfo", "file:///command_gw.dtd");
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
