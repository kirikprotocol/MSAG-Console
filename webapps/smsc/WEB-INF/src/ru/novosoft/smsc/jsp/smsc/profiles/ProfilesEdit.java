/*
 * Created by igork
 * Date: 04.11.2002
 * Time: 20:41:31
 */
package ru.novosoft.smsc.jsp.smsc.profiles;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.journal.Actions;
import ru.novosoft.smsc.admin.journal.SubjectTypes;
import ru.novosoft.smsc.admin.profiler.Profile;
import ru.novosoft.smsc.admin.route.Mask;
import ru.novosoft.smsc.jsp.SMSCErrors;

import javax.servlet.http.HttpServletRequest;
import java.util.List;

public class ProfilesEdit extends ProfilesBean
{
  protected String mbSave = null;
  protected String mbCancel = null;

  public ProfilesEdit()
  {
    report = -1;
    codepage = -1;
  }

  protected int init(final List errors)
  {
    final int result = super.init(errors);
    if (RESULT_OK != result)
      return result;

    if (null == mask)
      return error(SMSCErrors.error.profiles.profileNotSpecified);

    if (-1 == report && -1 == codepage) {
      try {
        final Profile p = smsc.profileLookup(new Mask(mask));
        report = p.getReportOptions();
        codepage = p.getCodepage();
        ussd7bit = p.isUssd7bit();
        locale = p.getLocale();
        aliasHide = p.isAliasHide();
        aliasModifiable = p.isAliasModifiable();
        divert = p.getDivert();
        divertActiveUnconditional = p.isDivertActiveUnconditional();
        divertActiveAbsent = p.isDivertActiveAbsent();
        divertActiveBlocked = p.isDivertActiveBlocked();
        divertActiveBarred = p.isDivertActiveBarred();
        divertActiveCapacity = p.isDivertActiveCapacity();
        divertModifiable = p.isDivertModifiable();
        udhConcat = p.isUdhConcat();
        translit = p.isTranslit();
      } catch (AdminException e) {
        logger.error("Couldn't lookup profile \"" + mask + '"', e);
        return error(SMSCErrors.error.profiles.couldntLookup, mask, e);
      }
    }

    return result;
  }

  public int process(final HttpServletRequest request)
  {
    if (null != mbCancel)
      return RESULT_DONE;

    final int result = super.process(request);
    if (RESULT_OK != result)
      return result;

    if (null != mbSave)
      return save();

    return RESULT_OK;
  }

  protected int save()
  {
    if (!Mask.isMaskValid(mask))
      return error(SMSCErrors.error.profiles.invalidMask, mask);

    try {
      final Mask address = new Mask(mask);
      final Profile profile = new Profile(address, codepage, ussd7bit, report, locale, aliasHide, aliasModifiable, divert, divertActiveUnconditional, divertActiveAbsent, divertActiveBlocked, divertActiveBarred, divertActiveCapacity, divertModifiable, udhConcat, translit);
      switch (smsc.profileUpdate(address, profile)) {
        case 1: //pusUpdated
          journalAppend(SubjectTypes.TYPE_profile, address.getMask(), Actions.ACTION_MODIFY);
          appContext.getStatuses().setProfilesChanged(true);
          return RESULT_DONE;
        case 2: //pusInserted
          journalAppend(SubjectTypes.TYPE_profile, address.getMask(), Actions.ACTION_ADD);
          appContext.getStatuses().setProfilesChanged(true);
          return RESULT_DONE;
        case 3: //pusUnchanged
          return RESULT_DONE;
        case 4: //pusError
        default:
          return error(SMSCErrors.error.unknown);
      }
    } catch (AdminException e) {
      logger.error("Couldn't update profile [\"" + mask + "\", " + codepage + ", " + report + "]", e);
      return error(SMSCErrors.error.profiles.couldntUpdate, e);
    }
  }

  /**
   * ************************ properties ********************************
   */

  public String getMbSave()
  {
    return mbSave;
  }

  public void setMbSave(final String mbSave)
  {
    this.mbSave = mbSave;
  }

  public String getMbCancel()
  {
    return mbCancel;
  }

  public void setMbCancel(final String mbCancel)
  {
    this.mbCancel = mbCancel;
  }
}
