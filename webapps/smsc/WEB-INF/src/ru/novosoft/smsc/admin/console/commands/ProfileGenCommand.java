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
  protected byte    report = Profile.REPORT_OPTION_None;
  protected byte    codepage = Profile.CODEPAGE_Default;
  protected boolean aliasHide = false;
  protected boolean aliasModifiable = false;
  protected String  divert = "";
  protected boolean divertActive = false;
  protected boolean divertModifiable = false;

  protected boolean isAliasOptions = false;
  protected boolean isDivertOptions = false;
  protected boolean isReport = false;
  protected boolean isCodepage = false;
  protected boolean isAliasHide = false;
  protected boolean isAliasModifiable = false;
  protected boolean isDivert = false;
  protected boolean isDivertActive = false;
  protected boolean isDivertModifiable = false;

  protected String locale;
  protected boolean isLocale = false;

  public void setNoneReport() {
    this.report = Profile.REPORT_OPTION_None; isReport = true;
  }
  public void setFullReport() {
    this.report = Profile.REPORT_OPTION_Full; isReport = true;
  }
  public void setLocale(String locale) {
    this.locale = locale; isLocale = true;
  }
  public void setGsm7Encoding() {
    this.codepage = Profile.CODEPAGE_Default; isCodepage = true;
  }
  public void setUcs2Encoding() {
    this.codepage = Profile.CODEPAGE_UCS2; isCodepage = true;
  }

  public void setAliasHide(boolean aliasHide) {
    this.aliasHide = aliasHide; isAliasHide = true;
  }
  public void setAliasModifiable(boolean aliasModifiable) {
    this.aliasModifiable = aliasModifiable; isAliasModifiable = true;
  }
  public void setDivert(String divert) {
    this.divert = divert; isDivert = true; isDivertOptions = true;
  }
  public void setDivertActive(boolean divertActive) {
    this.divertActive = divertActive; isDivertActive = true;
  }
  public void setDivertModifiable(boolean divertModifiable) {
    this.divertModifiable = divertModifiable; isDivertModifiable = true;
  }

  public void setAliasOptions(boolean aliasOptions) {
    isAliasOptions = aliasOptions;
  }
  public void setDivertOptions(boolean divertOptions) {
    isDivertOptions = divertOptions;
  }
}
