package ru.novosoft.smsc.jsp.smsc.profiles;

import ru.novosoft.smsc.admin.profiler.Profile;
import ru.novosoft.smsc.jsp.SMSCErrors;
import ru.novosoft.smsc.jsp.smsc.SmscBean;

import java.util.LinkedList;
import java.util.List;

/**
 * Created by igork
 * Date: 11.03.2003
 * Time: 18:38:02
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

  protected boolean aliasHide = false;
  protected boolean aliasModifiable = false;

  protected String divert = "";
  protected boolean divertActiveUnconditional;
  protected boolean divertActiveAbsent;
  protected boolean divertActiveBlocked;
  protected boolean divertActiveBarred;
  protected boolean divertActiveCapacity;
  protected boolean divertModifiable = false;

  protected int init(List errors)
  {
    int result = super.init(errors);
    if (result != RESULT_OK)
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

  public void setMask(String mask)
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

  public void setReport(String report)
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

  public void setCodepage(String codepage)
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

  public void setLocale(String locale)
  {
    this.locale = locale;
  }

  public List getRegisteredLocales()
  {
    return registeredLocales;
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

  public String getReturnPath()
  {
    return returnPath;
  }

  public void setReturnPath(String returnPath)
  {
    this.returnPath = returnPath;
  }

  public String getDivert()
  {
    return divert;
  }

  public void setDivert(String divert)
  {
    this.divert = divert;
  }

  public boolean isDivertModifiable()
  {
    return divertModifiable;
  }

  public void setDivertModifiable(boolean divertModifiable)
  {
    this.divertModifiable = divertModifiable;
  }

  public boolean isUssd7bit()
  {
    return ussd7bit;
  }

  public void setUssd7bit(boolean ussd7bit)
  {
    this.ussd7bit = ussd7bit;
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
}
