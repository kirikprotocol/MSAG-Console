package ru.novosoft.smsc.jsp.util.tables.impl.profile;

/*
 * Created by igork
 * Date: 28.08.2002
 * Time: 17:59:20
 */

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.profiler.Profile;
import ru.novosoft.smsc.jsp.util.tables.impl.AbstractDataItem;

public class ProfileDataItem extends AbstractDataItem {
    public ProfileDataItem(final Profile profile) throws AdminException {
        values.put("mask", profile.getMask().getMask());
        values.put("codeset", profile.getCodepageString());
        values.put("reportinfo", profile.getReportOptionsString());
        values.put("locale", profile.getLocale());
        switch (profile.getAliasHide()) {
            case Profile.ALIAS_HIDE_false:
                values.put("alias_hide", "nohide");
                break;
            case Profile.ALIAS_HIDE_true:
                values.put("alias_hide", "hide");
                break;
            case Profile.ALIAS_HIDE_substitute:
                values.put("alias_hide", "substitute");
                break;
            default:
                values.put("alias_hide", "unknown");
        }
        values.put("hidden_mod", new Boolean(profile.isAliasModifiable()));
        values.put("divert", profile.getDivert());
        values.put("divert_actAbs", new Boolean(profile.isDivertActiveAbsent()));
        values.put("divert_actBar", new Boolean(profile.isDivertActiveBarred()));
        values.put("divert_actBlo", new Boolean(profile.isDivertActiveBlocked()));
        values.put("divert_actCap", new Boolean(profile.isDivertActiveCapacity()));
        values.put("divert_actUnc", new Boolean(profile.isDivertActiveUnconditional()));
        values.put("divert_mod", new Boolean(profile.isDivertModifiable()));
        values.put("ussd7bit", new Boolean(profile.isUssd7bit()));
        values.put("translit", new Boolean(profile.isTranslit()));
    }

}
