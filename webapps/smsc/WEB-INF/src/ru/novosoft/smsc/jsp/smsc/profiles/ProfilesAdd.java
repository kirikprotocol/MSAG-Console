package ru.novosoft.smsc.jsp.smsc.profiles;

/*
 * Created by igork
 * Date: 04.11.2002
 * Time: 20:02:01
 */

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.journal.Actions;
import ru.novosoft.smsc.admin.journal.SubjectTypes;
import ru.novosoft.smsc.admin.profiler.Profile;
import ru.novosoft.smsc.admin.route.Mask;
import ru.novosoft.smsc.jsp.SMSCErrors;

import javax.servlet.http.HttpServletRequest;

public class ProfilesAdd extends ProfilesBean {
    protected String mbSave = null;
    protected String mbCancel = null;

    public ProfilesAdd() {
        report = Profile.REPORT_OPTION_None;
        codepage = Profile.CODEPAGE_Default;
    }

    public int process(HttpServletRequest request) {
        if (mbCancel != null)
            return RESULT_DONE;

        int result = super.process(request);
        if (result != RESULT_OK)
            return result;

        if (mbSave != null)
            return save(request);

        return RESULT_OK;
    }

    protected int save(final HttpServletRequest request) {
        logger.debug("Add new profile: " + mask);
        if (!Mask.isMaskValid(mask))
            return error(SMSCErrors.error.profiles.invalidMask, mask);
        if (!appContext.getSmsc().isLocaleRegistered(locale))
            return error(SMSCErrors.error.profiles.invalidLocale, locale);

        try {
            final Mask address = new Mask(mask);
            Profile profile = new Profile(address, codepage, ussd7bit, report, locale, aliasHide, aliasModifiable, divert, divertActiveUnconditional, divertActiveAbsent, divertActiveBlocked, divertActiveBarred, divertActiveCapacity, divertModifiable, udhConcat, translit, groupId, inputAccessMask, outputAccessMask);
            switch (smsc.profileUpdate(address, profile)) {
                case 1: //pusUpdated
                    journalAppend(SubjectTypes.TYPE_profile, address.getMask(), Actions.ACTION_MODIFY);
                    appContext.getStatuses().setProfilesChanged(true);
                    request.getSession().setAttribute("PROFILE_ADD_MASK", address.getMask());
                    return RESULT_DONE;
                case 2: //pusInserted
                    journalAppend(SubjectTypes.TYPE_profile, address.getMask(), Actions.ACTION_ADD);
                    appContext.getStatuses().setProfilesChanged(true);
                    request.getSession().setAttribute("PROFILE_ADD_MASK", address.getMask());
                    return RESULT_DONE;
                case 3: //pusUnchanged
                    return error(SMSCErrors.error.profiles.identicalToDefault);
                case 4: //pusError
                    return error(SMSCErrors.error.unknown);
                default:
                    return error(SMSCErrors.error.unknown);
            }
        } catch (AdminException e) {
            logger.error("Couldn't add profile [\"" + mask + "\", " + codepage + ", " + report + ", " + locale + "]", e);
            return error(SMSCErrors.error.profiles.couldntAdd, e);
        }
    }

    /**
     * ************************ properties ********************************
     */

    public String getMbSave() {
        return mbSave;
    }

    public void setMbSave(String mbSave) {
        this.mbSave = mbSave;
    }

    public String getMbCancel() {
        return mbCancel;
    }

    public void setMbCancel(String mbCancel) {
        this.mbCancel = mbCancel;
    }

}
