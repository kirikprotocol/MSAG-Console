package mobi.eyeline.smsquiz.replystats;

import ru.novosoft.smsc.jsp.util.tables.Filter;
import ru.novosoft.smsc.jsp.util.tables.DataItem;

import java.util.Date;

/**
 * author: alkhal
 * Date: 05.11.2008
 */
public class ReplyFilter implements Filter {

  private String address;

  private String quizPath;

  private String quizNumber;

  private Date dateBegin;

  private Date dateEnd;

  private Date quizDateBegin;

  private Date quizDateEnd;


  public boolean isEmpty() {
    return false;
  }

  public boolean isItemAllowed(DataItem item) {
    if((address !=null)&&(!item.getValue("msisdn").equals(address))) {
      return false;
    }
    if((getDateBegin() !=null)&&(!((Date)item.getValue("replyDate")).after(getDateBegin()))) {
      return false;
    }
    if((getDateEnd() !=null)&&(!((Date)item.getValue("replyDate")).before(getDateEnd()))) {
      return false;
    }
    return true;
  }

  public String getAddress() {
    return (address == null) ? "":address;
  }

  public void setAddress(String address) {
    this.address = address;
  }

  public String getQuizNumber() {
    return (quizNumber == null) ? "":quizNumber;
  }

  public void setQuizNumber(String quizNumber) {
    this.quizNumber = quizNumber;
  }

  public Date getDateBegin() {
    if(quizDateBegin==null)
      return null;
    if(dateBegin!=null) {
      if(dateBegin.after(quizDateBegin)) {
        return dateBegin;
      }
    }
    return quizDateBegin;
  }

  public void setDateBegin(Date dateBegin) {
    this.dateBegin = dateBegin;
  }

  public Date getDateEnd() {
    if(quizDateEnd==null)
      return null;
    if(dateEnd!=null) {
      if(dateEnd.before(quizDateEnd)) {
        return dateEnd;
      }
    }
    return quizDateEnd;
  }

  public void setDateEnd(Date dateEnd) {
    this.dateEnd = dateEnd;
  }

  public String getQuizPath() {
    return (quizPath!=null) ? quizPath : "";
  }

  public void setQuizPath(String quizPath) {
    this.quizPath = quizPath;
  }

  public Date getQuizDateBegin() {
    return quizDateBegin;
  }

  public void setQuizDateBegin(Date quizDateBegin) {
    this.quizDateBegin = quizDateBegin;
  }

  public Date getQuizDateEnd() {
    return quizDateEnd;
  }

  public void setQuizDateEnd(Date quizDateEnd) {
    this.quizDateEnd = quizDateEnd;
  }
}
