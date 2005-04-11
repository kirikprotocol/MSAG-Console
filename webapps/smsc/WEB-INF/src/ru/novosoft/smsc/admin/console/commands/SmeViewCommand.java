package ru.novosoft.smsc.admin.console.commands;

import ru.novosoft.smsc.admin.console.CommandContext;
import ru.novosoft.smsc.admin.smsc_service.SmeManager;
import ru.novosoft.smsc.admin.route.SME;

/**
 * Created by IntelliJ IDEA.
 * User: makar
 * Date: 11.04.2005
 * Time: 17:41:03
 * To change this template use File | Settings | File Templates.
 */
public class SmeViewCommand extends SmeGenCommand
{
    public void process(CommandContext ctx)
    {
        String out = "SME '"+smeId+"'";
        try {
            SmeManager manager = ctx.getSmeManager();
            if (!manager.contains(smeId)) throw new Exception("SME not exists");
            SME sme = manager.get(smeId);
            type = sme.getType();
            ctx.setMessage("SME info");
            ctx.setStatus(CommandContext.CMD_LIST);
            ctx.addResult("id: " + sme.getId() + ", smeN: " + sme.getSmeN());
            ctx.addResult("type: " + ((type == SME.SMPP) ? "SMPP":(type == SME.SS7) ? "SS7":"UNKNOWN ("+type+")"));
            ctx.addResult("mode: " + sme.getModeStr().toUpperCase());
            ctx.addResult("interfaceVersion: " + sme.getInterfaceVersion());
            ctx.addResult("systemType: " + sme.getSystemType());
            ctx.addResult("receipt scheme: " + sme.getReceiptSchemeName());
            ctx.addResult("priority: " + sme.getPriority());
            ctx.addResult("TON: " + sme.getTypeOfNumber() + ", NPI: " + sme.getNumberingPlan());
            ctx.addResult("address range: " + sme.getAddrRange());
            ctx.addResult("password: " + sme.getPassword());
            ctx.addResult("timeout: " + sme.getTimeout());
            ctx.addResult("proc/sched limit: " + sme.getProclimit() + "/" + sme.getSchedlimit());
            ctx.addResult("flags: " + (sme.isDisabled() ? "disabled":"enabled") +
                           ", wantAlias " + (sme.isWantAlias() ? "yes":"no") +
                           ", forceDC " + (sme.isForceDC() ? "yes":"no"));
            String connected = "unknown";
            try { connected = ctx.getSmsc().getSmeStatus(smeId).isConnected() ? "connected":"disconnected"; }
            catch (Exception exc) { exc.printStackTrace(); }
            ctx.addResult("current status: " + connected);
        } catch (Exception e) {
            ctx.setMessage("Couldn't view "+out+". Cause: "+e.getMessage());
            ctx.setStatus(CommandContext.CMD_PROCESS_ERROR);
        }
    }

    public String getId() {
        return "SME_VIEW";
    }

}
