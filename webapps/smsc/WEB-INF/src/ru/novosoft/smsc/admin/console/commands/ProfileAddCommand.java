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

public class ProfileAddCommand extends ProfileGenCommand
{
    private String mask;

    public void setMask(String mask) {
        this.mask = mask;
    }

    public void process(CommandContext ctx)
    {
        String out = "Profile for mask '"+mask+"' ";
        try {
            Mask profileMask = new Mask(mask);
            Profile profile = new Profile(profileMask, codepage, report);
            ctx.getSmsc().updateProfile(profileMask, profile);
            ctx.setMessage(out+"added");
            ctx.setStatus(CommandContext.CMD_OK);
        } catch (Exception e) {
            ctx.setMessage("Failed to add "+out+". Cause: "+e.getMessage());
            ctx.setStatus(CommandContext.CMD_PROCESS_ERROR);
        }
    }

    public String getId() {
        return "PROFILE_ADD";
    }

}

