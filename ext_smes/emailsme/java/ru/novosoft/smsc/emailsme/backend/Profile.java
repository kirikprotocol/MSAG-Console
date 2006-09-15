package ru.novosoft.smsc.emailsme.backend;

/**
 * User: artem
 * Date: 08.09.2006
 */
public class Profile {
  private String addr;
  private String userid;
  private int dayLimit;
  private String forward;
  private String realName;

  public Profile(String addr, String userid, int dayLimit, String forward, String realName) {
    this.addr = addr;
    this.userid = userid;
    this.dayLimit = dayLimit;
    this.forward = forward;
    this.realName = realName;
  }

  public String getRealName() {
    return realName;
  }

  public void setRealName(String realName) {
    this.realName = realName;
  }

  public String getAddr() {
    return addr;
  }

  public void setAddr(String addr) {
    this.addr = addr;
  }

  public String getUserid() {
    return userid;
  }

  public void setUserid(String userid) {
    this.userid = userid;
  }

  public int getDayLimit() {
    return dayLimit;
  }

  public void setDayLimit(int dayLimit) {
    this.dayLimit = dayLimit;
  }

  public String getForward() {
    return forward;
  }

  public void setForward(String forward) {
    this.forward = forward;
  }

  public Profile cloneProfile() {
    return new Profile(addr, userid, dayLimit, forward, realName);
  }

  public boolean equals(Object obj) {
    if (obj == null || !(obj instanceof Profile))
      return false;

    final Profile profile = (Profile)obj;
    return (profile.addr.equals(addr) && 
            profile.userid.equals(userid) &&
            profile.dayLimit == dayLimit &&
            profile.forward.equals(forward));
  }
}
