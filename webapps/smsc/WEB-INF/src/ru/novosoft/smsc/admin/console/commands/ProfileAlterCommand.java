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
        if (!isCodepage && !isReport && !isLocale && !isAliasHide && !isAliasModifiable &&
            isDivertOptions && !isDivert && !isDivertModifiable && !isDivertActiveAbsent &&
            !isDivertActiveBarred && !isDivertActiveBlocked &&  !isDivertActiveCapacity &&
            !isDivertActiveUnconditional && !isUdhConcat)
        {
            ctx.setMessage("expecting 'encoding', 'report', 'locale', 'alias', 'divert' option(s). "+
                           "Syntax: alter profile <profile_address> "+
                           "[report (full|none)] [locale <locale_name>] "+
                           "[encoding (default|ucs2|latin1|ucs2-latin1) [ussd7bit]] "+
                           "[alias [hide|nohide|substitute] [modifiable|notmodifiable]] "+
                           "[divert [(set <divert>)|clear] [(on|off) [absent][barred][blocked][capacity][unconditional]] "+
                           "[modifiable|notmodifiable]] [udhconcat on|off]");
            ctx.setStatus(CommandContext.CMD_PARSE_ERROR);
            return;
        }
        String out = "Profile for address '"+address+"'";
        try {
            Mask profileMask = new Mask(address);
            Profile profile = ctx.getSmsc().profileLookup(profileMask);
            if (profile == null) {
                ctx.setMessage(out+" not found");
                ctx.setStatus(CommandContext.CMD_PROCESS_ERROR);
            }
            else
            {
                if (isCodepage) {
                  profile.setCodepage(codepage);
                  profile.setUssd7bit(ussd7bit);
                }
                if (isReport) profile.setReportOptions(report);
                if (isAliasHide) profile.setAliasHide(aliasHide);
                if (isAliasModifiable) profile.setAliasModifiable(aliasModifiable);
                if (isDivert) profile.setDivert(divert);
                if (isDivertActiveAbsent) profile.setDivertActiveAbsent(divertActiveOn ? divertActiveAbsent:false);
                if (isDivertActiveBarred) profile.setDivertActiveBarred(divertActiveOn ? divertActiveBarred:false);
                if (isDivertActiveBlocked) profile.setDivertActiveBlocked(divertActiveOn ? divertActiveBlocked:false);
                if (isDivertActiveCapacity) profile.setDivertActiveCapacity(divertActiveOn ? divertActiveCapacity:false);
                if (isDivertActiveUnconditional) profile.setDivertActiveUnconditional(divertActiveOn ? divertActiveUnconditional:false);
                if (isDivertModifiable) profile.setDivertModifiable(divertModifiable);
                if (isUdhConcat) profile.setUdhConcat(udhConcat);
                if (isLocale) {
                   if (!ctx.getSmsc().isLocaleRegistered(locale))
                      throw new Exception("Locale '"+locale+"' is not registered");
                  profile.setLocale(locale);
                }

                switch (ctx.getSmsc().profileUpdate(profileMask, profile))
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

