/**
 * Created by IntelliJ IDEA.
 * User: makarov
 * Date: Mar 7, 2003
 * Time: 4:37:35 PM
 * To change this template use Options | File Templates.
 */
package ru.novosoft.smsc.admin.console.commands;

import ru.novosoft.smsc.admin.console.Command;
import ru.novosoft.smsc.admin.console.CommandContext;
import ru.novosoft.smsc.admin.dl.DistributionListAdmin;
import ru.novosoft.smsc.admin.dl.Principal;
import ru.novosoft.smsc.admin.dl.exceptions.PrincipalAlreadyExistsException;
import ru.novosoft.smsc.admin.dl.exceptions.PrincipalNotExistsException;

public class PrincipalAlterCommand implements Command
{
    private String address = null;
    private int maxLists = 0;
    private int maxElements = 0;

    private boolean setMaxLists = false;
    private boolean setMaxElements = false;

    public void setAddress(String address) {
        this.address = address;
    }
    public void setMaxLists(int maxLists) {
        this.maxLists = maxLists; setMaxLists = true;
    }
    public void setMaxElements(int maxElements) {
        this.maxElements = maxElements; setMaxElements = true;
    }

    public void process(CommandContext ctx)
    {
        if (!setMaxLists && !setMaxElements) {
            ctx.setMessage("expecting 'numlist' and/or 'numelem' option. "+
                           "Syntax: alter principal <principal_address> "+
                           "[numlist <number>] [numelem <number>]");
            ctx.setStatus(CommandContext.CMD_PARSE_ERROR);
        } else {
            String out = "Principal for '"+address+"'";
            try {
                DistributionListAdmin admin = ctx.getSmsc().getDistributionListAdmin();
                admin.alterPrincipal(new Principal(address, maxLists, maxElements),
                                     setMaxLists, setMaxElements);
                ctx.setMessage(out+" altered");
                ctx.setStatus(ctx.CMD_OK);
            } catch (PrincipalNotExistsException e) {
                ctx.setMessage(out+" not exists");
                ctx.setStatus(CommandContext.CMD_PROCESS_ERROR);
            } catch (Exception e) {
                ctx.setMessage("Couldn't alter "+out+". Cause: "+e.getMessage());
                ctx.setStatus(CommandContext.CMD_PROCESS_ERROR);
            }
        }
    }

    public String getId() {
        return "PRINCIPAL_ALTER";
    }
}
