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
        if (!isCodepage && !isReport) {
            ctx.setMessage("expecting 'encoding' or 'report' option. "+
                           "Syntax: alter profile <profile_address> "+
                           "[report (full|none)] [encoding (ucs2|default)]");
            ctx.setStatus(CommandContext.CMD_PARSE_ERROR);
        }
        String out = "Profile for address '"+address+"'";
        try {
            Mask profileMask = new Mask(address);
            Profile profile = ctx.getSmsc().lookupProfile(profileMask);
            if (profile == null) {
                ctx.setMessage(out+" not found");
                ctx.setStatus(CommandContext.CMD_PROCESS_ERROR);
            } else {
                String beforeCp = profile.getCodepageString();
                String beforeRp = profile.getReportOptionsString();
                if (isCodepage) {
                    profile.setCodepage(codepage);
                    System.out.println("profileAlterCmd: Cp set "+codepage);
                }
                if (isReport)   {
                    profile.setReportOptions(report);
                    System.out.println("profileAlterCmd: Rp set "+report);
                }
                switch (ctx.getSmsc().updateProfile(profileMask, profile))
                {
                    case 1:	//pusUpdated
                        ctx.setMessage(out+" altered");
                        ctx.setStatus(CommandContext.CMD_OK);
                        break;
                    case 2: //pusInserted
                        ctx.setMessage(out+" added new");
                        ctx.setStatus(CommandContext.CMD_OK);
                        break;
                    case 3: //pusUnchanged
                        ctx.setMessage(out+" unchanged.");
                        ctx.setStatus(CommandContext.CMD_OK);
                        break;
                    default: // pusError
                        ctx.setMessage("Couldn't alter "+out+". Unknown cause");
                        ctx.setStatus(CommandContext.CMD_PROCESS_ERROR);
                        break;
                }
                String afterCp = profile.getCodepageString();
                String afterRp = profile.getReportOptionsString();
                System.out.println("profileAlterCmd Cp: "+beforeCp+" -> "+afterCp+" req: "+codepage);
                System.out.println("profileAlterCmd Rp: "+beforeRp+" -> "+afterRp+" req: "+report);
            }
        } catch (Exception e) {
            ctx.setMessage("Couldn't alter "+out+". Cause: "+e.getMessage());
            ctx.setStatus(CommandContext.CMD_PROCESS_ERROR);
        }
    }

    public String getId() {
        return "PROFILE_ALTER";
    }

}

