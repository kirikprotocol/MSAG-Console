/**
 * Created by IntelliJ IDEA.
 * User: makarov
 * Date: Dec 16, 2002
 * Time: 6:58:28 PM
 * To change this template use Options | File Templates.
 */
package ru.novosoft.smsc.admin.console.commands.profile;


import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.alias.Alias;
import ru.novosoft.smsc.admin.alias.AliasSet;
import ru.novosoft.smsc.admin.console.CommandContext;
import ru.novosoft.smsc.admin.console.commands.CommandClass;
import ru.novosoft.smsc.admin.profiler.Profile;
import ru.novosoft.smsc.admin.route.Mask;

public class ProfileViewCommand extends CommandClass {
    private String address;

    public void setAddress(String address) {
        this.address = address;
    }

    private String showProfile(Profile profile, Alias alias, CommandContext ctx)
            throws AdminException {
        final byte aliasHide = profile.getAliasHide();
        final String aliasHideStr =
                ((aliasHide == Profile.ALIAS_HIDE_substitute) ? " substitute" :
                        ((aliasHide == Profile.ALIAS_HIDE_false) ? " nohide" :
                                ((aliasHide == Profile.ALIAS_HIDE_true) ? " hide" : "unknown")));
        String divert = profile.getDivert();
        Mask aliasAlias = (alias != null) ? alias.getAlias() : null;
        String result = "Profile '" + profile.getMask().getMask() + "'" +
                " Report: " + profile.getReportOptionsString() +
                " Locale: " + profile.getLocale() +
                " Encoding: " + profile.getCodepageString() +
                " ussd7bit: " + ((profile.isUssd7bit()) ? "on" : "off") +
                " translit: " + ((profile.isTranslit()) ? "on" : "off") +
                " Alias: " + ((aliasAlias != null) ? aliasAlias.getMask() : "-") + aliasHideStr +
                (profile.isAliasModifiable() ? " modifiable" : " notmodifiable") +
                " Divert: " + ((divert == null || divert.length() <= 0) ? "-" : divert);

        if (profile.isDivertActiveAbsent() || profile.isDivertActiveBarred() ||
                profile.isDivertActiveBlocked() || profile.isDivertActiveCapacity() ||
                profile.isDivertActiveUnconditional()) {
            result += " on: ";
            int count = 0;
            if (profile.isDivertActiveAbsent()) {
                count++;
                result += "absent";
            }
            if (profile.isDivertActiveBarred()) {
                if (count++ != 0) result += ",";
                result += "barred";
            }
            if (profile.isDivertActiveBlocked()) {
                if (count++ != 0) result += ",";
                result += "blocked";
            }
            if (profile.isDivertActiveCapacity()) {
                if (count++ != 0) result += ",";
                result += "capacity";
            }
            if (profile.isDivertActiveUnconditional()) {
                if (count != 0) result += ",";
                result += "unconditional";
            }
        } else
            result += " off";
        result += (profile.isDivertModifiable() ? " modifiable" : " notmodifiable");
        result += " udhconcat: " + ((profile.isUdhConcat()) ? "on" : "off");
        if (profile.getGroupId() != 0)
            result += " group: " + ctx.getClosedGroupManager().getClosedGroups().get(profile.getGroupId()).getName();
        else result += " group: none";
        result += " inputAccessMask: " + Long.toBinaryString(profile.getInputAccessMask());
        result += " outputAccessMask: " + Long.toBinaryString(profile.getOutputAccessMask());
        return result;
    }

    public void process(CommandContext ctx) {
        String out = "Profile for address '" + address + "'";
        try {
            Mask userAddress = new Mask(address);
            final AliasSet aliasSet = ctx.getSmsc().getAliases();
            Mask profileAlias = aliasSet.dealias(userAddress);
            Mask profileAddress = (profileAlias == null) ? userAddress : profileAlias;

            Profile profile = ctx.getSmsc().profileLookup(profileAddress);
            if (profile != null) {
                Alias alias = aliasSet.getAliasByAddress(profile.getMask());
                ctx.setMessage(showProfile(profile, alias, ctx));
                ctx.setStatus(CommandContext.CMD_OK);
            } else {
                ctx.setMessage(out + " not found");
                ctx.setStatus(CommandContext.CMD_PROCESS_ERROR);
            }
        } catch (Exception e) {
            ctx.setMessage("Couldn't view " + out + ". Cause: " + e.getMessage());
            ctx.setStatus(CommandContext.CMD_PROCESS_ERROR);
        }
    }

    public String getId() {
        return "PROFILE_VIEW";
    }
}

