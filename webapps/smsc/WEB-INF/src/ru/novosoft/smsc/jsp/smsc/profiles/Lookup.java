/*
 * Created by igork
 * Date: 04.11.2002
 * Time: 18:49:34
 */
package ru.novosoft.smsc.jsp.smsc.profiles;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.Constants;
import ru.novosoft.smsc.admin.alias.Alias;
import ru.novosoft.smsc.admin.alias.AliasSet;
import ru.novosoft.smsc.admin.profiler.ProfileEx;
import ru.novosoft.smsc.admin.profiler.Profile;
import ru.novosoft.smsc.admin.route.Mask;
import ru.novosoft.smsc.admin.service.ServiceInfo;
import ru.novosoft.smsc.jsp.PageBean;
import ru.novosoft.smsc.jsp.SMSCErrors;

import javax.servlet.http.HttpServletRequest;
import java.util.List;

public class Lookup extends PageBean
{
  public static final int RESULT_ADD = PageBean.PRIVATE_RESULT;
  public static final int RESULT_EDIT = PageBean.PRIVATE_RESULT + 1;
  public static final int RESULT_REFRESH = PageBean.PRIVATE_RESULT + 2;
  public static final int RESULT_EDIT_MASK = PageBean.PRIVATE_RESULT + 3;
  protected static final int PRIVATE_RESULT = PageBean.PRIVATE_RESULT + 4;

  private String profile = null;
  private String codepage = null;
  private boolean ussd7bit = false;
  private String reportOptions = null;
  private String locale = null;
  private byte aliasHide = Profile.ALIAS_HIDE_false;
  private boolean aliasModifiable = false;
  private byte matchType = ProfileEx.MATCH_UNKNOWN;
  private Mask matchAddress = null;
  private String divert = "";
  private boolean divertActiveUnconditional = false;
  private boolean divertActiveAbsent = false;
  private boolean divertActiveBlocked = false;
  private boolean divertActiveBarred = false;
  private boolean divertActiveCapacity = false;
  private boolean divertModifiable = false;

  private String profileDealiased = null;
  private String profileAliased = null;
  private boolean udhConcat = false;
  private boolean translit = false;

  private String mbAdd = null;
  private String mbEdit = null;
  private String mbDelete = null;
  private String mbEditMask = null;
  private String mbRefreshed = null;
  private String mbDeleteMask = null;


  protected int init(final List errors)
  {
    final int result = super.init(errors);
    if (RESULT_OK != result)
      return result;

    try {
      if (ServiceInfo.STATUS_RUNNING != appContext.getHostsManager().getServiceInfo(Constants.SMSC_SME_ID).getStatus()) {
        return warning(SMSCErrors.warning.SMSCNotRunning);
      }
    } catch (Exception e) {
      return error(SMSCErrors.error.CouldntGetSMSCStatus, e);
    }

    if (null != profile) {
      try {
        final Mask profileMask = new Mask(profile);
        final AliasSet aliases = appContext.getSmsc().getAliases();
        final Mask alias = aliases.dealias(profileMask);
        if (null != alias)
          profileDealiased = alias.getMask();
        final ProfileEx p = appContext.getSmsc().profileLookupEx(null != profileDealiased ? new Mask(profileDealiased) : profileMask);
        if (ProfileEx.MATCH_EXACT == p.getMatchType()) {
          final Alias aliased = aliases.getAliasByAddress(p.getMatchAddress());
          if (null != aliased)
            profileAliased = aliased.getAlias().getMask();
        }
        codepage = p.getCodepageString();
        ussd7bit = p.isUssd7bit();
        reportOptions = p.getReportOptionsString();
        locale = p.getLocale();
        aliasHide = p.getAliasHide();
        aliasModifiable = p.isAliasModifiable();
        divert = p.getDivert();
        divertActiveUnconditional = p.isDivertActiveUnconditional();
        divertActiveAbsent = p.isDivertActiveAbsent();
        divertActiveBlocked = p.isDivertActiveBlocked();
        divertActiveBarred = p.isDivertActiveBarred();
        divertActiveCapacity = p.isDivertActiveCapacity();
        divertModifiable = p.isDivertModifiable();
        matchType = p.getMatchType();
        matchAddress = p.getMatchAddress();
        udhConcat = p.isUdhConcat();
        translit = p.isTranslit();
      } catch (Exception e) {
        logger.debug("Couldn't lookup profile \"" + profile + "\", nested: " + e.getMessage(), e);
        reportOptions = codepage = "unknown";
        return error(SMSCErrors.error.profiles.couldntLookup, profile, e);
      }
    } else {
      reportOptions = codepage = "unknown";
    }
    return RESULT_OK;
  }

  public int process(final HttpServletRequest request)
  {
    final int result = super.process(request);
    if (RESULT_OK != result)
      return result;

    if (null != mbAdd)
      return RESULT_ADD;
    if (null != mbEdit)
      return RESULT_EDIT;
    if (null != mbEditMask)
      return RESULT_EDIT_MASK;
    if (null != mbDelete) {
      try {
        return delete(new Mask(profile));
      } catch (AdminException e) {
        logger.debug("Couldn't delete profile \"" + profile + "\"", e);
        return error(SMSCErrors.error.profiles.couldntDelete, profile, e);
      }
    }
    if (null != mbDeleteMask)
      return delete(matchAddress);

    if (null != mbRefreshed)
      return RESULT_OK;
    else
      return RESULT_REFRESH;
  }

  private int delete(final Mask profileToDelete)
  {
    try {
      appContext.getSmsc().profileDelete(profileToDelete);
    } catch (AdminException e) {
      logger.debug("Couldn't delete profile \"" + profile + "\"", e);
      return error(SMSCErrors.error.profiles.couldntDelete, profile, e);
    }
    return RESULT_REFRESH;
  }

  public String getProfile()
  {
    return profile;
  }

  public void setProfile(final String profile)
  {
    this.profile = profile;
  }

  public String getCodepage()
  {
    return codepage;
  }

  public boolean isUssd7bit()
  {
    return ussd7bit;
  }

  public String getReportOptions()
  {
    return reportOptions;
  }

  public String getLocale()
  {
    return locale;
  }

  public byte getAliasHide()
  {
    return aliasHide;
  }

  public boolean isAliasModifiable()
  {
    return aliasModifiable;
  }

  public byte getMatchType()
  {
    return matchType;
  }

  public Mask getMatchAddress()
  {
    return matchAddress;
  }

  public String getMbAdd()
  {
    return mbAdd;
  }

  public void setMbAdd(final String mbAdd)
  {
    this.mbAdd = mbAdd;
  }

  public String getMbEdit()
  {
    return mbEdit;
  }

  public void setMbEdit(final String mbEdit)
  {
    this.mbEdit = mbEdit;
  }

  public String getMbDelete()
  {
    return mbDelete;
  }

  public void setMbDelete(final String mbDelete)
  {
    this.mbDelete = mbDelete;
  }

  public String getMbEditMask()
  {
    return mbEditMask;
  }

  public void setMbEditMask(final String mbEditMask)
  {
    this.mbEditMask = mbEditMask;
  }

  public String getMbRefreshed()
  {
    return mbRefreshed;
  }

  public void setMbRefreshed(final String mbRefreshed)
  {
    this.mbRefreshed = mbRefreshed;
  }

  public String getMbDeleteMask()
  {
    return mbDeleteMask;
  }

  public void setMbDeleteMask(final String mbDeleteMask)
  {
    this.mbDeleteMask = mbDeleteMask;
  }

  public String getDivert()
  {
    return divert;
  }

  public boolean isDivertModifiable()
  {
    return divertModifiable;
  }

  public String getProfileDealiased()
  {
    return profileDealiased;
  }

  public String getProfileAliased()
  {
    return profileAliased;
  }

  public boolean isDivertActiveUnconditional()
  {
    return divertActiveUnconditional;
  }

  public boolean isDivertActiveAbsent()
  {
    return divertActiveAbsent;
  }

  public boolean isDivertActiveBlocked()
  {
    return divertActiveBlocked;
  }

  public boolean isDivertActiveBarred()
  {
    return divertActiveBarred;
  }

  public boolean isDivertActiveCapacity()
  {
    return divertActiveCapacity;
  }

  public boolean isUdhConcat()
  {
    return udhConcat;
  }

  public boolean isTranslit()
  {
    return translit;
  }

  public void setTranslit(final boolean translit)
  {
    this.translit = translit;
  }
}
