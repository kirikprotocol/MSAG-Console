package mobi.eyeline.smsquiz.quizes.view;

import mobi.eyeline.smsquiz.quizes.AnswerCategory;

import java.util.Date;
import java.util.List;
import java.util.LinkedList;
import java.util.Iterator;

/**
 * author: alkhal
 * Date: 10.11.2008
 */
public class QuizFullData {

  private String quiz;

  private String activeDays="";

  private String dateBegin;

  private String dateEnd;

  private String abFile;

  private String question;

  private String timeBegin;

  private String timeEnd;

  private String txmode;

  private String sourceAddress;

  private String destAddress;

  private String maxRepeat;

  private String defaultCategory="";

  private List categories = new LinkedList();


  public String getDateBegin() {
    return dateBegin;
  }

  public void setDateBegin(String dateBegin) {
    this.dateBegin = dateBegin;
  }

  public String getDateEnd() {
    return dateEnd;
  }

  public void setDateEnd(String dateEnd) {
    this.dateEnd = dateEnd;
  }

  public String getAbFile() {
    return abFile;
  }

  public void setAbFile(String abFile) {
    this.abFile = abFile;
  }

  public String getQuestion() {
    return question;
  }

  public void setQuestion(String question) {
    this.question = question;
  }

  public String getActiveDays() {
    return activeDays;
  }

  public void setActiveDays(String activeDays) {
    this.activeDays = activeDays;
  }

  public String getTimeBegin() {
    return timeBegin;
  }

  public void setTimeBegin(String timeBegin) {
    this.timeBegin = timeBegin;
  }

  public String getTimeEnd() {
    return timeEnd;
  }

  public void setTimeEnd(String timeEnd) {
    this.timeEnd = timeEnd;
  }

  public void addActiveDay(String day) {
    if((day!=null)&&(!day.equals(""))) {
      if(!activeDays.equals("")) {
        activeDays+=",";
      }
      activeDays+=day;
    }
  }

  public String getTxmode() {
    return txmode;
  }

  public void setTxmode(String txmode) {
    this.txmode = txmode;
  }

  public String getSourceAddress() {
    return sourceAddress;
  }

  public void setSourceAddress(String sourceAddress) {
    this.sourceAddress = sourceAddress;
  }

  public String getDestAddress() {
    return destAddress;
  }

  public void setDestAddress(String destAddress) {
    this.destAddress = destAddress;
  }

  public String getMaxRepeat() {
    return maxRepeat;
  }

  public void setMaxRepeat(String maxRepeat) {
    this.maxRepeat = maxRepeat;
  }

  public String getDefaultCategory() {
    return defaultCategory;
  }

  public void setDefaultCategory(String defaultCategory) {
    this.defaultCategory = defaultCategory;
  }

  public void addCategory(AnswerCategory cat) {
    if(cat!=null) {
      categories.add(cat);
    }
  }

  public Iterator getCategories() {
    return categories.iterator();
  }

  public String getQuiz() {
    return quiz;
  }

  public void setQuiz(String quiz) {
    this.quiz = quiz;
  }

}
