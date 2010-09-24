package mobi.eyeline.informer.admin.journal;

import java.util.Date;

/**
 * Фильтр извлечения записей журнала
 * @author Aleksandr Khalitov
 */
public class JournalFilter {

  protected Date startDate;

  protected Date endDate;

  protected Subject subject;

  protected String user;

  public String getUser() {
    return user;
  }

  public JournalFilter setUser(String user) {
    this.user = user;
    return this;
  }

  public Subject getSubject() {
    return subject;
  }

  public JournalFilter setSubject(Subject subject) {
    this.subject = subject;
    return this;
  }

  public Date getStartDate() {
    return startDate;
  }

  public JournalFilter setStartDate(Date startDate) {
    this.startDate = startDate;
    return this;
  }

  public Date getEndDate() {
    return endDate;
  }

  public JournalFilter setEndDate(Date endDate) {
    this.endDate = endDate;
    return this;
  }

  /**
   * Показывать запись или нет
   * @param r запись
   * @return true - показывать запись, false - нет
   */
  @SuppressWarnings({"RedundantIfStatement"})
  protected boolean accept(JournalRecord r) {
    if(subject != null && !subject.equals(r.getSubject())){
      return false;      
    }
    if(user != null && !user.equals(r.getUser())){
      return false;      
    }
    if(startDate != null && startDate.getTime()>r.getTime()){
      return false;      
    }
    if(endDate != null && endDate.getTime()<r.getTime()){
      return false;      
    }
    return true;    
  }
}
