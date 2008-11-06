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

  private boolean dateBeginEnabled = false;

  private boolean dateEndEnabled = false;


  public boolean isEmpty() {
    return false;
  }

  public boolean isItemAllowed(DataItem item) {
    if((address !=null)&&(!item.getValue("msisdn").equals(address))) {
      return false;
    }
    if(dateBeginEnabled) {
      if(!((Date)item.getValue("replyDate")).after(dateBegin)) {
        return false;
      }
    } else {
      if(!((Date)item.getValue("replyDate")).after(quizDateBegin)) {
        return false;
      }
    }
    if(dateEndEnabled) {
      if(!((Date)item.getValue("replyDate")).before(dateEnd)) {
        return false;
      }
    } else {
      if(!((Date)item.getValue("replyDate")).before(quizDateEnd)) {
        return false;
      }
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
    return dateBeginEnabled ? dateBegin:null;
  }

  public void setDateBegin(Date dateBegin) {
    this.dateBegin = dateBegin;
  }

  public Date getDateEnd() {
    return dateEndEnabled ? dateEnd:null;
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

  public boolean isDateBeginEnabled() {
    return dateBeginEnabled;
  }

  public void setDateBeginEnabled(boolean dateBeginEnabled) {
    this.dateBeginEnabled = dateBeginEnabled;
  }

  public boolean isDateEndEnabled() {
    return dateEndEnabled;
  }

  public void setDateEndEnabled(boolean dateEndEnabled) {
    this.dateEndEnabled = dateEndEnabled;
  }
}
