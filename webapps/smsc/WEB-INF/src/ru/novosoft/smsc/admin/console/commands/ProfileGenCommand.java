/**
 * Created by IntelliJ IDEA.
 * User: makarov
 * Date: Dec 23, 2002
 * Time: 2:33:18 PM
 * To change this template use Options | File Templates.
 */
package ru.novosoft.smsc.admin.console.commands;

import ru.novosoft.smsc.admin.console.Command;
import ru.novosoft.smsc.admin.profiler.Profile;

public abstract class ProfileGenCommand implements Command
{
  protected byte report = Profile.REPORT_OPTION_None;
  protected byte codepage = Profile.CODEPAGE_Default;
  protected boolean ussd7bit = false;

  protected boolean aliasHide = false;
  protected boolean aliasModifiable = false;

  protected String divert = "";
  protected boolean divertActiveOn = false;
  protected boolean divertActiveAbsent = false;
  protected boolean divertActiveBarred = false;
  protected boolean divertActiveBlocked = false;
  protected boolean divertActiveCapacity = false;
  protected boolean divertActiveUnconditional = false;
  protected boolean divertModifiable = false;

  protected boolean isAliasOptions = false;
  protected boolean isDivertOptions = false;
  protected boolean isReport = false;
  protected boolean isCodepage = false;
  protected boolean isAliasHide = false;
  protected boolean isAliasModifiable = false;
  protected boolean isDivert = false;
  protected boolean isDivertActiveAbsent = false;
  protected boolean isDivertActiveBarred = false;
  protected boolean isDivertActiveBlocked = false;
  protected boolean isDivertActiveCapacity = false;
  protected boolean isDivertActiveUnconditional = false;
  protected boolean isDivertModifiable = false;
  protected boolean isUdhConcat = false; //todo: implement

  protected String locale;
  protected boolean isLocale = false;

  public void setNoneReport()
  {
    this.report = Profile.REPORT_OPTION_None;
    isReport = true;
  }

  public void setFullReport()
  {
    this.report = Profile.REPORT_OPTION_Full;
    isReport = true;
  }

  public void setLocale(String locale)
  {
    this.locale = locale;
    isLocale = true;
  }

  public void setGsm7Encoding()
  {
    this.codepage = Profile.CODEPAGE_Default;
    isCodepage = true;
  }

  public void setUcs2Encoding()
  {
    this.codepage = Profile.CODEPAGE_UCS2;
    isCodepage = true;
  }

  public void setLatin1Encoding()
  {
    this.codepage = Profile.CODEPAGE_Latin1;
    isCodepage = true;
  }

  public void setUcs2Latin1Encoding()
  {
    this.codepage = Profile.CODEPAGE_UCS2AndLatin1;
    isCodepage = true;
  }

  public void setUssd7Bit(boolean ussd7bit)
  {
    this.ussd7bit = ussd7bit;
  }

  public void setAliasHide(boolean aliasHide)
  {
    this.aliasHide = aliasHide;
    isAliasHide = true;
  }

  public void setAliasModifiable(boolean aliasModifiable)
  {
    this.aliasModifiable = aliasModifiable;
    isAliasModifiable = true;
  }

  public void setDivert(String divert)
  {
    this.divert = divert;
    isDivert = true;
    isDivertOptions = true;
  }

  public void setDivertActiveOn(boolean on)
  {
    this.divertActiveOn = on;
    isDivertOptions = true;
  }

  public void setDivertActiveAbsent(boolean divertActiveAbsent)
  {
    this.divertActiveAbsent = divertActiveAbsent;
    this.isDivertActiveAbsent = true;
  }

  public void setDivertActiveBarred(boolean divertActiveBarred)
  {
    this.divertActiveBarred = divertActiveBarred;
    this.isDivertActiveBarred = true;
  }

  public void setDivertActiveCapacity(boolean divertActiveCapacity)
  {
    this.divertActiveCapacity = divertActiveCapacity;
    this.isDivertActiveCapacity = true;
  }

  public void setDivertActiveUnconditional(boolean divertActiveUnconditional)
  {
    this.divertActiveUnconditional = divertActiveUnconditional;
    this.isDivertActiveUnconditional = true;
  }

  public void setDivertModifiable(boolean divertModifiable)
  {
    this.divertModifiable = divertModifiable;
    isDivertModifiable = true;
  }

  public void setAliasOptions(boolean aliasOptions)
  {
    isAliasOptions = aliasOptions;
  }

  public void setDivertOptions(boolean divertOptions)
  {
    isDivertOptions = divertOptions;
  }
}
