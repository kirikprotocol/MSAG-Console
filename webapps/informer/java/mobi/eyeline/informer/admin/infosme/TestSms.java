package mobi.eyeline.informer.admin.infosme;

import mobi.eyeline.informer.util.Address;

/**
 * @author Aleksandr Khalitov
 */
public class TestSms {

  private Address sourceAddr;

  private Address destAddr;
  private String text;
  private boolean flash;

  private Mode mode;

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

  public void setFlash(boolean flash) {
    this.flash = flash;
  }

  public Mode getMode() {
    return mode;
  }

  public void setMode(Mode mode) {
    this.mode = mode;
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

  public static enum Mode {
    SMS,
    USSD_PUSH,
    USSD_PUSH_VLR
  }


}
