/**
 * Created by IntelliJ IDEA.
 * User: makarov
 * Date: Dec 16, 2002
 * Time: 6:55:34 PM
 * To change this template use Options | File Templates.
 */
package ru.novosoft.smsc.admin.console.commands;


import ru.novosoft.smsc.admin.console.CommandContext;
import ru.novosoft.smsc.admin.route.Mask;
import ru.novosoft.smsc.admin.profiler.Profile;

public class ProfileAlterCommand extends ProfileGenCommand
{
    private String address;

    public void setAddress(String address) {
        this.address = address;
    }

    public void process(CommandContext ctx)
    {
        String out = "Profile for address '"+address+"' ";
        try {
            Mask profileMask = new Mask(address);
            Profile profile = ctx.getSmsc().lookupProfile(profileMask);
            if (profile == null) {
                ctx.setMessage(out+"not exists");
                ctx.setStatus(CommandContext.CMD_PROCESS_ERROR);
            } else {
                profile.setCodepage(codepage);
                profile.setReportOptions(report);
                ctx.getSmsc().updateProfile(profileMask, profile);
                ctx.setMessage(out+"altered");
                ctx.setStatus(CommandContext.CMD_OK);
            }
        } catch (Exception e) {
            ctx.setMessage("Failed to alter "+out+". Cause: "+e.getMessage());
            ctx.setStatus(CommandContext.CMD_PROCESS_ERROR);
        }
    }

    public String getId() {
        return "PROFILE_ALTER";
    }

}

