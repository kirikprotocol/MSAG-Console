package ru.novosoft.smsc.admin.console.commands.sme;

import ru.novosoft.smsc.admin.console.CommandContext;
import ru.novosoft.smsc.admin.route.SME;
import ru.novosoft.smsc.admin.route.Subject;
import ru.novosoft.smsc.admin.smsc_service.SmeManager;

/**
 * Created by IntelliJ IDEA.
 * User: makar
 * Date: 11.04.2005
 * Time: 16:50:12
 * To change this template use File | Settings | File Templates.
 */
public class SmeAddCommand extends SmeGenCommand
{
    public void process(CommandContext ctx)
    {
        String out = "SME '"+smeId+"'";
        try {
            SmeManager manager = ctx.getSmeManager();
            if (manager.contains(smeId)) throw new Exception("SME already exists");
            manager.add(smeId, priority, type, TON, NPI, interfaceVersion, systemType, password,
                        addressRange, smeN, wantAlias, forceDC, timeout, receiptScheme,
                        disabled, mode, proclimit, schedlimit);
            ctx.setMessage(out+" added");
            ctx.setStatus(CommandContext.CMD_OK);
        } catch (Exception e) {
            ctx.setMessage("Couldn't add "+out+". Cause: "+e.getMessage());
            ctx.setStatus(CommandContext.CMD_PROCESS_ERROR);
        }
    }

    public String getId() {
        return "SME_ADD";
    }
}
