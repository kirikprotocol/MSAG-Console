/*
 * User: igork
 * Date: 20.08.2002
 * Time: 13:30:24
 */
package ru.novosoft.smsc.admin.profiler;

import ru.novosoft.smsc.admin.AdminException;

public class Profile {
  public static final byte CODEPAGE_Default = 0;
  public static final byte CODEPAGE_UCS2    = 1;
  public static final byte REPORT_OPTION_None = 0;
  public static final byte REPORT_OPTION_Full = 1;

  private byte codepage;
  private byte reportOptions;

  /**
   * Constructs <code>Profile</code> from string returned by SMSC.
   * @param profileString profile string returned by SMSC. Must be
   * formatted as <code>&lt;codepage&gt;,&lt;report options&gt;</code>
   * where &lt;codepage&gt; must be "default" or "UCS2" and &lt;report
   * options&gt; must be "full" or "none".
   */
  public Profile(String profileString)
    throws AdminException
  {
    int delimPos = profileString.indexOf(',');
    if (delimPos<1)
      throw new AdminException("profile string returned by SMSC misformatted");
    String codepageString = profileString.substring(0, delimPos);
    String reportoptionsString = profileString.substring(delimPos +1);

    if (codepageString.equalsIgnoreCase("default"))
      codepage = CODEPAGE_Default;
    else if (codepageString.equalsIgnoreCase("UCS2"))
      codepage = CODEPAGE_UCS2;
    else
      throw new AdminException("Unknown codepage: " + codepageString);

    if (reportoptionsString.equalsIgnoreCase("full"))
      reportOptions = REPORT_OPTION_Full;
    else if (reportoptionsString.equalsIgnoreCase("none"))
      reportOptions = REPORT_OPTION_None;
    else
      throw new AdminException("Unknown report option: " + reportoptionsString);
  }

  public Profile(byte codepage, byte reportOptions) {
    this.codepage = codepage;
    this.reportOptions = reportOptions;
  }

  public byte getCodepage() {
    return codepage;
  }

  public void setCodepage(byte codepage) {
    this.codepage = codepage;
  }

  public byte getReportOptions() {
    return reportOptions;
  }

  public void setReportOptions(byte reportOptions) {
    this.reportOptions = reportOptions;
  }

  public String getStringRepresentation()
      throws AdminException
  {
    String result = "";
    switch (codepage)
    {
      case CODEPAGE_Default:
        result += "default";
        break;
      case CODEPAGE_UCS2:
        result += "UCS2";
        break;
      default:
        throw new AdminException("Codepage is not initialized");
    }
    result += ',';
    switch (reportOptions)
    {
      case REPORT_OPTION_Full:
        result += "full";
        break;
      case REPORT_OPTION_None:
        result += "none";
        break;
      default:
        throw new AdminException("Report option is not initialized");
    }
    return result;
  }
}
