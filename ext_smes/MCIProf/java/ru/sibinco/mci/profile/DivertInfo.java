package ru.sibinco.mci.profile;

/**
 * Created by IntelliJ IDEA.
 * User: makar
 * Date: 06.09.2004
 * Time: 18:44:36
 * To change this template use File | Settings | File Templates.
 */
public class DivertInfo
{
  public final static String BUSY     = "busy";
  public final static String ABSENT   = "absent";
  public final static String NOTAVAIL = "notavail";
  public final static String UNCOND   = "uncond";

  protected String busy     = null;
  protected String absent   = null;
  protected String notavail = null;
  protected String uncond   = null;

  public DivertInfo() {
  }
  public DivertInfo(String busy, String absent, String notavail, String uncond) {
    this.busy = busy; this.absent = absent; this.notavail = notavail; this.uncond = uncond;
  }

  public String getBusy() {
    return busy;
  }
  public String getAbsent() {
    return absent;
  }
  public String getNotavail() {
    return notavail;
  }
  public String getUncond() {
    return uncond;
  }
}
