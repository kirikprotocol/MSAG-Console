package ru.novosoft.smsc.web.controllers.sms_view;

/**
 * @author Artem Snopkov
 */
public class RoutingInfo {
  private String msc;
  private String imsi;
  private int sme;

  RoutingInfo(String imsi, String msc, int sme) {
    this.msc = msc;
    this.imsi = imsi;
    this.sme = sme;
  }

  public String getMsc() {
    return msc;
  }

  public void setMsc(String msc) {
    this.msc = msc;
  }

  public String getImsi() {
    return imsi;
  }

  public void setImsi(String imsi) {
    this.imsi = imsi;
  }

  public int getSme() {
    return sme;
  }

  public void setSme(int sme) {
    this.sme = sme;
  }
}
