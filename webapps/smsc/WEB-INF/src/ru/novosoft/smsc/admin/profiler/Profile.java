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

  private boolean aliasHide = false;
  private boolean aliasModifiable = true;
  private boolean udhConcat;

  public Profile(Mask mask, byte codepage, boolean ussd7bit, byte reportOptions, String locale, boolean aliasHide, boolean aliasModifiable, String divert, boolean divertActiveUnconditional, boolean divertActiveAbsent, boolean divertActiveBlocked, boolean divertActiveBarred, boolean divertActiveCapacity, boolean divertModifiable, boolean udhConcat)
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
  }

  public Profile(Mask mask, byte codepage, boolean ussd7bit, byte reportOptions, String locale, boolean aliasHide, boolean aliasModifiable, String divert, String divertActive, boolean divertModifiable, boolean udhConcat)
  {
    this.mask = mask;
    this.ussd7bit = ussd7bit;
    this.divert = divert;
    setDivertActive(divertActive);
    this.divertModifiable = divertModifiable;
    this.codepage = codepage;
    this.reportOptions = reportOptions;
    this.locale = locale;
    this.aliasHide = aliasHide;
    this.aliasModifiable = aliasModifiable;
    this.udhConcat = udhConcat;
  }

  public Profile(Mask mask, String codepage, String ussd7bit, String reportOptions, String locale, String aliasHide, String aliasModifiable, String divert, String divert_act, String divert_mod, String udhConcat) throws AdminException
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
  }

  public Profile(Mask mask, List profileProperties) throws AdminException
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
         (String) profileProperties.get(9));
  }

  public byte getCodepage()
  {
    return codepage;
  }

  public String getCodepageString() throws AdminException
  {
    return getCodepageString(codepage);
  }

  public static String getCodepageString(byte codepage) throws AdminException
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

  public void setCodepage(byte codepage)
  {
    this.codepage = codepage;
  }

  private void setCodepage(String codepageString) throws AdminException
  {
    if (codepageString.equalsIgnoreCase("default"))
      codepage = CODEPAGE_Default;
    else if (codepageString.equalsIgnoreCase("UCS2"))
      codepage = CODEPAGE_UCS2;
    else if (codepageString.equalsIgnoreCase("Latin1"))
      codepage = CODEPAGE_Latin1;
    else if (codepageString.equalsIgnoreCase("UCS2&Latin1"))
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

  public static String getReportOptionsString(byte reportOptions) throws AdminException
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

  public void setReportOptions(byte reportOptions)
  {
    this.reportOptions = reportOptions;
  }

  private void setReportOptions(String reportoptionsString) throws AdminException
  {
    if (reportoptionsString.equalsIgnoreCase("full"))
      reportOptions = REPORT_OPTION_Full;
    else if (reportoptionsString.equalsIgnoreCase("final"))
      reportOptions = REPORT_OPTION_Final;
    else if (reportoptionsString.equalsIgnoreCase("none"))
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

  public void setLocale(String locale)
  {
    this.locale = locale;
    if (this.locale == null)
      this.locale = "";
  }

  public boolean isAliasHide()
  {
    return aliasHide;
  }

  public void setAliasHide(boolean aliasHide)
  {
    this.aliasHide = aliasHide;
  }

  private void setAliasHide(String aliasHide)
  {
    if( aliasHide == null ) this.aliasHide = false;
    else {
      this.aliasHide = (aliasHide.equalsIgnoreCase("true") ||
                      aliasHide.equalsIgnoreCase("hide"));
    }
  }

  public boolean isAliasModifiable()
  {
    return aliasModifiable;
  }

  public void setAliasModifiable(boolean aliasModifiable)
  {
    this.aliasModifiable = aliasModifiable;
  }

  private void setAliasModifiable(String aliasModifiable)
  {
    this.aliasModifiable = (aliasModifiable.equalsIgnoreCase("true") ||
                            aliasModifiable.equalsIgnoreCase("modifiable"));
  }

  public String getDivert()
  {
    return divert;
  }

  public void setDivert(String divert)
  {
    this.divert = divert;
  }

  public String getDivertActive()
  {
    StringBuffer result = new StringBuffer(5);
    result.append(divertActiveUnconditional ? 'Y' : 'N');
    result.append(divertActiveAbsent ? 'Y' : 'N');
    result.append(divertActiveBlocked ? 'Y' : 'N');
    result.append(divertActiveBarred ? 'Y' : 'N');
    result.append(divertActiveCapacity ? 'Y' : 'N');
    return result.toString();
  }

  public void setDivertActive(String divertActive)
  {
    this.divertActiveUnconditional = Character.toUpperCase(divertActive.charAt(0)) == 'Y';
    this.divertActiveAbsent = Character.toUpperCase(divertActive.charAt(1)) == 'Y';
    this.divertActiveBlocked = Character.toUpperCase(divertActive.charAt(2)) == 'Y';
    this.divertActiveBarred = Character.toUpperCase(divertActive.charAt(3)) == 'Y';
    this.divertActiveCapacity = Character.toUpperCase(divertActive.charAt(4)) == 'Y';
  }

  public boolean isDivertModifiable()
  {
    return divertModifiable;
  }

  public void setDivertModifiable(boolean divertModifiable)
  {
    this.divertModifiable = divertModifiable;
  }

  private void setDivertModifiable(String divert_mod)
  {
    this.divertModifiable = divert_mod.equalsIgnoreCase("true");
  }

  public boolean isUssd7bit()
  {
    return ussd7bit;
  }

  public void setUssd7bit(boolean ussd7bit)
  {
    this.ussd7bit = ussd7bit;
  }

  private void setUssd7bit(String ussd7bit)
  {
    this.ussd7bit = Boolean.valueOf(ussd7bit).booleanValue();
  }

  public boolean isDivertActiveUnconditional()
  {
    return divertActiveUnconditional;
  }

  public void setDivertActiveUnconditional(boolean divertActiveUnconditional)
  {
    this.divertActiveUnconditional = divertActiveUnconditional;
  }

  public boolean isDivertActiveAbsent()
  {
    return divertActiveAbsent;
  }

  public void setDivertActiveAbsent(boolean divertActiveAbsent)
  {
    this.divertActiveAbsent = divertActiveAbsent;
  }

  public boolean isDivertActiveBlocked()
  {
    return divertActiveBlocked;
  }

  public void setDivertActiveBlocked(boolean divertActiveBlocked)
  {
    this.divertActiveBlocked = divertActiveBlocked;
  }

  public boolean isDivertActiveBarred()
  {
    return divertActiveBarred;
  }

  public void setDivertActiveBarred(boolean divertActiveBarred)
  {
    this.divertActiveBarred = divertActiveBarred;
  }

  public boolean isDivertActiveCapacity()
  {
    return divertActiveCapacity;
  }

  public void setDivertActiveCapacity(boolean divertActiveCapacity)
  {
    this.divertActiveCapacity = divertActiveCapacity;
  }

  public boolean isUdhConcat()
  {
    return udhConcat;
  }

  public void setUdhConcat(boolean udhConcat)
  {
    this.udhConcat = udhConcat;
  }

  public void setUdhConcat(String udhConcat)
  {
    this.udhConcat = udhConcat != null
                     ? (udhConcat.length() == 1
                        ? Character.toUpperCase(udhConcat.charAt(0)) == 'Y'
                        : Boolean.valueOf(udhConcat).booleanValue())
                     : false;
  }
}
