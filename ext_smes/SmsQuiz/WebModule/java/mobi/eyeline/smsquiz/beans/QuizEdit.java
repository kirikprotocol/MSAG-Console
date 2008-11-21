package mobi.eyeline.smsquiz.beans;

import mobi.eyeline.smsquiz.quizes.view.QuizFullData;
import mobi.eyeline.smsquiz.quizes.CategoriesTableHelper;
import mobi.eyeline.smsquiz.QuizBuilder;

import javax.servlet.http.HttpServletRequest;
import java.util.List;
import java.util.Date;
import java.io.File;
import java.text.SimpleDateFormat;
import java.text.ParseException;

import ru.novosoft.smsc.jsp.util.helper.Validation;

/**
 * author: alkhal
 * Date: 21.11.2008
 */
public class QuizEdit extends SmsQuizBean{
  private QuizFullData quizData = new QuizFullData();

  public static final int RESULT_DONE = SmsQuizBean.PRIVATE_RESULT + 1;

  private final CategoriesTableHelper tableHelper = new CategoriesTableHelper("smsquiz.label.category", "categories", 30, Validation.NON_EMPTY, true);

  private static String datePattern = "dd.MM.yyyy HH:mm";

  private String quiz;

  private String quizDir;

  private String mbDone = null;

  private String mbEdit = null;

  private String mbCancel = null;

  private String destAddress;

  private String maxRepeat;

  private String defaultCategory;

  private boolean initialized = false;

  protected int init(List errors) {
    int result = super.init(errors);
    if (result != RESULT_OK)
      return result;

    if((quiz==null)||("".equals(quiz))) {
      return error("Wrong request");
    }

    try {
      quizDir = getSmsQuizContext().getConfig().getString("quizmanager.dir.quiz");
    } catch (Exception e) {
      logger.error(e);
      e.printStackTrace();
      result = error(e.getMessage());
    }



    if (!buildData()) {
      logger.error("Can't read quiz: " + quiz);
      System.out.println("Can't read quiz: " + quiz);
      return error("Can't read quiz: " + quiz);
    }

    if(isFinished()) {
      return RESULT_DONE;
    }

    if(!initialized) {
      defaultCategory = quizData.getDefaultCategory();
      maxRepeat = quizData.getMaxRepeat();
      if(!isActive()) {
        destAddress = quizData.getDestAddress();
        tableHelper.fillCategories(quizData.getCategories());
      }
    }
    return result;
  }

  public int process(HttpServletRequest request) {
    int result = super.process(request);
    if (result != RESULT_OK) {
      return result;
    }

    if(mbDone != null) {
      mbDone = null;
      return save();
    }
    if (mbCancel != null) {
      result = RESULT_DONE;
    }
    return result;
  }


  private int save() {
    int result;
    if ((result = validation()) != RESULT_OK) {
      return result;
    }
    quizData.setMaxRepeat(maxRepeat);
    quizData.setDefaultCategory(defaultCategory);
    if(!isActive()) {
      quizData.setCategory(tableHelper.getCategories());
      quizData.setDestAddress(destAddress);
    }
    QuizBuilder.saveQuiz(quizData,quizDir+File.separator+quiz+".xml");
    return RESULT_DONE;
  }

  private int validation() {

    if ((!isActive())&&((tableHelper.getCategories() == null) || (tableHelper.getCategories().size() == 0))) {
      System.out.println("Please select one or more answer's categories");
      return warning("Please select one or more answer's categories");
    }

    return RESULT_OK;
  }

  private boolean buildData() {
    if (quiz == null) {
      return false;
    }
    String path = quizDir + File.separator + quiz + ".xml";
    File file = new File(path);
    if (!file.exists()) {
      file = new File(path + ".old");
    }
    if (!file.exists()) {
      return false;
    }
    try {
      quizData = QuizBuilder.parseAll(file.getAbsolutePath());
    } catch (Exception e) {
      logger.error(e);
      e.printStackTrace();
      return false;
    }
    return true;
  }

  public boolean isActive(){
    SimpleDateFormat format = new SimpleDateFormat(datePattern);
    if(quizData!=null) {
      try{
        Date dateBegin = format.parse(quizData.getDateBegin());
        Date dateEnd = format.parse(quizData.getDateEnd());
        Date now = new Date();
        return dateBegin.before(now)&&dateEnd.after(now);
      }catch(Exception e) {
        logger.error(e,e);
      }
    }
    return false;
  }

  private boolean isFinished() {
    SimpleDateFormat format = new SimpleDateFormat(datePattern);
    if(quizData!=null) {
      try{
        Date dateEnd = format.parse(quizData.getDateEnd());
        Date now = new Date();
        return now.after(dateEnd);
      }catch(Exception e) {
        logger.error(e,e);
      }
    }
    return false;
  }

  public QuizFullData getQuizData() {
    return quizData;
  }

  public String getQuiz() {
    return quiz;
  }

  public void setQuiz(String quiz) {
    this.quiz = quiz;
  }

  public String getMbDone() {
    return mbDone;
  }

  public void setMbDone(String mbDone) {
    this.mbDone = mbDone;
  }

  public String getMbEdit() {
    return mbEdit;
  }

  public void setMbEdit(String mbEdit) {
    this.mbEdit = mbEdit;
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

  public CategoriesTableHelper getTableHelper() {
    return tableHelper;
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
}
