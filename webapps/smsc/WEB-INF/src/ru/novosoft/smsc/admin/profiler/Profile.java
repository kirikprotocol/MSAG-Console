/*
 * User: igork
 * Date: 20.08.2002
 * Time: 13:30:24
 */
package ru.novosoft.smsc.admin.profiler;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.route.Mask;

import java.util.List;


public class Profile
{
  public static final byte CODEPAGE_Default = 0;
  public static final byte CODEPAGE_Latin1 = 3;
  public static final byte CODEPAGE_UCS2 = 8;
  public static final byte CODEPAGE_UCS2AndLatin1 = 11;

  public static final byte REPORT_OPTION_None = 0;
  public static final byte REPORT_OPTION_Full = 1;
  public static final byte REPORT_OPTION_Final = 3;

  public static final byte ALIAS_HIDE_false = 0;
  public static final byte ALIAS_HIDE_true = 1;
  public static final byte ALIAS_HIDE_substitute = 2;

  private Mask mask;
  private boolean ussd7bit;
  private String divert;

  // unconditional, absent, blocked, barred, capacity
  private boolean divertActiveUnconditional;
  private boolean divertActiveAbsent;
  private boolean divertActiveBlocked;
  private boolean divertActiveBarred;
  private boolean divertActiveCapacity;
  private boolean divertModifiable;

  private byte codepage;
  private byte reportOptions;
  private String locale;

  private byte aliasHide = ALIAS_HIDE_false;
  private boolean aliasModifiable = true;
  private boolean udhConcat;
  private boolean translit;

  public Profile(final Mask mask, final byte codepage, final boolean ussd7bit, final byte reportOptions, final String locale, final byte aliasHide,
                 final boolean aliasModifiable, final String divert, final boolean divertActiveUnconditional, final boolean divertActiveAbsent,
                 final boolean divertActiveBlocked, final boolean divertActiveBarred, final boolean divertActiveCapacity, final boolean divertModifiable,
                 final boolean udhConcat, final boolean translit)
  {
    this.mask = mask;
    this.ussd7bit = ussd7bit;
    this.divert = divert;
    this.divertActiveUnconditional = divertActiveUnconditional;
    this.divertActiveAbsent = divertActiveAbsent;
    this.divertActiveBlocked = divertActiveBlocked;
    this.divertActiveBarred = divertActiveBarred;
    this.divertActiveCapacity = divertActiveCapacity;
    this.divertModifiable = divertModifiable;
    this.codepage = codepage;
    this.reportOptions = reportOptions;
    this.locale = locale;
    this.aliasHide = aliasHide;
    this.aliasModifiable = aliasModifiable;
    this.udhConcat = udhConcat;
    this.translit = translit;
  }

  public Profile(final Mask mask, final byte codepage, final boolean ussd7bit, final byte reportOptions, final String locale, final byte aliasHide,
                 final boolean aliasModifiable, final String divert, final String divertActive, final boolean divertModifiable, final boolean udhConcat,
                 final boolean translit)
  {
    this.mask = mask;
    this.ussd7bit = ussd7bit;
    this.divert = divert;
    this.translit = translit;
    setDivertActive(divertActive);
    this.divertModifiable = divertModifiable;
    this.codepage = codepage;
    this.reportOptions = reportOptions;
    this.locale = locale;
    this.aliasHide = aliasHide;
    this.aliasModifiable = aliasModifiable;
    this.udhConcat = udhConcat;
  }

  public Profile(final Mask mask, final String codepage, final String ussd7bit, final String reportOptions, final String locale, final String aliasHide,
                 final String aliasModifiable, final String divert, final String divert_act, final String divert_mod, final String udhConcat,
                 final String translit) throws AdminException
  {
    this.mask = mask;
    setCodepage(codepage);
    setUssd7bit(ussd7bit);
    setReportOptions(reportOptions);
    setLocale(locale);
    setAliasHide(aliasHide);
    setAliasModifiable(aliasModifiable);
    setDivert(divert);
    setDivertActive(divert_act);
    setDivertModifiable(divert_mod);
    setUdhConcat(udhConcat);
    setTranslit(translit);
  }

  public Profile(final Mask mask, final List profileProperties) throws AdminException
  {
    this(mask, (String) profileProperties.get(0),
         (String) profileProperties.get(8),
         (String) profileProperties.get(1),
         (String) profileProperties.get(2),
         (String) profileProperties.get(3),
         (String) profileProperties.get(4),
         (String) profileProperties.get(5),
         (String) profileProperties.get(6),
         (String) profileProperties.get(7),
         (String) profileProperties.get(9),
         (String) profileProperties.get(10));
  }

  public byte getCodepage()
  {
    return codepage;
  }

  public String getCodepageString() throws AdminException
  {
    return getCodepageString(codepage);
  }

  public static String getCodepageString(final byte codepage) throws AdminException
  {
    switch (codepage) {
      case CODEPAGE_Default:
        return "default";
      case CODEPAGE_Latin1:
        return "Latin1";
      case CODEPAGE_UCS2:
        return "UCS2";
      case CODEPAGE_UCS2AndLatin1:
        return "UCS2&Latin1";
      default:
        throw new AdminException("Unknown codepage: " + codepage);
    }
  }

  public void setCodepage(final byte codepage)
  {
    this.codepage = codepage;
  }

  private void setCodepage(final String codepageString) throws AdminException
  {
    if ("default".equalsIgnoreCase(codepageString))
      codepage = CODEPAGE_Default;
    else if ("UCS2".equalsIgnoreCase(codepageString))
      codepage = CODEPAGE_UCS2;
    else if ("Latin1".equalsIgnoreCase(codepageString))
      codepage = CODEPAGE_Latin1;
    else if ("UCS2&Latin1".equalsIgnoreCase(codepageString))
      codepage = CODEPAGE_UCS2AndLatin1;
    else
      throw new AdminException("Unknown codepage: " + codepageString);
  }

  public byte getReportOptions()
  {
    return reportOptions;
  }

  public String getReportOptionsString() throws AdminException
  {
    return getReportOptionsString(reportOptions);
  }

  public static String getReportOptionsString(final byte reportOptions) throws AdminException
  {
    switch (reportOptions) {
      case REPORT_OPTION_Full:
        return "full";
      case REPORT_OPTION_Final:
        return "final";
      case REPORT_OPTION_None:
        return "none";
      default:
        throw new AdminException("Report option is not initialized");
    }
  }

  public void setReportOptions(final byte reportOptions)
  {
    this.reportOptions = reportOptions;
  }

  private void setReportOptions(final String reportoptionsString) throws AdminException
  {
    if ("full".equalsIgnoreCase(reportoptionsString))
      reportOptions = REPORT_OPTION_Full;
    else if ("final".equalsIgnoreCase(reportoptionsString))
      reportOptions = REPORT_OPTION_Final;
    else if ("none".equalsIgnoreCase(reportoptionsString))
      reportOptions = REPORT_OPTION_None;
    else
      throw new AdminException("Unknown report option: " + reportoptionsString);
  }

  public Mask getMask()
  {
    return mask;
  }

  public String getLocale()
  {
    return locale;
  }

  public void setLocale(final String locale)
  {
    this.locale = locale;
    if (null == this.locale)
      this.locale = "";
  }

  public byte getAliasHide()
  {
    return aliasHide;
  }

  public void setAliasHide(final byte aliasHide)
  {
    this.aliasHide = aliasHide;
  }

  private void setAliasHide(final String aliasHide)
  {
    if (null == aliasHide)
      this.aliasHide = ALIAS_HIDE_false;
    else if ("false".equalsIgnoreCase(aliasHide) || "N".equalsIgnoreCase(aliasHide))
      this.aliasHide = ALIAS_HIDE_false;
    else if ("true".equalsIgnoreCase(aliasHide) || "hide".equalsIgnoreCase(aliasHide) || "Y".equalsIgnoreCase(aliasHide))
      this.aliasHide = ALIAS_HIDE_true;
    else if ("substitute".equalsIgnoreCase(aliasHide) || "S".equalsIgnoreCase(aliasHide))
      this.aliasHide = ALIAS_HIDE_substitute;
    else
      this.aliasHide = ALIAS_HIDE_false;
  }

  public  String getAliasHideString()
  {
    return getAliasHideString(aliasHide);
  }
  
  public static String getAliasHideString(final byte aliasHide)
  {
    switch (aliasHide) {
      case ALIAS_HIDE_false:
        return "false";
      case ALIAS_HIDE_true:
        return "true";
      case ALIAS_HIDE_substitute:
        return "substitute";
      default:
        return "unknown";
    }
  }

  public boolean isAliasModifiable()
  {
    return aliasModifiable;
  }

  public void setAliasModifiable(final boolean aliasModifiable)
  {
    this.aliasModifiable = aliasModifiable;
  }

  private void setAliasModifiable(final String aliasModifiable)
  {
    this.aliasModifiable = "true".equalsIgnoreCase(aliasModifiable) || "modifiable".equalsIgnoreCase(aliasModifiable);
  }

  public String getDivert()
  {
    return divert;
  }

  public void setDivert(final String divert)
  {
    this.divert = divert;
  }

  public String getDivertActive()
  {
    final StringBuffer result = new StringBuffer(5);
    result.append(divertActiveUnconditional ? 'Y' : 'N');
    result.append(divertActiveAbsent ? 'Y' : 'N');
    result.append(divertActiveBlocked ? 'Y' : 'N');
    result.append(divertActiveBarred ? 'Y' : 'N');
    result.append(divertActiveCapacity ? 'Y' : 'N');
    return result.toString();
  }

  public void setDivertActive(final String divertActive)
  {
    this.divertActiveUnconditional = 'Y' == Character.toUpperCase(divertActive.charAt(0));
    this.divertActiveAbsent = 'Y' == Character.toUpperCase(divertActive.charAt(1));
    this.divertActiveBlocked = 'Y' == Character.toUpperCase(divertActive.charAt(2));
    this.divertActiveBarred = 'Y' == Character.toUpperCase(divertActive.charAt(3));
    this.divertActiveCapacity = 'Y' == Character.toUpperCase(divertActive.charAt(4));
  }

  public boolean isDivertModifiable()
  {
    return divertModifiable;
  }

  public void setDivertModifiable(final boolean divertModifiable)
  {
    this.divertModifiable = divertModifiable;
  }

  private void setDivertModifiable(final String divert_mod)
  {
    this.divertModifiable = "true".equalsIgnoreCase(divert_mod);
  }

  public boolean isUssd7bit()
  {
    return ussd7bit;
  }

  public void setUssd7bit(final boolean ussd7bit)
  {
    this.ussd7bit = ussd7bit;
  }

  private void setUssd7bit(final String ussd7bit)
  {
    this.ussd7bit = Boolean.valueOf(ussd7bit).booleanValue();
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

  public void setUdhConcat(final String udhConcat)
  {
    this.udhConcat = null != udhConcat
                     ? 1 == udhConcat.length()
                       ? 'Y' == Character.toUpperCase(udhConcat.charAt(0))
                       : Boolean.valueOf(udhConcat).booleanValue()
                     : false;
  }

  public boolean isTranslit()
  {
    return translit;
  }

  public void setTranslit(final boolean translit)
  {
    this.translit = translit;
  }

  public void setTranslit(final String translit)
  {
    this.translit = null != translit
                    ? 1 == translit.length()
                      ? 'Y' == Character.toUpperCase(translit.charAt(0))
                      : Boolean.valueOf(translit).booleanValue()
                    : false;
  }
}
