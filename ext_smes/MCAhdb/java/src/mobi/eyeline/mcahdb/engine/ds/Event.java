package mobi.eyeline.mcahdb.engine.ds;

import java.util.Date;

/**
 * User: artem
 * Date: 31.07.2008
 */

public interface Event {

  public Type getType();

  public Date getDate();

  public String getCaller();

  public String getCalled();

  public boolean isCalledProfileNotify();

  public boolean isCallerProfileWantNotifyMe();

  public enum Type {
    MissedCall, MissedCallAlert, MissedCallAlertFail
  }
}
