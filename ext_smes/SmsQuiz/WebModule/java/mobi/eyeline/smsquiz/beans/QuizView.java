package mobi.eyeline.smsquiz.beans;

import mobi.eyeline.smsquiz.quizes.view.QuizData;
import mobi.eyeline.smsquiz.quizes.CategoriesTableHelper;
import mobi.eyeline.smsquiz.quizes.AnswerCategory;
import mobi.eyeline.smsquiz.QuizBuilder;

import javax.servlet.http.HttpServletRequest;
import java.util.*;
import java.io.*;

import ru.novosoft.smsc.jsp.util.helper.Validation;

/**
 * author: alkhal
 * Date: 10.11.2008
 */
public class QuizView extends SmsQuizBean {

  private QuizData quizData = new QuizData();

  public static final int RESULT_DONE = SmsQuizBean.PRIVATE_RESULT + 1;

  public static final int RESULT_EDIT = SmsQuizBean.PRIVATE_RESULT + 2;

  private final CategoriesTableHelper tableHelper = new CategoriesTableHelper("smsquiz.label.category", "categories", 30, Validation.NON_EMPTY, true);

  private static String datePattern = "dd.MM.yyyy HH:mm";

  private String quizId;

  private String quizDir;

  private String mbDone = null;

  private String mbCancel = null;

  private String destAddress;

  private String maxRepeat;

  private String defaultCategory;

  private String status;

  private String reasonId;

  private String reason;

  private boolean initialized = false;

  protected int init(List errors) {
    int result = super.init(errors);
    if (result != RESULT_OK) {
      return result;
    }
    if((quizId ==null)||("".equals(quizId))) {
      return RESULT_DONE;
    }
    try {
      quizDir = getSmsQuizContext().getConfig().getString("quizmanager.dir_quiz");
      String workDir = getSmsQuizContext().getConfig().getString("quizmanager.dir_work");
      result = readStatus(workDir);
      if (result != RESULT_OK) {
        return result;
      }
    } catch (Exception e) {
      logger.error(e);
      e.printStackTrace();
      result = error(e.getMessage());
    }
    if (!buildData()) {
      logger.error("Can't read quiz: quizId doesn't exist with id=" + quizId);
      System.out.println("Can't read quiz: quizId doesn't exist with id=" + quizId);
      return error("Can't read quiz: quizId doesn't exist with id=" + quizId);
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
      result = save();
    }
    if(mbCancel !=null) {
      mbCancel = null;
      return RESULT_DONE;
    }
    return result;
  }

  public boolean isActive(){
    if(quizData!=null) {
        Date dateBegin = quizData.getDateBegin();
        Date dateEnd = quizData.getDateEnd();
        Date now = new Date();
        return dateBegin.before(now)&&dateEnd.after(now);
    }
    return false;
  }

  public boolean isFinished() {
    if(quizData!=null) {
        Date dateEnd = quizData.getDateEnd();
        Date now = new Date();
        return now.after(dateEnd);
    }
    return false;
  }

  private int save() {
    if(isFinished()) {
      return RESULT_DONE;
    }
    int result;
    if ((result = validation()) != RESULT_OK) {
      return result;
    }
    if(!isChanged()) {
      return RESULT_DONE;
    }
    quizData.setMaxRepeat(maxRepeat);
    quizData.setDefaultCategory(defaultCategory);
    if(!isActive()) {
      quizData.setCategory(tableHelper.getCategories());
      quizData.setDestAddress(destAddress);
    }
    QuizBuilder.saveQuiz(quizData,quizDir+File.separator+quizId +".xml");
    return RESULT_DONE;
  }

  private boolean isChanged() {
    if((!maxRepeat.equals(quizData.getMaxRepeat()))
        ||((defaultCategory!=null)&&(quizData.getDefaultCategory()==null))
        ||((defaultCategory==null)&&(quizData.getDefaultCategory()!=null))
        ||((defaultCategory!=null)&&(quizData.getDefaultCategory()!=null)
            &&(!defaultCategory.equals(quizData.getDefaultCategory())))) {
      return true;
    }
    Collection newCat = tableHelper.getCategories();
    if(newCat==null) {
      return false;
    }
    Collection oldCat = quizData.getCategories();
    if(newCat.size()!=oldCat.size()) {
      return true;
    }
    Iterator newIter = newCat.iterator();
    while(newIter.hasNext()) {
      AnswerCategory cat1 = (AnswerCategory)newIter.next();
      Iterator oldIter = oldCat.iterator();
      int flag = 0;
      while(oldIter.hasNext()) {
        AnswerCategory cat2 = (AnswerCategory)oldIter.next();
        if(cat2.getName().equals(cat1.getName())) {
          flag = 1;
          if((!cat2.getPattern().equals(cat1.getPattern()))
              ||(!cat2.getAnswer().equals(cat1.getAnswer()))) {
            return true;
          }
          else {
            break;
          }
        }
      }
      if(flag==0) {
        return true;
      }
    }
    return false;
  }



  private int validation() {

    if ((!isActive())&&((tableHelper.getCategories() == null) || (tableHelper.getCategories().size() == 0))) {
      System.out.println("Please select one or more answer's categories");
      return warning("Please select one or more answer's categories");
    }
    if((defaultCategory!=null)&&(!"".equals(defaultCategory))) {
      Collection categories = null;
      if(!isActive()) {
        categories = tableHelper.getCategories();
      } else {
        categories = quizData.getCategories();
      }
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
    return RESULT_OK;
  }


  private boolean buildData() {
    if (quizId == null) {
      return false;
    }
    String path = quizDir + File.separator + quizId + ".xml";
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
  private int readStatus(String dirWork) {
    InputStream is = null;
    try {
      is = new FileInputStream(dirWork + File.separator + quizId + ".status");
      Properties prop = new Properties();
      prop.load(is);
      status = prop.getProperty("quiz.status");
      reason = prop.getProperty("quiz.error.reason");
      reasonId = prop.getProperty("quiz.error.id");
      return RESULT_OK;
    } catch (FileNotFoundException e) {
      logger.warn(e,e);
      status = "NEW";
      return RESULT_OK;
    } catch (IOException e) {
      logger.error(e,e);
      return error(e.toString());
    } finally{
      if(is!=null) {
        try {
          is.close();
        } catch (IOException e) {}
      }
    }
  }
  public QuizData getQuizData() {
    return quizData;
  }

  public String getQuizId() {
    return quizId;
  }

  public void setQuizId(String quizId) {
    this.quizId = quizId;
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

  public String getStatus() {
    return status;
  }

  public void setStatus(String status) {
    this.status = status;
  }

  public String getReasonId() {
    return reasonId;
  }

  public void setReasonId(String reasonId) {
    this.reasonId = reasonId;
  }

  public String getReason() {
    return reason;
  }

  public void setReason(String reason) {
    this.reason = reason;
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
}
