package mobi.eyeline.mcahdb.engine.scheduler.ds;

import java.util.Date;

/**
 * User: artem
 * Date: 27.08.2008
 */

public class Task {

  public static final int TYPE_EXPIRED_NOTIFY = 0;

  private final String caller;
  private final String called;

  private Date time;
  private int type;

  private Object id;

  public Task(String caller, String called) {
    this.caller = caller;
    this.called = called;
  }

  public String getCaller() {
    return caller;
  }

  public String getCalled() {
    return called;
  }

  public Date getTime() {
    return time;
  }

  public void setTime(Date time) {
    this.time = time;
  }

  public int getType() {
    return type;
  }

  public void setType(int type) {
    this.type = type;
  }

  public Object getId() {
    return id;
  }

  public void setId(Object id) {
    this.id = id;
  }
}
