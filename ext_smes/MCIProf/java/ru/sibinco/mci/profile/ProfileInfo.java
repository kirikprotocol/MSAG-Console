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
  public final static int MASK_NONE    = 0x00;
  public final static int MASK_ABSENT  = 0x01;
  public final static int MASK_BUSY    = 0x02;
  public final static int MASK_NOREPLY = 0x04;
  public final static int MASK_UNCOND  = 0x08;
  public final static int MASK_ALL     = 0xFF;

  protected boolean inform = true;
  protected boolean notify = false;
  protected int  eventMask = MASK_ALL;
  protected FormatType informFormat = new FormatType();
  protected FormatType notifyFormat = new FormatType();

  public ProfileInfo() {
  }
  public ProfileInfo(FormatType informFormat, FormatType notifyFormat,
                     boolean inform, boolean notify, int eventMask)
  {
    this.eventMask = eventMask;
    this.inform = inform;  this.notify = notify;
    this.informFormat = informFormat; this.notifyFormat = notifyFormat;
  }

  public int getEventMask() {
    return eventMask;
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
