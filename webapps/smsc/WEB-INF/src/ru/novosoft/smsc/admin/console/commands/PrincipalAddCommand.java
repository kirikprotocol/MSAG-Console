/**
 * Created by IntelliJ IDEA.
 * User: makarov
 * Date: Mar 7, 2003
 * Time: 4:33:17 PM
 * To change this template use Options | File Templates.
 */
package ru.novosoft.smsc.admin.console.commands;

import ru.novosoft.smsc.admin.console.Command;
import ru.novosoft.smsc.admin.console.CommandContext;
import ru.novosoft.smsc.admin.dl.DistributionListAdmin;
import ru.novosoft.smsc.admin.dl.Principal;
import ru.novosoft.smsc.admin.dl.exceptions.PrincipalAlreadyExistsException;

public class PrincipalAddCommand implements Command
{
    private String address = null;
    private int maxLists = 0;
    private int maxElements = 0;

    public void setAddress(String address) {
        this.address = address;
    }
    public void setMaxLists(int maxLists) {
        this.maxLists = maxLists;
    }
    public void setMaxElements(int maxElements) {
        this.maxElements = maxElements;
    }

    public void process(CommandContext ctx)
    {
        String out = "Principal for '"+address+"'";
        try {
            DistributionListAdmin admin = ctx.getSmsc().getDistributionListAdmin();
            admin.addPrincipal(new Principal(address, maxLists, maxElements));
            ctx.setMessage(out+" added");
            ctx.setStatus(ctx.CMD_OK);
        } catch (PrincipalAlreadyExistsException e) {
            ctx.setMessage(out+" already exists");
            ctx.setStatus(CommandContext.CMD_PROCESS_ERROR);
        } catch (Exception e) {
            ctx.setMessage("Couldn't add "+out+". Cause: "+e.getMessage());
            ctx.setStatus(CommandContext.CMD_PROCESS_ERROR);
        }
    }

    public String getId() {
        return "PRINCIPAL_ADD";
    }
}
