package ru.novosoft.smsc.admin.smsview;

/**
 * Created by Serge Lugovoy
 * Date: Feb 28, 2003
 * Time: 8:57:24 AM
 */
public class SmsDescriptor {
  private String msc;
  private String imsi;
  private int    sme;

  public SmsDescriptor() {
  }

  public SmsDescriptor(String imsi, String msc, int sme) {
    this.imsi = imsi;
    this.msc = msc;
    this.sme = sme;
  }

  public String getImsi() {
    return imsi;
  }

  public String getMsc() {
    return msc;
  }

  public int getSme() {
    return sme;
  }

  public void setImsi(String imsi) {
    this.imsi = imsi;
  }

  public void setMsc(String msc) {
    this.msc = msc;
  }

  public void setSme(int sme) {
    this.sme = sme;
  }
}
