/**
 * Created by IntelliJ IDEA.
 * User: makarov
 * Date: Dec 16, 2002
 * Time: 6:58:28 PM
 * To change this template use Options | File Templates.
 */
package ru.novosoft.smsc.admin.console.commands;


import ru.novosoft.smsc.admin.console.Command;
import ru.novosoft.smsc.admin.console.CommandContext;
import ru.novosoft.smsc.admin.route.Mask;
import ru.novosoft.smsc.admin.profiler.Profile;
import ru.novosoft.smsc.admin.AdminException;

public class ProfileViewCommand implements Command
{
    private String address;

    public void setAddress(String address) {
        this.address = address;
    }

    private String showProfile(Profile profile)
        throws AdminException
    {
        return "Profile '"+profile.getMask().getMask()+"'"+
               " Report: "+profile.getReportOptionsString()+
               " Encoding: "+profile.getCodepageString();
    }

    public void process(CommandContext ctx)
    {
        String out = "Profile for address '"+address+"' ";
        try {
            Profile profile = ctx.getSmsc().lookupProfile(new Mask(address));
            if (profile != null) {
                ctx.setMessage(showProfile(profile));
                ctx.setStatus(CommandContext.CMD_OK);
            } else {
                ctx.setMessage(out+"not found");
                ctx.setStatus(CommandContext.CMD_PROCESS_ERROR);
            }
        } catch (Exception e) {
            ctx.setMessage("Failed to view "+out+". Cause: "+e.getMessage());
            ctx.setStatus(CommandContext.CMD_PROCESS_ERROR);
        }
    }
}

