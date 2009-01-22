package mobi.eyeline.smsquiz.beans;

import ru.novosoft.smsc.jsp.SMSCJspException;
import ru.novosoft.smsc.jsp.util.helper.statictable.PagedStaticTableHelper;

import javax.servlet.http.HttpServletRequest;
import java.util.List;
import java.util.Iterator;
import java.util.Date;
import java.io.File;
import java.text.SimpleDateFormat;
import java.text.ParseException;

import mobi.eyeline.smsquiz.QuizesDataSource;
import mobi.eyeline.smsquiz.quizes.view.QuizesStaticTableHelper;
import mobi.eyeline.smsquiz.quizes.view.QuizFilter;
import mobi.eyeline.smsquiz.quizes.QuizState;

/**
 * author: alkhal
 * Date: 07.11.2008
 */
public class QuizesList extends SmsQuizBean {

  public static final int RESULT_VIEW = SmsQuizBean.PRIVATE_RESULT + 1;
  public static final int RESULT_ADD = SmsQuizBean.PRIVATE_RESULT + 2;

  private String mbAdd = null;
  private String mbDelete = null;

  private String mbRefresh = null;


  private QuizesStaticTableHelper tableHelper = new QuizesStaticTableHelper("quizesTable");

  private QuizFilter filter;

  private String selectedQuizId;

  private String quizDir;

  private String arcDir;

  private  static final SimpleDateFormat df = new SimpleDateFormat("yyyyMMddHHmmss");

  private static final SimpleDateFormat formatter = new SimpleDateFormat("dd.MM.yyyy HH:mm:ss");

  private String startDate;

  private String tillDate;

  private String prefix;

  private String state;

  private List stateStringList;

  protected int init(List errors) {
    int result = super.init(errors);
    if (result != RESULT_OK)
      return result;

    try {
      filter = new QuizFilter(smsQuizContext.getSmsQuiz());
      stateStringList = QuizState.getQuizStateStringList();
      int pageSize = getSmsQuizContext().getQuizesPageSize();
      quizDir = getSmsQuizContext().getConfig().getString("quizmanager.dir_quiz");
      arcDir = getSmsQuizContext().getConfig().getString("quizmanager.dir_archive");
      tableHelper.setPageSize(pageSize);
      tableHelper.setDataSource(QuizesDataSource.getInstance());
      tableHelper.setFilter(filter);
      tableHelper.setSmsQuiz(smsQuizContext.getSmsQuiz());
      if(prefix!=null&&!prefix.trim().equals("")) {
        filter.setPrefix(prefix);
      }
      if(startDate!=null && !startDate.trim().equals("")) {
        filter.setStartDate(convertStringToDate(startDate));
      }
      if(tillDate!=null && !tillDate.trim().equals("")) {
        filter.setTillDate(convertStringToDate(tillDate));
      }
      if(state!=null && !state.trim().equals("")) {
        filter.setState(state);
      }
    } catch (Exception e) {
      logger.error(e,e);
      e.printStackTrace();
      return RESULT_ERROR;
    }

    return result;
  }

  public int process(HttpServletRequest request) {
    int result = super.process(request);
    if (result != RESULT_OK)
      return result;

    try {
      if (tableHelper.isDataCellSelected()) {
        result = processEdit(tableHelper.getSelectedCellId());

      } else if (mbAdd != null) {
        mbAdd = null;
        result = processAdd();

      } else if (mbDelete != null) {
        mbDelete = null;
        result = processDelete(request);
      } else if(mbRefresh != null) {
        mbRefresh = null;
        result = processRefresh();
      }

      this.tableHelper.fillTable();

    } catch (Exception e) {
      logger.error("Can't process request", e);
      return _error(new SMSCJspException("Can't create table", SMSCJspException.ERROR_CLASS_ERROR, e));
    }

    return result;
  }

  private int processRefresh() {
    try{
      QuizesDataSource.getInstance().refreshQuizes();
      return RESULT_OK;
    }catch(Exception e) {
      logger.error(e,e);
      return error(e.getMessage());
    }
  }

  private int processDelete(HttpServletRequest request) {
    if(!isOnline()) {
      return warning("Service SmsQuiz is offline");
    }
    String warnings = "";
    try {
      for (Iterator iter = tableHelper.getSelectedQuizesList(request).iterator(); iter.hasNext();) {
        final String quizId = (String) iter.next();
        String quizPath = quizDir + File.separator + quizId + ".xml";
        if (!new File(quizPath).exists()) {
          warnings += "Quiz's  file not found for id:" + quizId + System.getProperty("line.separator");
        }
        delete(quizPath);
        QuizesDataSource.getInstance().refreshQuiz(quizId);
        smsQuizContext.getSmsQuiz().quizChanged(quizId);
      }
    }
    catch (Exception e) {
      logger.error(e,e);
      e.printStackTrace();
    }

    tableHelper.setStartPosition(0);
    if (!warnings.equals("")) {
      return warning(warnings);
    }
    return RESULT_OK;
  }

  private void delete(String path) {
    File file = new File(arcDir);
    if(!file.exists()) {
      file.mkdirs();
    }
    renameFile(new File(path));
  }

  private void renameFile(File file) {
    try{
      String name = file.getName();
      file.renameTo(new File(arcDir+File.separator+name+"."+df.format(new Date())));
    }catch(Exception e) {
      logger.error(e,e);
    }
  }
  private int processAdd() {
    return RESULT_ADD;
  }

  private int processEdit(String cellId) {
    selectedQuizId = cellId;
    return RESULT_VIEW;
  }

  public String getMbAdd() {
    return mbAdd;
  }

  public void setMbAdd(String mbAdd) {
    this.mbAdd = mbAdd;
  }

  public String getMbDelete() {
    return mbAdd;
  }

  public void setMbDelete(String mbDelete) {
    this.mbDelete = mbDelete;
  }

  public PagedStaticTableHelper getTableHelper() {
    return tableHelper;
  }


  public String getSelectedQuizId() {
    return selectedQuizId;
  }

  public boolean isOnline() {
    try {
      return getAppContext().getHostsManager().getServiceInfo(getSmeId()).isOnline();
    } catch (Exception e) {
      logger.error(e,e);
    }
    return false;
  }

  public String getStartDate() {
    return (startDate == null) ? "" : startDate;
  }

  public void setStartDate(String startDate) {
    this.startDate = startDate;
  }

  public String getTillDate() {
    return (tillDate == null) ? "" : tillDate;
  }

  public void setTillDate(String tillDate) {
    this.tillDate = tillDate;
  }

  public String getPrefix() {
    return (prefix == null) ? "" : prefix;
  }

  public void setPrefix(String prefix) {
    this.prefix = prefix;
  }
  private Date convertStringToDate(String date) {
    Date converted = new Date();
    try {
      converted = formatter.parse(date);
    } catch (ParseException e) {
      e.printStackTrace();
      logger.error(e,e);
    }
    return converted;
  }

  public String convertDateToString(Date date) {
    return formatter.format(date);
  }

  public String getState() {
    return state;
  }

  public void setState(String state) {
    this.state = state;
  }

  public List getStateStringList() {
    return stateStringList;
  }
  public boolean isState(String state) {
    return (this.state != null) && state.equals(this.state);
  }

  public String getMbRefresh() {
    return mbRefresh;
  }

  public void setMbRefresh(String mbRefresh) {
    this.mbRefresh = mbRefresh;
  }
}
