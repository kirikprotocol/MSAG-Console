package mobi.eyeline.informer.admin.delivery.stat;

import java.util.Date;

/**
 * @author Artem Snopkov
 */
public class UserStatRecord {

  private Date date;
  private String user;
  private int paused;
  private int planned;
  private int active;
  private int finish;
  private int cancel;
  private int created;
  private int deleted;

  UserStatRecord() {
    
  }

  public Date getDate() {
    return date;
  }

  void setDate(Date date) {
    this.date = date;
  }

  public String getUser() {
    return user;
  }

  void setUser(String user) {
    this.user = user;
  }

  public int getPaused() {
    return paused;
  }

  void setPaused(int paused) {
    this.paused = paused;
  }

  public int getPlanned() {
    return planned;
  }

  void setPlanned(int planned) {
    this.planned = planned;
  }

  public int getActive() {
    return active;
  }

  void setActive(int active) {
    this.active = active;
  }

  public int getFinish() {
    return finish;
  }

  void setFinish(int finish) {
    this.finish = finish;
  }

  public int getCancel() {
    return cancel;
  }

  void setCancel(int cancel) {
    this.cancel = cancel;
  }

  public int getCreated() {
    return created;
  }

  void setCreated(int created) {
    this.created = created;
  }

  public int getDeleted() {
    return deleted;
  }

  void setDeleted(int deleted) {
    this.deleted = deleted;
  }
}
