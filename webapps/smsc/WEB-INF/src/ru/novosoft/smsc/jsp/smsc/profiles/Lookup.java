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
  private boolean aliasHide = false;
  private boolean aliasModifiable = false;
  private byte matchType = ProfileEx.MATCH_UNKNOWN;
  private Mask matchAddress = null;
  private String divert = "";
  private boolean divertActive = false;
  private boolean divertModifiable = false;

  private String profileDealiased = null;
  private String profileAliased = null;

  private String mbAdd = null;
  private String mbEdit = null;
  private String mbDelete = null;
  private String mbEditMask = null;
  private String mbRefreshed = null;
  private String mbDeleteMask = null;


  protected int init(List errors)
  {
    int result = super.init(errors);
    if (result != RESULT_OK)
      return result;

    try {
      if (appContext.getHostsManager().getServiceInfo(Constants.SMSC_SME_ID).getStatus() != ServiceInfo.STATUS_RUNNING) {
        return warning(SMSCErrors.warning.SMSCNotRunning);
      }
    } catch (Exception e) {
      return error(SMSCErrors.error.CouldntGetSMSCStatus, e);
    }

    if (profile != null) {
      try {
        final Mask profileMask = new Mask(profile);
        final AliasSet aliases = appContext.getSmsc().getAliases();
        Alias alias = aliases.getAddressByAlias(profileMask);
        if (alias != null)
          profileDealiased = alias.getAddress().getMask();
        ProfileEx p = appContext.getSmsc().profileLookupEx(profileDealiased != null ? new Mask(profileDealiased) : profileMask);
        if (p.getMatchType() == ProfileEx.MATCH_EXACT) {
          Alias aliased = aliases.getAliasByAddress(p.getMatchAddress());
          if (aliased != null)
            profileAliased = aliased.getAlias().getMask();
        }
        codepage = p.getCodepageString();
        ussd7bit = p.isUssd7bit();
        reportOptions = p.getReportOptionsString();
        locale = p.getLocale();
        aliasHide = p.isAliasHide();
        aliasModifiable = p.isAliasModifiable();
        divert = p.getDivert();
        divertActive = p.isDivertActive();
        divertModifiable = p.isDivertModifiable();
        matchType = p.getMatchType();
        matchAddress = p.getMatchAddress();
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

  public int process(HttpServletRequest request)
  {
    int result = super.process(request);
    if (result != RESULT_OK)
      return result;

    if (mbAdd != null)
      return RESULT_ADD;
    if (mbEdit != null)
      return RESULT_EDIT;
    if (mbEditMask != null)
      return RESULT_EDIT_MASK;
    if (mbDelete != null) {
      try {
        return delete(new Mask(profile));
      } catch (AdminException e) {
        logger.debug("Couldn't delete profile \"" + profile + "\"", e);
        return error(SMSCErrors.error.profiles.couldntDelete, profile, e);
      }
    }
    if (mbDeleteMask != null)
      return delete(matchAddress);

    if (mbRefreshed != null)
      return RESULT_OK;
    else
      return RESULT_REFRESH;
  }

  private int delete(Mask profileToDelete)
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

  public void setProfile(String profile)
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

  public boolean isAliasHide()
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

  public void setMbAdd(String mbAdd)
  {
    this.mbAdd = mbAdd;
  }

  public String getMbEdit()
  {
    return mbEdit;
  }

  public void setMbEdit(String mbEdit)
  {
    this.mbEdit = mbEdit;
  }

  public String getMbDelete()
  {
    return mbDelete;
  }

  public void setMbDelete(String mbDelete)
  {
    this.mbDelete = mbDelete;
  }

  public String getMbEditMask()
  {
    return mbEditMask;
  }

  public void setMbEditMask(String mbEditMask)
  {
    this.mbEditMask = mbEditMask;
  }

  public String getMbRefreshed()
  {
    return mbRefreshed;
  }

  public void setMbRefreshed(String mbRefreshed)
  {
    this.mbRefreshed = mbRefreshed;
  }

  public String getMbDeleteMask()
  {
    return mbDeleteMask;
  }

  public void setMbDeleteMask(String mbDeleteMask)
  {
    this.mbDeleteMask = mbDeleteMask;
  }

  public String getDivert()
  {
    return divert;
  }

  public boolean isDivertActive()
  {
    return divertActive;
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
}
