package mobi.eyeline.mcahdb.engine.journal.ds;

import java.util.Date;

/**
 * User: artem
 * Date: 01.09.2008
 */

public class JournalEvent {

  private Type type;
  private Date date;
  private String caller;
  private String called;
  private boolean calledProfileNotify;
  private boolean callerProfileWantNotifyMe;

  public Type getType() {
    return type;
  }

  public void setType(Type type) {
    this.type = type;
  }

  public Date getDate() {
    return date;
  }

  public void setDate(Date date) {
    this.date = date;
  }

  public String getCaller() {
    return caller;
  }

  public void setCaller(String caller) {
    this.caller = caller;
  }

  public String getCalled() {
    return called;
  }

  public void setCalled(String called) {
    this.called = called;
  }

  public boolean isCalledProfileNotify() {
    return calledProfileNotify;
  }

  public void setCalledProfileNotify(boolean calledProfileNotify) {
    this.calledProfileNotify = calledProfileNotify;
  }

  public boolean isCallerProfileWantNotifyMe() {
    return callerProfileWantNotifyMe;
  }

  public void setCallerProfileWantNotifyMe(boolean callerProfileWantNotifyMe) {
    this.callerProfileWantNotifyMe = callerProfileWantNotifyMe;
  }

  public enum Type {
    MissedCall, MissedCallAlert, MissedCallAlertFail, MissedCallRemove, ProfileChaged
  }
}
