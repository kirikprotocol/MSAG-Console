package ru.sibinco.mci.profile;

/**
 * Created by IntelliJ IDEA.
 * User: makar
 * Date: 09.09.2004
 * Time: 15:39:51
 * To change this template use File | Settings | File Templates.
 */
public class ProfileInfo
{
  protected boolean inform = false;
  protected boolean notify = false;
  protected String  informFormat = null;
  protected String  notifyFormat = null;

  public ProfileInfo() {
  }
  public ProfileInfo(String informFormat, String notifyFormat, boolean inform, boolean notify) {
    this.inform = inform;  this.notify = notify;
    this.informFormat = informFormat; this.notifyFormat = notifyFormat;
  }

  public boolean isInform() {
    return inform;
  }
  public boolean isNotify() {
    return notify;
  }
  public String getInformFormat() {
    return informFormat;
  }
  public String getNotifyFormat() {
    return notifyFormat;
  }
}
