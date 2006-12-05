package ru.novosoft.smsc.jsp.smsc.profiles;

import ru.novosoft.smsc.admin.closedgroups.ClosedGroupList;
import ru.novosoft.smsc.admin.profiler.Profile;
import ru.novosoft.smsc.admin.profiler.SupportExtProfile;
import ru.novosoft.smsc.jsp.SMSCErrors;
import ru.novosoft.smsc.jsp.smsc.SmscBean;
import ru.novosoft.smsc.util.config.Config;

import java.util.*;


/**
 * Created by igork Date: 11.03.2003 Time: 18:38:02
 */
public class ProfilesBean extends SmscBean {
    protected String mask = "";
    protected byte report = Profile.REPORT_OPTION_None;
    protected byte codepage = Profile.CODEPAGE_Default;
    protected boolean ussd7bit = false;
    protected String locale = "";
    protected List registeredLocales = new LinkedList();
    protected ClosedGroupList closedGroups = new ClosedGroupList();
    protected Map smsExtraServices = null;
    protected String returnPath = null;

    protected byte aliasHide = Profile.ALIAS_HIDE_true;
    protected boolean aliasModifiable = false;

    protected String divert = "";
    protected boolean divertActiveUnconditional = false;
    protected boolean divertActiveAbsent = false;
    protected boolean divertActiveBlocked = false;
    protected boolean divertActiveBarred = false;
    protected boolean divertActiveCapacity = false;
    protected boolean divertModifiable = false;

    protected boolean udhConcat = false;
    protected boolean translit = false;

    protected int groupId = 0;
    protected long inputAccessMask = 1;
    protected long outputAccessMask = 1;
    protected long services = 0;
    protected short sponsored = 0;

    protected String[] checkedServicesNames = null;

    protected String groupName = "";

    protected int init(final List errors) {
        final int result = super.init(errors);
        if (RESULT_OK != result)
            return result;

        try {
            registeredLocales = smsc.getRegisteredLocales();
            //todo profiler-default properties
            if (mask.equals("")) {
                codepage = Profile.convertCodepageStringToByte(smsc.getDefaultProfilePropString("DataCoding"));
                ussd7bit = smsc.getDefaultProfilePropBoolean("UssdIn7Bit");
                report = Profile.convertReportOptionsStringToByte(smsc.getDefaultProfilePropString("Report"));
                aliasHide = (smsc.getDefaultProfilePropBoolean("Hide") ? Profile.ALIAS_HIDE_true : Profile.ALIAS_HIDE_false);
                aliasModifiable = smsc.getDefaultProfilePropBoolean("HideModifiable");
                divertModifiable = smsc.getDefaultProfilePropBoolean("DivertModifiable");
                udhConcat = smsc.getDefaultProfilePropBoolean("UdhConcat");
                locale = smsc.getDefaultProfilePropString("Locale");
            }

//            if (!isTranslit) translit = ctx.getSmsc().getDefaultProfilePropBoolean("Translit");
//            if (!isInputAccessMask) inputAccessMask = ctx.getSmsc().getDefaultProfilePropInt("InputAccessMask");
//            if (!isOutputAccessMask) outputAccessMask = ctx.getSmsc().getDefaultProfilePropInt("OutputAccessMask");
//            if (!isDivert) divert = ctx.getSmsc().getDefaultProfilePropString("Divert");
//            if (isDivertActiveAbsent) profile.setDivertActiveAbsent(divertActiveOn && divertActiveAbsent);
//            if (isDivertActiveBarred) profile.setDivertActiveBarred(divertActiveOn && divertActiveBarred);
//            if (isDivertActiveBlocked) profile.setDivertActiveBlocked(divertActiveOn && divertActiveBlocked);
//            if (isDivertActiveCapacity) profile.setDivertActiveCapacity(divertActiveOn && divertActiveCapacity);
//            if (isDivertActiveUnconditional) profile.setDivertActiveUnconditional(divertActiveOn && divertActiveUnconditional);
        } catch (Throwable e) {
            logger.error("Couldn't get registered profiles", e);
            return error(SMSCErrors.error.profiles.couldntGetRegisteredLocales, e);
        }
        try {
            closedGroups = appContext.getClosedGroupManager().getClosedGroups();
        } catch (Throwable e) {
            logger.error("Couldn't get closed group list", e);
            return error(SMSCErrors.error.profiles.couldntGetRegisteredLocales, e);
        }
        try {
            if (SupportExtProfile.enabled) {
                smsExtraServices = new HashMap();
                Config webconfig = appContext.getConfig();
                Set set = webconfig.getSectionChildParamsNames("profiler.extra");
                for (Iterator i = set.iterator(); i.hasNext();) { String name = (String) i.next();
                    Object param = webconfig.getParameter(name);
                    smsExtraServices.put(name.substring("profiler.extra.".length()), param);  // вырезали profiler.extra.
                }
            }
        } catch (Throwable e) {
            logger.error("Couldn't get extra services names from webconfig", e);
            return error(SMSCErrors.error.profiles.couldntGetSmsExtraServices, e);
        }
        return result;
    }

    public String getMask() {
        return mask;
    }

    public void setMask(final String mask) {
        this.mask = mask;
    }

    public String getReport() {
        return Byte.toString(report);
    }

    public byte getByteReport() {
        return report;
    }

    public void setReport(final String report) {
        try {
            this.report = Byte.parseByte(report);
        } catch (NumberFormatException e) {
            this.report = Profile.REPORT_OPTION_None;
        }
    }

    public String getCodepage() {
        return Byte.toString(codepage);
    }

    public byte getByteCodepage() {
        return codepage;
    }

    public void setCodepage(final String codepage) {
        try {
            this.codepage = Byte.decode(codepage).byteValue();
        } catch (NumberFormatException e) {
            this.codepage = Profile.CODEPAGE_Default;
        }
    }

    public String getLocale() {
        return locale;
    }

    public void setLocale(final String locale) {
        this.locale = locale;
    }

    public List getRegisteredLocales() {
        return registeredLocales;
    }

    public byte getAliasHideByte() {
        return aliasHide;
    }

    public String getAliasHide() {
        return Profile.getAliasHideString(aliasHide);
    }

    public void setAliasHide(final String aliasHide) {
        if ("true".equalsIgnoreCase(aliasHide) || "hide".equalsIgnoreCase(aliasHide))
            this.aliasHide = Profile.ALIAS_HIDE_true;
        else if ("false".equalsIgnoreCase(aliasHide))
            this.aliasHide = Profile.ALIAS_HIDE_false;
        else if ("substitute".equalsIgnoreCase(aliasHide))
            this.aliasHide = Profile.ALIAS_HIDE_substitute;
    }

    public boolean isAliasModifiable() {
        return aliasModifiable;
    }

    public void setAliasModifiable(final boolean aliasModifiable) {
        this.aliasModifiable = aliasModifiable;
    }

    public void setAliasModifiable(final String aliasModifiable) {
        this.aliasModifiable = "true".equalsIgnoreCase(aliasModifiable) || "modifiable".equalsIgnoreCase(aliasModifiable);
    }

    public String getReturnPath() {
        return returnPath;
    }

    public void setReturnPath(final String returnPath) {
        this.returnPath = returnPath;
    }

    public String getDivert() {
        return divert;
    }

    public void setDivert(final String divert) {
        this.divert = divert;
    }

    public boolean isDivertModifiable() {
        return divertModifiable;
    }

    public void setDivertModifiable(final boolean divertModifiable) {
        this.divertModifiable = divertModifiable;
    }

    public boolean isUssd7bit() {
        return ussd7bit;
    }

    public void setUssd7bit(final boolean ussd7bit) {
        this.ussd7bit = ussd7bit;
    }

    public boolean isDivertActiveUnconditional() {
        return divertActiveUnconditional;
    }

    public void setDivertActiveUnconditional(final boolean divertActiveUnconditional) {
        this.divertActiveUnconditional = divertActiveUnconditional;
    }

    public boolean isDivertActiveAbsent() {
        return divertActiveAbsent;
    }

    public void setDivertActiveAbsent(final boolean divertActiveAbsent) {
        this.divertActiveAbsent = divertActiveAbsent;
    }

    public boolean isDivertActiveBlocked() {
        return divertActiveBlocked;
    }

    public void setDivertActiveBlocked(final boolean divertActiveBlocked) {
        this.divertActiveBlocked = divertActiveBlocked;
    }

    public boolean isDivertActiveBarred() {
        return divertActiveBarred;
    }

    public void setDivertActiveBarred(final boolean divertActiveBarred) {
        this.divertActiveBarred = divertActiveBarred;
    }

    public boolean isDivertActiveCapacity() {
        return divertActiveCapacity;
    }

    public void setDivertActiveCapacity(final boolean divertActiveCapacity) {
        this.divertActiveCapacity = divertActiveCapacity;
    }

    public boolean isUdhConcat() {
        return udhConcat;
    }

    public void setUdhConcat(final boolean udhConcat) {
        this.udhConcat = udhConcat;
    }

    public boolean isTranslit() {
        return translit;
    }

    public void setTranslit(final boolean translit) {
        this.translit = translit;
    }

    public String getInputAccessMask() {
        return Long.toHexString(inputAccessMask);
    }

    public void setInputAccessMask(String inputAccessMask) {
        try {
            this.inputAccessMask = Long.parseLong(inputAccessMask, 16);
        } catch (NumberFormatException e) {
            this.inputAccessMask = 1;
        }
    }

    public String getOutputAccessMask() {
        return Long.toHexString(outputAccessMask);
    }

    public void setOutputAccessMask(String outputAccessMask) {
        try {
            this.outputAccessMask = Long.parseLong(outputAccessMask, 16);
        } catch (NumberFormatException e) {
            this.outputAccessMask = 1;
        }
    }

    public String getGroupId() {
        return Integer.toString(groupId);
    }

    public void setGroupId(String groupId) {
        try {
            this.groupId = Integer.parseInt(groupId);
        } catch (NumberFormatException e) {
            this.groupId = 0;
        }
    }

    public ClosedGroupList getClosedGroups() {
        return closedGroups;
    }

    public Map getSmsExtraServices() {
        return smsExtraServices;
    }

    public boolean isServiceChecked(String bitNum) {
        long bitNumLong = Long.parseLong(bitNum) - 1;
        return (services & (1L << bitNumLong)) == (1L << bitNumLong);
    }

    public void setCheckedServicesNames(String[] checkedServicesNames) {
        this.checkedServicesNames = checkedServicesNames;
        services = 0;
        for (int i = 0; i < checkedServicesNames.length; i++) {
            int bitNum = Integer.parseInt(checkedServicesNames[i]);
            services =  services | (1 << bitNum);
        }
    }

    public String getSponsored() {
      return String.valueOf(sponsored);
    }

    public void setSponsored(byte sponsored) {
      this.sponsored = sponsored;
    }
}
