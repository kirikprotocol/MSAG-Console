/**
 * Created by IntelliJ IDEA.
 * User: makarov
 * Date: Dec 16, 2002
 * Time: 6:57:16 PM
 * To change this template use Options | File Templates.
 */
package ru.novosoft.smsc.admin.console.commands;

import ru.novosoft.smsc.admin.console.CommandContext;
import ru.novosoft.smsc.admin.route.Mask;
import ru.novosoft.smsc.admin.profiler.Profile;
import ru.novosoft.smsc.jsp.SMSCErrors;

public class ProfileAddCommand extends ProfileGenCommand
{
    private String mask;

    public void setMask(String mask) {
        this.mask = mask;
    }

    public void process(CommandContext ctx)
    {
        String out = "Profile for mask '"+mask+"'";
        try {
            Mask profileMask = new Mask(mask);
            Profile profile = new Profile(profileMask, codepage, report);
            switch (ctx.getSmsc().updateProfile(profileMask, profile))
            {
                case 1:	//pusUpdated
                    ctx.setMessage(out+" was updated");
                    ctx.setStatus(CommandContext.CMD_OK);
                    break;
                case 2: //pusInserted
                    ctx.setMessage(out+" added");
                    ctx.setStatus(CommandContext.CMD_OK);
                    break;
                case 3: //pusUnchanged
                    ctx.setMessage(out+" is identical to default.");
                    ctx.setStatus(CommandContext.CMD_PROCESS_ERROR);
                    break;
                default: // pusError
                    ctx.setMessage("Couldn't add "+out+". Unknown cause");
                    ctx.setStatus(CommandContext.CMD_PROCESS_ERROR);
                    break;
            }
        } catch (Exception e) {
            ctx.setMessage("Couldn't add "+out+". Cause: "+e.getMessage());
            ctx.setStatus(CommandContext.CMD_PROCESS_ERROR);
        }
    }

    public String getId() {
        return "PROFILE_ADD";
    }

}

