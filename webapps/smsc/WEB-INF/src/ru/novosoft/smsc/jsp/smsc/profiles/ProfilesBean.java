package ru.novosoft.smsc.jsp.smsc.profiles;

import ru.novosoft.smsc.admin.profiler.Profile;
import ru.novosoft.smsc.jsp.SMSCErrors;
import ru.novosoft.smsc.jsp.smsc.SmscBean;

import java.util.LinkedList;
import java.util.List;


/**
 * Created by igork Date: 11.03.2003 Time: 18:38:02
 */
public class ProfilesBean extends SmscBean
{
  protected String mask = "";
  protected byte report = Profile.REPORT_OPTION_None;
  protected byte codepage = Profile.CODEPAGE_Default;
  protected boolean ussd7bit = false;
  protected String locale = "";
  protected List registeredLocales = new LinkedList();
  protected String returnPath = null;

  protected byte aliasHide = Profile.ALIAS_HIDE_false;
  protected boolean aliasModifiable = false;

  protected String divert = "";
  protected boolean divertActiveUnconditional;
  protected boolean divertActiveAbsent;
  protected boolean divertActiveBlocked;
  protected boolean divertActiveBarred;
  protected boolean divertActiveCapacity;
  protected boolean divertModifiable = false;
  protected boolean udhConcat = false;
  protected boolean translit = false;

  protected int init(final List errors)
  {
    final int result = super.init(errors);
    if (RESULT_OK != result)
      return result;

    try {
      registeredLocales = smsc.getRegisteredLocales();
    } catch (Throwable e) {
      logger.error("Couldn't get registered profiles", e);
      return error(SMSCErrors.error.profiles.couldntGetRegisteredLocales, e);
    }
    return result;
  }

  public String getMask()
  {
    return mask;
  }

  public void setMask(final String mask)
  {
    this.mask = mask;
  }

  public String getReport()
  {
    return Byte.toString(report);
  }

  public byte getByteReport()
  {
    return report;
  }

  public void setReport(final String report)
  {
    try {
      this.report = Byte.decode(report).byteValue();
    } catch (NumberFormatException e) {
      this.report = Profile.REPORT_OPTION_None;
    }
  }


  public String getCodepage()
  {
    return Byte.toString(codepage);
  }

  public byte getByteCodepage()
  {
    return codepage;
  }

  public void setCodepage(final String codepage)
  {
    try {
      this.codepage = Byte.decode(codepage).byteValue();
    } catch (NumberFormatException e) {
      this.codepage = Profile.CODEPAGE_Default;
    }
  }

  public String getLocale()
  {
    return locale;
  }

  public void setLocale(final String locale)
  {
    this.locale = locale;
  }

  public List getRegisteredLocales()
  {
    return registeredLocales;
  }

  public byte getAliasHideByte()
  {
    return aliasHide;
  }

  public String getAliasHide()
  {
    return Profile.getAliasHideString(aliasHide);
  }

  public void setAliasHide(final String aliasHide)
  {
    if ("true".equalsIgnoreCase(aliasHide) || "hide".equalsIgnoreCase(aliasHide))
      this.aliasHide = Profile.ALIAS_HIDE_true;
    else if ("false".equalsIgnoreCase(aliasHide))
      this.aliasHide = Profile.ALIAS_HIDE_false;
    else if ("substitute".equalsIgnoreCase(aliasHide))
      this.aliasHide = Profile.ALIAS_HIDE_substitute;
  }

  public boolean isAliasModifiable()
  {
    return aliasModifiable;
  }

  public void setAliasModifiable(final boolean aliasModifiable)
  {
    this.aliasModifiable = aliasModifiable;
  }

  public void setAliasModifiable(final String aliasModifiable)
  {
    this.aliasModifiable = "true".equalsIgnoreCase(aliasModifiable) || "modifiable".equalsIgnoreCase(aliasModifiable);
  }

  public String getReturnPath()
  {
    return returnPath;
  }

  public void setReturnPath(final String returnPath)
  {
    this.returnPath = returnPath;
  }

  public String getDivert()
  {
    return divert;
  }

  public void setDivert(final String divert)
  {
    this.divert = divert;
  }

  public boolean isDivertModifiable()
  {
    return divertModifiable;
  }

  public void setDivertModifiable(final boolean divertModifiable)
  {
    this.divertModifiable = divertModifiable;
  }

  public boolean isUssd7bit()
  {
    return ussd7bit;
  }

  public void setUssd7bit(final boolean ussd7bit)
  {
    this.ussd7bit = ussd7bit;
  }

  public boolean isDivertActiveUnconditional()
  {
    return divertActiveUnconditional;
  }

  public void setDivertActiveUnconditional(final boolean divertActiveUnconditional)
  {
    this.divertActiveUnconditional = divertActiveUnconditional;
  }

  public boolean isDivertActiveAbsent()
  {
    return divertActiveAbsent;
  }

  public void setDivertActiveAbsent(final boolean divertActiveAbsent)
  {
    this.divertActiveAbsent = divertActiveAbsent;
  }

  public boolean isDivertActiveBlocked()
  {
    return divertActiveBlocked;
  }

  public void setDivertActiveBlocked(final boolean divertActiveBlocked)
  {
    this.divertActiveBlocked = divertActiveBlocked;
  }

  public boolean isDivertActiveBarred()
  {
    return divertActiveBarred;
  }

  public void setDivertActiveBarred(final boolean divertActiveBarred)
  {
    this.divertActiveBarred = divertActiveBarred;
  }

  public boolean isDivertActiveCapacity()
  {
    return divertActiveCapacity;
  }

  public void setDivertActiveCapacity(final boolean divertActiveCapacity)
  {
    this.divertActiveCapacity = divertActiveCapacity;
  }

  public boolean isUdhConcat()
  {
    return udhConcat;
  }

  public void setUdhConcat(final boolean udhConcat)
  {
    this.udhConcat = udhConcat;
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
