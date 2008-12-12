package mobi.eyeline.smsquiz.beans;

import mobi.eyeline.smsquiz.quizes.view.QuizData;
import mobi.eyeline.smsquiz.quizes.view.QuizDataItem;
import mobi.eyeline.smsquiz.quizes.CategoriesTableHelper;
import mobi.eyeline.smsquiz.quizes.AnswerCategory;
import mobi.eyeline.smsquiz.QuizBuilder;
import mobi.eyeline.smsquiz.DistributionHelper;

import javax.servlet.http.HttpServletRequest;
import java.util.*;
import java.io.*;
import java.text.SimpleDateFormat;
import java.text.ParseException;

import ru.novosoft.smsc.jsp.util.helper.Validation;

/**
 * author: alkhal
 * Date: 10.11.2008
 */
public class QuizView extends SmsQuizBean {

  private QuizData quizData = new QuizData();

  public static final int RESULT_DONE = SmsQuizBean.PRIVATE_RESULT + 1;

  public static final int RESULT_EDIT = SmsQuizBean.PRIVATE_RESULT + 2;

  private final CategoriesTableHelper tableHelper = new CategoriesTableHelper("smsquiz.label.category", "categories", 70, Validation.NON_EMPTY, true);

  private SimpleDateFormat dateFormat = new SimpleDateFormat("dd.MM.yyyy HH:mm");
  private SimpleDateFormat timeFormat = new SimpleDateFormat("HH:mm");

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

  DistributionHelper distributionHelper = new DistributionHelper("SmsQuiz");

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
      String[] activeWeekDays = new String[quizData.getActiveDays().size()];
      for(int i=0;i<activeWeekDays.length;i++) {
        activeWeekDays[i] =((String)quizData.getActiveDays().get(i)).substring(0);
        System.out.println("Write "+activeWeekDays[i]);
      }
      distributionHelper.setActiveWeekDays(activeWeekDays);
      distributionHelper.setTimeBegin(quizData.getTimeBegin());
      distributionHelper.setTimeEnd(quizData.getTimeEnd());
      distributionHelper.setTxmode(quizData.getTxmode());
      distributionHelper.setSourceAddress(quizData.getSourceAddress());
      distributionHelper.setDistrDateEnd(quizData.getDistrDateEndStr());
    }
    return result;
  }

  public int process(HttpServletRequest request) {
    int result = super.process(request);
    if (result != RESULT_OK) {
      return result;
    }
    if(initialized) {
      distributionHelper.processRequest(request);
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
    if(status!=null) {
        return status.equals(QuizDataItem.State.ACTIVE);
    }
    return false;
  }

  public boolean isFinished() {
    if(status!=null) {
      return status.equals(QuizDataItem.State.FINISHED.getName())||status.equals(QuizDataItem.State.FINISHED_ERROR.getName());

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
    try {
      if(distributionHelper.getDistrDateEnd()!=null && !distributionHelper.getDistrDateEnd().equals("")) {
        quizData.setDistrDateEnd(dateFormat.parse(distributionHelper.getDistrDateEnd()));
      }
    } catch (ParseException e) {
      e.printStackTrace();
      logger.error(e,e);
      return warning(e.getMessage());
    }

    try{
      quizData.setTimeBegin(distributionHelper.getTimeBegin());
      quizData.setTimeEnd(distributionHelper.getTimeEnd());
      quizData.setSourceAddress(distributionHelper.getSourceAddress());
      quizData.setActiveDays(distributionHelper.getActiveWeekDays());
      quizData.setTxmode(Boolean.toString(distributionHelper.isTxmode()));

      QuizBuilder.saveQuiz(quizData,quizDir+File.separator+quizId +".xml");
    }catch(Exception e) {
      e.printStackTrace();
    }
    return RESULT_DONE;
  }

  private boolean isChanged() {
    String[] activeWeekDays = distributionHelper.getActiveWeekDays();
    String distrDateEnd = distributionHelper.getDistrDateEnd();
    String sa = distributionHelper.getSourceAddress();
    String timeBegin = distributionHelper.getTimeBegin();
    String timeEnd = distributionHelper.getTimeEnd();
    boolean txmode = distributionHelper.isTxmode();
    if((!maxRepeat.equals(quizData.getMaxRepeat()))
        ||((defaultCategory!=null)&&(quizData.getDefaultCategory()==null))
        ||((defaultCategory==null)&&(quizData.getDefaultCategory()!=null))
        ||((defaultCategory!=null)&&(quizData.getDefaultCategory()!=null)
            &&(!defaultCategory.equals(quizData.getDefaultCategory())))
        ||(!distrDateEnd.equals(quizData.getDistrDateEndStr()))||(!sa.equals(quizData.getSourceAddress()))
        ||(!timeBegin.equals(quizData.getTimeBegin()))||(!timeEnd.equals(quizData.getTimeEnd()))
        ||(txmode != Boolean.valueOf(quizData.getTxmode()).booleanValue())
        ||(activeWeekDays.length != quizData.getActiveDays().size())) {
      return true;
    }
    Iterator iter = quizData.getActiveDays().iterator();
    while(iter.hasNext()) {
      String day1 = (String)iter.next();
      boolean flag = false;
      for(int i = 0; i<activeWeekDays.length;i++) {
        if(day1.equals(activeWeekDays[i])) {
         flag = true; break;
        }
      }
      if(!flag) {
        return true;
      }
    }

    Collection newCat = tableHelper.getCategories();
    if(newCat==null) {
      return true;
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

    try {
      String distrDateEnd = distributionHelper.getDistrDateEnd();
      if ((distrDateEnd != null) && (!distrDateEnd.trim().equals(""))) {
        Date distrDate =  dateFormat.parse(distrDateEnd);
        if(distrDate.before(new Date())||distrDate.after(quizData.getDateEnd())||distrDate.before(quizData.getDateBegin())) {
          return warning("Incorrect distribution end date");
        }
      }
      timeFormat.parse(distributionHelper.getTimeBegin());
      timeFormat.parse(distributionHelper.getTimeEnd());
    } catch (ParseException e) {
      logger.warn(e);
      e.printStackTrace();
      return warning(e.getMessage());
    }
    if(distributionHelper.getActiveWeekDays().length == 0) {
      return warning("Select one or more active days");
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

  public DistributionHelper getDistributionHelper() {
    return distributionHelper;
  }
}
