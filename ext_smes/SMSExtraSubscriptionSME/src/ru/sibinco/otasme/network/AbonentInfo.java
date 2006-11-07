package ru.sibinco.otasme.network;

class AbonentInfo {

  public static final int ENG = 0;
  public static final int RUS = 1;

  private String phone = null;
  private int language = ENG;

  public AbonentInfo(String phone) {
    setPhone(phone);
  }

  public String getPhone() {
    return phone;
  }

  public void setPhone(String phone) {
    this.phone = phone;
  }

  public int getLanguage() {
    return language;
  }

  public void setLanguage(int language) {
    switch (language) {
      case RUS:
        this.language = language;
        break;
      default:
        this.language = ENG;
        break;
    }
  }
}
