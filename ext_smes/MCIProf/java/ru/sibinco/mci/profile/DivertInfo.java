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

  private String busy     = null; private boolean busyChanged     = false;
  private String absent   = null; private boolean absentChanged   = false;
  private String notavail = null; private boolean notavailChanged = false;
  private String uncond   = null; private boolean uncondChanged   = false;

  public DivertInfo() {
  }
  public DivertInfo(DivertInfo info) {
    this.busy = info.busy; this.absent = info.absent;
    this.notavail = info.notavail; this.uncond = info.uncond;
  }
  public DivertInfo(String busy, String absent, String notavail, String uncond) {
    this.busy = busy; this.absent = absent; this.notavail = notavail; this.uncond = uncond;
  }

  public void clearBusy()     { busyChanged = false; }
  public void clearAbsent()   { absentChanged = false; }
  public void clearNotavail() { notavailChanged = false; }
  public void clearUncond()   { uncondChanged = false; }

  public boolean isBusyChanged() {
    return busyChanged;
  }
  public boolean isAbsentChanged() {
    return absentChanged;
  }
  public boolean isNotavailChanged() {
    return notavailChanged;
  }
  public boolean isUncondChanged() {
    return uncondChanged;
  }

  public String getBusy() {
    return busy;
  }
  public void setBusy(String busy) {
    this.busy = busy; busyChanged = true;
  }
  public String getAbsent() {
    return absent;
  }
  public void setAbsent(String absent) {
    this.absent = absent; absentChanged = true;
  }
  public String getNotavail() {
    return notavail;
  }
  public void setNotavail(String notavail) {
    this.notavail = notavail; notavailChanged = true;
  }
  public String getUncond() {
    return uncond;
  }
  public void setUncond(String uncond) {
    this.uncond = uncond; uncondChanged = true;
  }

}
