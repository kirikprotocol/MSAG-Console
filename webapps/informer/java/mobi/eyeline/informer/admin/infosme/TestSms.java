package mobi.eyeline.informer.admin.infosme;

import mobi.eyeline.informer.util.Address;

/**
 * @author Aleksandr Khalitov
 */
public final class TestSms {

  private Address sourceAddr;

  private Address destAddr;
  private String text;
  private boolean flash;

  private Mode mode;

  private TestSms() {
  }

  public static TestSms sms(boolean flash) {
    TestSms sms = new TestSms();
    sms.mode = Mode.SMS;
    sms.flash = flash;
    return sms;
  }

  public static TestSms sms(Address src, Address dst, String text, boolean flash) {
    TestSms sms = sms(flash);
    sms.setSourceAddr(src);
    sms.setDestAddr(dst);
    sms.setText(text);
    return sms;
  }

  public static TestSms ussdPush() {
    TestSms sms = new TestSms();
    sms.mode = Mode.USSD_PUSH;
    return sms;
  }

  public static TestSms ussdPush(Address src, Address dst, String text) {
    TestSms sms = ussdPush();
    sms.setSourceAddr(src);
    sms.setDestAddr(dst);
    sms.setText(text);
    return sms;
  }

  public static TestSms ussdPushViaVlr() {
    TestSms sms = new TestSms();
    sms.mode = Mode.USSD_PUSH_VLR;
    return sms;
  }

  public static TestSms ussdPushViaVlr(Address src, Address dst, String text) {
    TestSms sms = ussdPushViaVlr();
    sms.setSourceAddr(src);
    sms.setDestAddr(dst);
    sms.setText(text);
    return sms;
  }

  public Address getSourceAddr() {
    return sourceAddr;
  }

  public void setSourceAddr(Address sourceAddr) {
    this.sourceAddr = sourceAddr;
  }

  public Address getDestAddr() {
    return destAddr;
  }

  public void setDestAddr(Address destAddr) {
    this.destAddr = destAddr;
  }

  public String getText() {
    return text;
  }

  public void setText(String text) {
    this.text = text;
  }

  public boolean isFlash() {
    return flash;
  }



  Mode getMode() {
    return mode;
  }

  @Override
  public String toString() {
    final StringBuilder sb = new StringBuilder();
    sb.append("TestSms");
    sb.append("{sourceAddr=").append(sourceAddr);
    sb.append(", destAddr=").append(destAddr);
    sb.append(", text='").append(text).append('\'');
    sb.append(", flash=").append(flash);
    sb.append(", mode=").append(mode);
    sb.append('}');
    return sb.toString();
  }

  protected static enum Mode {
    SMS,
    USSD_PUSH,
    USSD_PUSH_VLR
  }


}
