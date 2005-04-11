package ru.novosoft.smsc.admin.console.commands;

import ru.novosoft.smsc.admin.console.CommandContext;
import ru.novosoft.smsc.admin.smsc_service.SmeManager;
import ru.novosoft.smsc.admin.route.SME;

import java.util.ArrayList;
import java.util.List;

/**
 * Created by IntelliJ IDEA.
 * User: makar
 * Date: 11.04.2005
 * Time: 17:39:33
 * To change this template use File | Settings | File Templates.
 */
public class SmeAlterCommand extends SmeGenCommand
{
    public void process(CommandContext ctx)
    {
        boolean disconnectOnly = false;
        if (!isAddressRange && !isDisabled && !isForceDC && !isInterfaceVersion && !isMode &&
            !isNPI && !isTON && !isPassword && !isPriority && !isProclimit && !isSchedlimit && !isType &&
            !isReceiptScheme && !isSmeN && !isSystemType && !isTimeout && !isWantAlias)
        {
            if (!disconnect) {
                ctx.setMessage("No one alter option specified");
                ctx.setStatus(CommandContext.CMD_PARSE_ERROR);
                return;
            }
            disconnectOnly = true;
        }

        String out = "SME '"+smeId+"'";
        try {
            SmeManager manager = ctx.getSmeManager();
            if (!manager.contains(smeId)) throw new Exception("SME not exists");
            if (disconnect && ctx.getSmsc().getSmeStatus(smeId).isConnected()) {
                List toDisconnect = new ArrayList(); toDisconnect.add(smeId);
                manager.disconnectSmes(toDisconnect);
            }
            if (!disconnectOnly) {
                SME sme = manager.get(smeId);
                SME newSme = new SME(smeId, isPriority ? priority:sme.getPriority(), isType ? type:sme.getType(),
                        isTON ? TON:sme.getTypeOfNumber(), isNPI ? NPI:sme.getNumberingPlan(),
                        isInterfaceVersion ? interfaceVersion:sme.getInterfaceVersion(),
                        isSystemType ? systemType:sme.getSystemType(), isPassword ? password:sme.getPassword(),
                        isAddressRange ? addressRange:sme.getAddrRange(), isSmeN ? smeN:sme.getSmeN(),
                        isWantAlias ? wantAlias:sme.isWantAlias(), isForceDC ? forceDC:sme.isForceDC(),
                        isTimeout ? timeout:sme.getTimeout(), isReceiptScheme ? receiptScheme:sme.getReceiptSchemeName(),
                        isDisabled ? disabled:sme.isDisabled(), isMode ? mode:sme.getMode(),
                        isProclimit ? proclimit:sme.getProclimit(), isSchedlimit ? schedlimit:sme.getSchedlimit());
                manager.update(newSme);
            }
            ctx.setMessage(out+" altered");
            ctx.setStatus(CommandContext.CMD_OK);
        } catch (Exception e) {
            ctx.setMessage("Couldn't alter "+out+". Cause: "+e.getMessage());
            ctx.setStatus(CommandContext.CMD_PROCESS_ERROR);
        }
    }

    public String getId() {
        return "SME_ALTER";
    }
}
