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
  protected boolean inform = true;
  protected boolean notify = false;
  protected FormatType informFormat = new FormatType();
  protected FormatType notifyFormat = new FormatType();

  public ProfileInfo() {
  }
  public ProfileInfo(FormatType informFormat, FormatType notifyFormat, boolean inform, boolean notify) {
    this.inform = inform;  this.notify = notify;
    this.informFormat = informFormat; this.notifyFormat = notifyFormat;
  }

  public boolean isInform() {
    return inform;
  }
  public boolean isNotify() {
    return notify;
  }
  public FormatType getInformFormat() {
    return informFormat;
  }
  public FormatType getNotifyFormat() {
    return notifyFormat;
  }
}
