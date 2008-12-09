package mobi.eyeline.smsquiz.quizmanager.quiz;

import mobi.eyeline.smsquiz.distribution.Distribution;

import java.util.*;

/**
 * author: alkhal
 */

public class QuizData {

  private String destAddress;
  private String sourceAddress;
  private String question;
  private Date dateBegin;
  private Date dateEnd;
  private Calendar timeBegin;
  private Calendar timeEnd;
  private boolean txmode;
  private EnumSet<Distribution.WeekDays> days = EnumSet.noneOf(Distribution.WeekDays.class);
  private Date distrDateEnd;
  private int maxRepeat;
  private String defaultCategory;

  private List<ReplyPattern> replyPatterns = new LinkedList<ReplyPattern>();

  private String origAbFile;

  private String quizName;

  public String getDestAddress() {
    return destAddress;
  }

  public void setDestAddress(String destAddress) {
    this.destAddress = destAddress;
  }

  public String getSourceAddress() {
    return sourceAddress;
  }

  public void setSourceAddress(String sourceAddress) {
    this.sourceAddress = sourceAddress;
  }

  public String getQuestion() {
    return question;
  }

  public void setQuestion(String question) {
    this.question = question;
  }


  public Date getDateBegin() {
    return dateBegin;
  }

  public void setDateBegin(Date dateBegin) {
    this.dateBegin = dateBegin;
  }

  public Date getDateEnd() {
    return dateEnd;
  }

  public void setDateEnd(Date dateEnd) {
    this.dateEnd = dateEnd;
  }

  public Calendar getTimeBegin() {
    return timeBegin;
  }

  public void setTimeBegin(Calendar timeBegin) {
    this.timeBegin = timeBegin;
  }

  public Calendar getTimeEnd() {
    return timeEnd;
  }

  public void setTimeEnd(Calendar timeEnd) {
    this.timeEnd = timeEnd;
  }

  public boolean isTxmode() {
    return txmode;
  }

  public void setTxmode(boolean txmode) {
    this.txmode = txmode;
  }

  public Date getDistrDateEnd() {
    return distrDateEnd;
  }

  public void setDistrDateEnd(Date distrDateEnd) {
    this.distrDateEnd = distrDateEnd;
  }

  public int getMaxRepeat() {
    return maxRepeat;
  }

  public void setMaxRepeat(int maxRepeat) {
    this.maxRepeat = maxRepeat;
  }

  public String getDefaultCategory() {
    return defaultCategory;
  }

  public void setDefaultCategory(String defaultCategory) {
    this.defaultCategory = defaultCategory;
  }

  public void addReplyPattern(ReplyPattern replyPattern) {
    if (replyPattern != null) {
      replyPatterns.add(replyPattern);
    }
  }

  public void clearPatterns() {
    replyPatterns.clear();
  }

  public void addDay(Distribution.WeekDays weekDays) {
    days.add(weekDays);
  }

  public EnumSet<Distribution.WeekDays> getDays() {
    return days;
  }

  public List<ReplyPattern> getPatterns() {
    return replyPatterns;
  }

  public String getOrigAbFile() {
    return origAbFile;
  }

  public void setOrigAbFile(String origAbFile) {
    this.origAbFile = origAbFile;
  }

  public String getQuizName() {
    return quizName;
  }

  public void setQuizName(String quizName) {
    this.quizName = quizName;
  }

  public void setDays(EnumSet<Distribution.WeekDays> weekDays) {
    if (weekDays == null) {
      throw new IllegalArgumentException("Some arguments are null");
    }
    this.days = weekDays;
  }

  public void setReplyPatterns(List<ReplyPattern> replyPatterns) {
    if (replyPatterns == null) {
      throw new IllegalArgumentException("Some arguments are null");
    }
    this.replyPatterns = replyPatterns;
  }
}
