/**
 * Created by IntelliJ IDEA.
 * User: makarov
 * Date: Mar 7, 2003
 * Time: 4:58:55 PM
 * To change this template use Options | File Templates.
 */
package ru.novosoft.smsc.admin.console.commands;

import ru.novosoft.smsc.admin.console.Command;
import ru.novosoft.smsc.admin.console.CommandContext;
import ru.novosoft.smsc.admin.dl.DistributionListAdmin;
import ru.novosoft.smsc.admin.dl.Principal;
import ru.novosoft.smsc.admin.dl.exceptions.PrincipalNotExistsException;

public class PrincipalViewCommand implements Command
{
    private String address = null;

    public void setAddress(String address) {
        this.address = address;
    }

    public void process(CommandContext ctx)
    {
        String out = "Principal for '"+address+"'";
        try {
            DistributionListAdmin admin = ctx.getSmsc().getDistributionListAdmin();
            Principal prc = admin.getPrincipal(address);
            if (prc == null) throw new PrincipalNotExistsException(address);
            ctx.setMessage(out+", maxLists: "+prc.getMaxLists()+
                               ", maxElemets: "+prc.getMaxElements());
            ctx.setStatus(ctx.CMD_OK);
        } catch (PrincipalNotExistsException e) {
            ctx.setMessage(out+" not exists");
            ctx.setStatus(CommandContext.CMD_PROCESS_ERROR);
        } catch (Exception e) {
            ctx.setMessage("Couldn't view "+out+". Cause: "+e.getMessage());
            ctx.setStatus(CommandContext.CMD_PROCESS_ERROR);
        }
    }

    public String getId() {
        return "PRINCIPAL_VIEW";
    }
}