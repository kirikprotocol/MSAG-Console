package ru.novosoft.smsc.jsp.util.tables.impl.profile;

/*
 * Created by igork
 * Date: 28.08.2002
 * Time: 17:59:20
 */

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.profiler.Profile;
import ru.novosoft.smsc.jsp.util.tables.impl.AbstractDataItem;
import ru.novosoft.smsc.jsp.util.tables.impl.QueryResultSetImpl;
import ru.novosoft.smsc.jsp.util.tables.DataItem;

import java.util.Comparator;


public class ProfileDataItem extends AbstractDataItem {
    public ProfileDataItem(final Profile profile) throws AdminException {
        values.put("Mask", profile.getMask().getMask());
        values.put("Codepage", profile.getCodepageString());
        values.put("Report info", profile.getReportOptionsString());
        values.put("locale", profile.getLocale());
        switch (profile.getAliasHide()) {
            case Profile.ALIAS_HIDE_false:
                values.put("alias hide", "nohide");
                break;
            case Profile.ALIAS_HIDE_true:
                values.put("alias hide", "hide");
                break;
            case Profile.ALIAS_HIDE_substitute:
                values.put("alias hide", "substitute");
                break;
            default:
                values.put("alias hide", "unknown");
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

    private static class DataItemComparator implements Comparator {
        private int sortType;
        private String name;

        public DataItemComparator(String name) {
            this.name = name;
        }

        public int compare(Object o1, Object o2) {
            DataItem c1 = (DataItem) o1;
            DataItem c2 = (DataItem) o2;
            return ((String) c1.getValue(name)).compareToIgnoreCase((String) c1.getValue(name));
        }
    }

    public static Comparator comparator(String name) {
        return new ProfileDataItem.DataItemComparator(name);
    }

}
