package mobi.eyeline.smsquiz.beans;

import ru.novosoft.smsc.util.config.Config;
import ru.novosoft.smsc.util.Functions;
import ru.novosoft.smsc.util.WebAppFolders;
import ru.novosoft.smsc.jsp.util.helper.Validation;
import ru.novosoft.smsc.jsp.util.helper.dynamictable.IncorrectValueException;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.util.jsp.MultipartServletRequest;
import ru.novosoft.util.jsp.MultipartDataSource;

import javax.servlet.http.HttpServletRequest;
import java.util.List;
import java.text.SimpleDateFormat;
import java.text.ParseException;
import java.io.File;
import java.io.IOException;
import java.io.InputStream;

import mobi.eyeline.smsquiz.quizes.adding.CategoriesTableHelper;
import mobi.eyeline.smsquiz.quizes.view.QuizFullData;
import mobi.eyeline.smsquiz.QuizBuilder;

/**
 * author: alkhal
 * Date: 10.11.2008
 */

public class QuizAdd extends SmsQuizBean{

  private String mbDone;
  private String mbCancel;
  private boolean initialized = false;
  private SimpleDateFormat dateFormat = new SimpleDateFormat("dd.MM.yyyy HH:mm");
  private SimpleDateFormat timeFormat = new SimpleDateFormat("HH:mm");
  private String[] activeWeekDays = new String[7];

  private final CategoriesTableHelper tableHelper = new CategoriesTableHelper("Categories","categories", 30, Validation.NON_EMPTY, true);

  private String quiz;

  private String dateBegin;

  private String dateEnd;

  private String abFile;

  private String question;

  private String timeBegin;

  private String timeEnd;

  private boolean txmode;

  private String sourceAddress;

  private String destAddress;

  private String maxRepeat;


  private String quizDir;

  private String defaultCategory;


  protected int init(List errors) {
    int result = super.init(errors);
    if(result!=RESULT_OK) {
      return result;
    }
    try{
      quizDir = getSmsQuizContext().getConfig().getString("quizmanager.dir.quiz");
    }
    catch(Exception e) {
      logger.error(e);
      e.printStackTrace();
      return RESULT_ERROR;
    }
    if(!initialized) {
      timeBegin="00:00";
      timeEnd = "23:59";
      txmode = false;
      maxRepeat = "3";
      activeWeekDays[0]="Mon";
    }
    return result;
  }

  public int process(HttpServletRequest request) {
    int result = super.process(request);
    if(result!=RESULT_OK) {
      return result;
    }
    MultipartServletRequest multi = (MultipartServletRequest)request.getAttribute("multipart.request");
    if(multi!=null) {

    }
    if (mbDone != null) result = save(multi);
    else if (mbCancel != null) result = RESULT_DONE;

    return result;
  }

  private int save(HttpServletRequest request) {
    System.out.println("Saving...");
    int result;
    if((result = validation(request))!=RESULT_OK) {
      return result;
    }
    QuizFullData data = new QuizFullData();
    MultipartDataSource ds = null;
    InputStream is = null;
    File file = null;
    try {
      ds= ((MultipartServletRequest)request.getAttribute("multipart.request")).getMultipartDataSource("file");
      is = ds.getInputStream();
      file = new File(quizDir+File.separator+quiz+".csv");
      file = Functions.saveFileToTemp(is, file);

      for(int j=0;j<activeWeekDays.length;j++) {
        if(activeWeekDays[j]!=null) {
          data.addActiveDay(activeWeekDays[j]);
        }
      }
      data.setDateBegin(dateBegin);
      data.setDateEnd(dateEnd);
      data.setDefaultCategory(defaultCategory);
      data.setDestAddress(destAddress);
      data.setMaxRepeat(maxRepeat);
      data.setQuestion(question);
      data.setQuiz(quiz);
      data.setSourceAddress(sourceAddress);
      data.setTimeBegin(timeBegin);
      data.setTimeEnd(timeEnd);
      data.setTxmode(Boolean.toString(txmode));
      data.setAbFile(file.getAbsolutePath());
      QuizBuilder.saveQuiz(data, quizDir+File.separator+quiz+".xml");
    } catch (Exception e) {
      logger.error(e);
      e.printStackTrace();
      return error(e.getMessage());
    } finally{
      if(ds!=null) {
        ds.close();
      }
      if(is!=null) {
        try {
          is.close();
        } catch (IOException e) {}
      }
    }

    return RESULT_DONE;
  }


  private int validation(HttpServletRequest request){
    System.out.println("Validation...");
    try{
      dateFormat.parse(dateBegin);
      dateFormat.parse(dateEnd);
      timeFormat.parse(timeBegin);
      timeFormat.parse(timeEnd);
    } catch (ParseException e) {
      logger.warn(e);
      e.printStackTrace();
      return warning(e.getMessage());
    }
    String path = quizDir + File.separator + quiz + ".xml";
    if((new File(path).exists())||(new File(path+".old").exists())) {
      System.out.println("Quiz with this name already exists");
      return warning("Quiz with this name already exists");
    }
    if((activeWeekDays==null)||(activeWeekDays.length==0)) {
      System.out.println("Please select one or more active days");
      return warning("Please select one or more active days");
    }

    return RESULT_OK;
  }

  public String getMbDone() {
    return mbDone;
  }

  public void setMbDone(String mbDone) {
    this.mbDone = mbDone;
  }

  public String getMbCancel() {
    return mbCancel;
  }

  public void setMbCancel(String mbCancel) {
    this.mbCancel = mbCancel;
  }

  public boolean isInitialized() {
    return initialized;
  }

  public void setInitialized(boolean initialized) {
    this.initialized = initialized;
  }

  public String getDateBegin() {
    return (dateBegin==null) ? "" : dateBegin;
  }

  public void setDateBegin(String dateBegin) {
    this.dateBegin = dateBegin;
  }

  public String getDateEnd() {
    return (dateEnd==null) ? "" : dateEnd;
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

  public String getTxmode() {
    return Boolean.toString(txmode);
  }

  public void setTxmode(String txmode) {

    this.txmode = Boolean.valueOf(txmode).booleanValue();
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


  public String[] getActiveWeekDays() {
    return activeWeekDays;
  }
  public void setActiveWeekDays(String[] activeWeekDays) {
    this.activeWeekDays = activeWeekDays;
  }

  public boolean isWeekDayActive(String weekday) {
    if(activeWeekDays!=null) {
    for(int i=0; i<activeWeekDays.length; i++)
      if ((activeWeekDays[i]!=null)&&(activeWeekDays[i].equals(weekday))) {
        return true;
      }                       }
    return false;
  }

  public String getActiveWeekDaysString()
  {
    String str = "";
    int total = activeWeekDays.length;
    if (total > 0) {
      int added=0;
      if (isWeekDayActive("Mon")) { str += "Monday";    if (++added < total) str += ", "; }
      if (isWeekDayActive("Tue")) { str += "Tuesday";   if (++added < total) str += ", "; }
      if (isWeekDayActive("Wed")) { str += "Wednesday"; if (++added < total) str += ", "; }
      if (isWeekDayActive("Thu")) { str += "Thursday";  if (++added < total) str += ", "; }
      if (isWeekDayActive("Fri")) { str += "Friday";    if (++added < total) str += ", "; }
      if (isWeekDayActive("Sat")) { str += "Saturday";  if (++added < total) str += ", "; }
      if (isWeekDayActive("Sun"))   str += "Sunday";
    }
    return str;
  }

  public String getQuiz() {
    return quiz;
  }

  public void setQuiz(String quiz) {
    this.quiz = quiz;
  }

  public String getDefaultCategory() {
    return defaultCategory;
  }

  public void setDefaultCategory(String defaultCategory) {
    if(defaultCategory.equals("")) {
      defaultCategory = null;
    }
    this.defaultCategory = defaultCategory;
  }

  public CategoriesTableHelper getTableHelper() {
    return tableHelper;
  }
}
