/*
 * User: igork
 * Date: 20.08.2002
 * Time: 13:30:24
 */
package ru.novosoft.smsc.admin.profiler;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.route.Mask;

import java.util.List;
import java.util.StringTokenizer;

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
  private String divert;
  private boolean divertActive;
  private boolean divertModifiable;

  private byte codepage;
  private byte reportOptions;
  private String locale;

  private boolean aliasHide = false;
  private boolean aliasModifiable = true;

  public Profile(Mask mask, List profileProperties) throws AdminException
  {
    this.mask = mask;
    setCodepage((String) profileProperties.get(0));
    setReportOptions((String) profileProperties.get(1));
    setLocale((String) profileProperties.get(2));
    setAliasHide((String) profileProperties.get(3));
    setAliasModifiable((String) profileProperties.get(4));
    setDivert((String) profileProperties.get(5));
    setDivert_act((String) profileProperties.get(6));
    setDivert_mod((String) profileProperties.get(7));
  }

  /**
   * Constructs <code>Profile</code> from string returned by SMSC.
   * @param profileString profile string returned by SMSC. Must be
   * formatted as <code>&lt;codepage&gt;,&lt;report options&gt;</code>
   * where &lt;codepage&gt; must be "default" or "UCS2" and &lt;report
   * options&gt; must be "full" or "none".
   */
/*
  public Profile(Mask mask, String profileString) throws AdminException
  {
    this.mask = mask;
    int i = 0;
    for (StringTokenizer tokenizer = new StringTokenizer(profileString, ",", false); tokenizer.hasMoreTokens();) {
      String token = tokenizer.nextToken().trim();
      switch (i++) {
        case 0:
          setCodepage(token);
          break;
        case 1:
          setReportOptions(token);
          break;
        case 2:
          setLocale(token);
          break;
        case 3:
          setAliasHide(token);
          break;
        case 4:
          setAliasModifiable(token);
          break;
      }
    }
    if (i < 5)
      throw new AdminException("profile string returned by SMSC misformatted: " + profileString);
  }
*/

  public Profile(Mask mask, byte codepage, byte reportOptions, String locale, boolean aliasHide, boolean aliasModifiable, String divert, boolean divertActive, boolean divertModifiable)
  {
    this.mask = mask;
    this.divert = divert;
    this.divertActive = divertActive;
    this.divertModifiable = divertModifiable;
    setCodepage(codepage);
    setReportOptions(reportOptions);
    setLocale(locale);
    this.aliasHide = aliasHide;
    this.aliasModifiable = aliasModifiable;
  }

  public Profile(Mask mask, String codepage, String reportOptions, String locale, boolean aliasHide, boolean aliasModifiable, String divert, boolean divert_act, boolean divert_mod) throws AdminException
  {
    this.mask = mask;
    this.divert = divert;
    this.divertActive = divert_act;
    this.divertModifiable = divert_mod;
    setCodepage(codepage);
    setReportOptions(reportOptions);
    setLocale(locale);
    this.aliasHide = aliasHide;
    this.aliasModifiable = aliasModifiable;
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

  public void setCodepage(String codepageString) throws AdminException
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

  public void setReportOptions(byte reportOptions)
  {
    this.reportOptions = reportOptions;
  }

  public void setReportOptions(String reportoptionsString) throws AdminException
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

/*  public String getStringRepresentation() throws AdminException
  {
    return getCodepageString() + ',' + getReportOptionsString() + ',' + locale +
            ',' + (aliasHide ? "true" : "false") + ',' + (aliasModifiable ? "true" : "false");
  }
*/
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

  public void setAliasHide(String aliasHide)
  {
    this.aliasHide = (aliasHide.equalsIgnoreCase("true") ||
            aliasHide.equalsIgnoreCase("hide"));
  }

  public boolean isAliasModifiable()
  {
    return aliasModifiable;
  }

  public void setAliasModifiable(boolean aliasModifiable)
  {
    this.aliasModifiable = aliasModifiable;
  }

  public void setAliasModifiable(String aliasModifiable)
  {
    this.aliasModifiable = (aliasModifiable.equalsIgnoreCase("true") ||
            aliasModifiable.equalsIgnoreCase("modifiable"));
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

  public String getDivert()
  {
    return divert;
  }

  public void setDivert(String divert)
  {
    this.divert = divert;
  }

  public boolean isDivertActive()
  {
    return divertActive;
  }

  public void setDivertActive(boolean divertActive)
  {
    this.divertActive = divertActive;
  }

  public void setDivert_act(String divert_act)
  {
    this.divertActive = divert_act.equalsIgnoreCase("true");
  }

  public boolean isDivertModifiable()
  {
    return divertModifiable;
  }

  public void setDivertModifiable(boolean divertModifiable)
  {
    this.divertModifiable = divertModifiable;
  }

  public void setDivert_mod(String divert_mod)
  {
    this.divertModifiable = divert_mod.equalsIgnoreCase("true");
  }
}
