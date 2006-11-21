/**
 * Created by IntelliJ IDEA.
 * User: makarov
 * Date: Dec 16, 2002
 * Time: 6:55:34 PM
 * To change this template use Options | File Templates.
 */
package ru.novosoft.smsc.admin.console.commands.profile;


import ru.novosoft.smsc.admin.console.CommandContext;
import ru.novosoft.smsc.admin.journal.Actions;
import ru.novosoft.smsc.admin.journal.SubjectTypes;
import ru.novosoft.smsc.admin.profiler.Profile;
import ru.novosoft.smsc.admin.profiler.SupportExtProfile;
import ru.novosoft.smsc.admin.route.Mask;

public class ProfileAlterCommand extends ProfileGenCommand {
    private String address;
    private Mask mask;
    private int updateResult;

    public void setAddress(String address) {
        this.address = address;
    }

    public void process(CommandContext ctx) {
        super.process(ctx);
        if (!isCodepage && !isReport && !isLocale && !isAliasHide && !isAliasModifiable &&
                isDivertOptions && !isDivert && !isDivertModifiable && !isDivertActiveAbsent &&
                !isDivertActiveBarred && !isDivertActiveBlocked && !isDivertActiveCapacity &&
                !isDivertActiveUnconditional && !isUdhConcat && !isTranslit && !isInputAccessMask &&
                !isOutputAccessMask && !isServices) {
            ctx.setMessage("expecting 'encoding', 'report', 'locale', 'alias', 'divert' option(s). " +
                    "Syntax: alter profile <profile_address> " +
                    "[report (full|none|final)] [locale <locale_name>] " +
                    "[encoding (default|ucs2|latin1|ucs2-latin1) [ussd7bit]] " +
                    "[alias [hide|nohide|substitute] [modifiable|notmodifiable]] " +
                    "[divert [(set <divert>)|clear] [(on|off) [absent][barred][blocked][capacity][unconditional]] " +
                    "[modifiable|notmodifiable]] [udhconcat on|off] [translit on|off] [group <string>] " +
                    "[inputaccessmask <number> | inputaccessbit <on|off> <bit number:0-31>] " +
                    "[outputaccessmask <number> | outputaccessbit <on|off> <bit number:0-31>] " +
                    (SupportExtProfile.enabled ? "[servicesmask <number> | servicesbit <on|off> <bit number:0-31>]" : ""));
            ctx.setStatus(CommandContext.CMD_PARSE_ERROR);
            return;
        }
        String out = "Profile for address '" + address + "'";
        try {
            mask = new Mask(address);
            Profile profile = ctx.getSmsc().profileLookup(mask);
            if (profile == null) {
                ctx.setMessage(out + " not found");
                ctx.setStatus(CommandContext.CMD_PROCESS_ERROR);
            } else {
                if (isCodepage) {
                    profile.setCodepage(codepage);
                    profile.setUssd7bit(ussd7bit);
                }
                if (isReport) profile.setReportOptions(report);
                if (isAliasHide) profile.setAliasHide(aliasHide);
                if (isAliasModifiable) profile.setAliasModifiable(aliasModifiable);
                if (isDivert) profile.setDivert(divert);
                if (isDivertActiveAbsent) profile.setDivertActiveAbsent(divertActiveOn && divertActiveAbsent);
                if (isDivertActiveBarred) profile.setDivertActiveBarred(divertActiveOn && divertActiveBarred);
                if (isDivertActiveBlocked) profile.setDivertActiveBlocked(divertActiveOn && divertActiveBlocked);
                if (isDivertActiveCapacity) profile.setDivertActiveCapacity(divertActiveOn && divertActiveCapacity);
                if (isDivertActiveUnconditional) profile.setDivertActiveUnconditional(divertActiveOn && divertActiveUnconditional);
                if (isDivertModifiable) profile.setDivertModifiable(divertModifiable);
                if (isUdhConcat) profile.setUdhConcat(udhConcat);
                if (isTranslit) profile.setTranslit(translit);
                if (isLocale) {
                    if (!ctx.getSmsc().isLocaleRegistered(locale))
                        throw new Exception("Locale '" + locale + "' is not registered");
                    profile.setLocale(locale);
                }
                if (isInputAccessMask) profile.setInputAccessMask(inputAccessMask);
                else if( inputAccessBitOff != 0 ) {
                  profile.setInputAccessMask(profile.getInputAccessMask()&(~inputAccessBitOff));
                } else if( inputAccessBitOn != 0 ) {
                  profile.setInputAccessMask(profile.getInputAccessMask()|inputAccessBitOn);
                }
                if (isOutputAccessMask) profile.setOutputAccessMask(outputAccessMask);
                else if( outputAccessBitOff != 0 ) {
                  profile.setOutputAccessMask(profile.getOutputAccessMask()&(~outputAccessBitOff));
                } else if( outputAccessBitOn != 0 ) {
                  profile.setOutputAccessMask(profile.getOutputAccessMask()|outputAccessBitOn);
                }
                if (isServices) profile.setServices(services);
                updateResult = ctx.getSmsc().profileUpdate(mask, profile);
                switch (updateResult) {
                    case 1:    //pusUpdated
                        ctx.setMessage(out + " altered");
                        ctx.setStatus(CommandContext.CMD_OK);
                        break;
                    case 2: //pusInserted
                        ctx.setMessage(out + " added new");
                        ctx.setStatus(CommandContext.CMD_OK);
                        break;
                    case 3: //pusUnchanged
                        ctx.setMessage(out + " unchanged.");
                        ctx.setStatus(CommandContext.CMD_OK);
                        break;
                    default: // pusError
                        ctx.setMessage("Couldn't alter " + out + ". Unknown cause");
                        ctx.setStatus(CommandContext.CMD_PROCESS_ERROR);
                        break;
                }
            }
        } catch (Exception e) {
            ctx.setMessage("Couldn't alter " + out + ". Cause: " + e.getMessage());
            ctx.setStatus(CommandContext.CMD_PROCESS_ERROR);
        }
    }

    public String getId() {
        return "PROFILE_ALTER";
    }

    public void updateJournalAndStatuses(CommandContext ctx, String userName) {
        byte act = 0;
        switch (updateResult) {
            case 1:
                act = Actions.ACTION_MODIFY;
                break;
            case 2:
                act = Actions.ACTION_ADD;
                break;
            default:
                return;
        }
        journalAppend(ctx, userName, SubjectTypes.TYPE_profile, mask.getMask(), act);
        ctx.getStatuses().setProfilesChanged(true);
    }
}

