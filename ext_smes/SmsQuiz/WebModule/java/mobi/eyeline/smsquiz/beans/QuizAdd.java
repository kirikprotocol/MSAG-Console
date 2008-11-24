package mobi.eyeline.smsquiz.beans;

import ru.novosoft.smsc.jsp.util.helper.Validation;
import ru.novosoft.smsc.jsp.util.tables.QueryResultSet;
import ru.novosoft.smsc.jsp.util.tables.DataItem;
import ru.novosoft.util.jsp.MultipartServletRequest;
import ru.novosoft.util.jsp.MultipartDataSource;

import javax.servlet.http.HttpServletRequest;
import java.util.List;
import java.util.Collection;
import java.util.Iterator;
import java.util.Date;
import java.text.SimpleDateFormat;
import java.text.ParseException;
import java.io.*;

import mobi.eyeline.smsquiz.quizes.CategoriesTableHelper;
import mobi.eyeline.smsquiz.quizes.AnswerCategory;
import mobi.eyeline.smsquiz.quizes.view.QuizFullData;
import mobi.eyeline.smsquiz.quizes.view.QuizesDataSource;
import mobi.eyeline.smsquiz.quizes.view.QuizQuery;
import mobi.eyeline.smsquiz.QuizBuilder;

/**
 * author: alkhal
 * Date: 10.11.2008
 */

public class QuizAdd extends SmsQuizBean {

  private String mbDone;
  private String mbCancel;
  private boolean initialized = false;
  private SimpleDateFormat dateFormat = new SimpleDateFormat("dd.MM.yyyy HH:mm");
  private SimpleDateFormat timeFormat = new SimpleDateFormat("HH:mm");
  private String[] activeWeekDays = new String[7];

  private final CategoriesTableHelper tableHelper = new CategoriesTableHelper("smsquiz.label.category", "categories", 30, Validation.NON_EMPTY, true);

  private String quizName;

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

  private String distrDateEnd;


  protected int init(List errors) {
    int result = super.init(errors);
    if (result != RESULT_OK) {
      return result;
    }
    try {
      quizDir = getSmsQuizContext().getConfig().getString("quizmanager.dir.quiz");
    }
    catch (Exception e) {
      logger.error(e);
      e.printStackTrace();
      return RESULT_ERROR;
    }
    if (!initialized) {
      timeBegin = "00:00:00";
      timeEnd = "23:59:00";
      txmode = false;
      maxRepeat = "3";
      activeWeekDays[0] = "Mon";
    }
    return result;
  }

  public int process(HttpServletRequest request) {
    int result = super.process(request);
    if (result != RESULT_OK) {
      return result;
    }
    MultipartServletRequest multi = (MultipartServletRequest) request.getAttribute("multipart.request");
    if (mbDone != null) result = save(multi);
    else if (mbCancel != null) result = RESULT_DONE;

    return result;
  }

  private int save(MultipartServletRequest request) {
    System.out.println("Saving...");
    int result;
    if ((result = validation(request)) != RESULT_OK) {
      return result;
    }
    QuizFullData data = new QuizFullData();
    MultipartDataSource ds = null;
    InputStream is = null;
    BufferedOutputStream outputStream = null;
    File file;
    try {
      ds = request.getMultipartDataSource("file");
      is = ds.getInputStream();

      String quizId = Integer.toString(getUniqName());
      file = new File(quizDir + File.separator + quizId + ".csv");
      outputStream = new BufferedOutputStream(new FileOutputStream(file));
      byte buffer[] = new byte[2048];
      boolean begin = true;
      for (int readed; (readed = is.read(buffer)) > -1;) {
        if (begin && (readed <= -1)) {
          return warning("Abonent's file is empty");
        }
        begin = false;
        outputStream.write(buffer, 0, readed);
      }

      for (int j = 0; j < activeWeekDays.length; j++) {
        if (activeWeekDays[j] != null) {
          data.addActiveDay(activeWeekDays[j]);
        }
      }
      List categories = tableHelper.getCategories();
      data.setCategory(categories);
      data.setDateBegin(dateBegin);
      data.setDateEnd(dateEnd);
      data.setDefaultCategory(defaultCategory);
      data.setDestAddress(destAddress);
      data.setMaxRepeat(maxRepeat);
      data.setQuestion(question);
      data.setName(quizName);
      data.setSourceAddress(sourceAddress);
      data.setTimeBegin(timeBegin);
      data.setTimeEnd(timeEnd);
      data.setTxmode(Boolean.toString(txmode));
      data.setAbFile(file.getAbsolutePath());
      data.setDistrDateEnd(distrDateEnd);
      QuizBuilder.saveQuiz(data, quizDir + File.separator + quizId + ".xml");
    } catch (Exception e) {
      logger.error(e);
      e.printStackTrace();
      return error(e.getMessage());
    } finally {
      if (ds != null) {
        ds.close();
      }
      if (is != null) {
        try {
          is.close();
        } catch (IOException e) {
        }
      }
      if (outputStream != null) {
        try {
          outputStream.close();
        } catch (IOException e) {
        }
      }
    }

    return RESULT_DONE;
  }


  private int validation(MultipartServletRequest request) {
    System.out.println("Validation...");
    try {
      dateFormat.parse(dateBegin);
      dateFormat.parse(dateEnd);
      dateFormat.parse(distrDateEnd);
      timeFormat.parse(timeBegin);
      timeFormat.parse(timeEnd);
    } catch (ParseException e) {
      logger.warn(e);
      e.printStackTrace();
      return warning(e.getMessage());
    }
    if (!validateQuizName()) {
      System.out.println("Quiz with this name already exists");
      return warning("Quiz with this name already exists");
    }
    if ((activeWeekDays == null) || (activeWeekDays.length == 0)) {
      System.out.println("Please select one or more active days");
      return warning("Please select one or more active days");
    }
    if ((tableHelper.getCategories() == null) || (tableHelper.getCategories().size() == 0)) {
      System.out.println("Please select one or more answer's categories");
      return warning("Please select one or more answer's categories");
    }
    if((defaultCategory!=null)&&(!"".equals(defaultCategory))) {
      Collection categories = tableHelper.getCategories();
      Iterator iter = categories.iterator();
      int flag=0;
      while(iter.hasNext()) {
        AnswerCategory cat = (AnswerCategory)iter.next();
        if(cat.getName().equals(defaultCategory)) {
          flag=1;
          break;
        }
      }
      if(flag==0) {
        return warning("Default category must be included in answer's categories");
      }
    }
    try {
      if (request.getMultipartDataSource("file") == null) {
        System.out.println("File data source is null, select the file");
        return warning("Please select abonents file");
      }
    } catch (IOException e) {
      e.printStackTrace();
      logger.error(e);
      return warning("Error during upload abonents file");
    }

    return RESULT_OK;
  }

  private boolean validateQuizName() {
    QuizesDataSource ds = new QuizesDataSource(quizDir);
    QueryResultSet quizesList = ds.query(new QuizQuery(1000, QuizesDataSource.QUIZ_NAME, 0));
    for (int i = 0; i < quizesList.size(); i++) {
      final DataItem item = quizesList.get(i);
      final String quizName = (String) item.getValue(QuizesDataSource.QUIZ_NAME);
      System.out.println("Compare "+this.quizName+" with "+quizName);
      if(quizName.equalsIgnoreCase(this.quizName)) {
        return false;
      }
    }
    return true;
  }

  private int getUniqName() {
    File dir = new File(quizDir);
    File[] files = dir.listFiles(new FilenameFilter() {
      public boolean accept(File dir, String name) {
        return name.endsWith(".xml") || name.endsWith(".xml.old");
      }
    });
    int result = 1;
    for(int i=0;i<files.length;i++) {
      String filename = files[i].getName();
      int subRes = 0;
      filename = filename.substring(0,filename.indexOf("."));
      try{
        subRes = Integer.parseInt(filename);
        System.out.println(result+" compare with file's number: "+subRes);
        if(result<=subRes) {
          result=subRes+1;
        }
      } catch(NumberFormatException e) {}
    }
    return result;
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
    return (dateBegin == null) ? "" : dateBegin;
  }

  public void setDateBegin(String dateBegin) {
    this.dateBegin = dateBegin;
  }

  public String getDateEnd() {
    return (dateEnd == null) ? "" : dateEnd;
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

  public void setTxmode(boolean txmode) {

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


  public String[] getActiveWeekDays() {
    return activeWeekDays;
  }

  public void setActiveWeekDays(String[] activeWeekDays) {
    this.activeWeekDays = activeWeekDays;
  }

  public boolean isWeekDayActive(String weekday) {
    if (activeWeekDays != null) {
      for (int i = 0; i < activeWeekDays.length; i++)
        if ((activeWeekDays[i] != null) && (activeWeekDays[i].equals(weekday))) {
          return true;
        }
    }
    return false;
  }

  public String getActiveWeekDaysString() {
    String str = "";
    int total = activeWeekDays.length;
    if (total > 0) {
      int added = 0;
      if (isWeekDayActive("Mon")) {
        str += "Monday";
        if (++added < total) str += ", ";
      }
      if (isWeekDayActive("Tue")) {
        str += "Tuesday";
        if (++added < total) str += ", ";
      }
      if (isWeekDayActive("Wed")) {
        str += "Wednesday";
        if (++added < total) str += ", ";
      }
      if (isWeekDayActive("Thu")) {
        str += "Thursday";
        if (++added < total) str += ", ";
      }
      if (isWeekDayActive("Fri")) {
        str += "Friday";
        if (++added < total) str += ", ";
      }
      if (isWeekDayActive("Sat")) {
        str += "Saturday";
        if (++added < total) str += ", ";
      }
      if (isWeekDayActive("Sun")) str += "Sunday";
    }
    return str;
  }

  public String getDefaultCategory() {
    return defaultCategory;
  }

  public void setDefaultCategory(String defaultCategory) {
    if (defaultCategory.equals("")) {
      defaultCategory = null;
    }
    this.defaultCategory = defaultCategory;
  }

  public CategoriesTableHelper getTableHelper() {
    return tableHelper;
  }

  public String getDistrDateEnd() {
    return distrDateEnd;
  }

  public void setDistrDateEnd(String distrDateEnd) {
    this.distrDateEnd = distrDateEnd;
  }

  public boolean isTxmode() {
    return txmode;
  }

  public String getQuizName() {
    return quizName;
  }

  public void setQuizName(String quizName) {
    this.quizName = quizName;
  }
}
