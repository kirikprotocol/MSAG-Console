package ru.novosoft.smsc.admin.archive_daemon;

//todo javadoc

/**
 * Дескриптор смс
 *
 * @author Aleksandr Khalitov
 */

public class SmsDescriptor {
  private String msc;
  private String imsi;
  private int sme;


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
