package mobi.eyeline.smsquiz.beans;

import ru.novosoft.smsc.jsp.SMSCJspException;
import ru.novosoft.smsc.jsp.util.helper.statictable.PagedStaticTableHelper;

import javax.servlet.http.HttpServletRequest;
import java.util.List;
import java.util.Iterator;
import java.util.Date;
import java.io.File;
import java.text.SimpleDateFormat;

import mobi.eyeline.smsquiz.quizes.view.QuizesDataSource;
import mobi.eyeline.smsquiz.quizes.view.QuizesStaticTableHelper;

/**
 * author: alkhal
 * Date: 07.11.2008
 */
public class QuizesList extends SmsQuizBean {

  public static final int RESULT_VIEW = SmsQuizBean.PRIVATE_RESULT + 1;
  public static final int RESULT_ADD = SmsQuizBean.PRIVATE_RESULT + 2;

  private String mbAdd = null;
  private String mbDelete = null;


  private QuizesStaticTableHelper tableHelper = new QuizesStaticTableHelper("quizesTable");

  private String selectedQuizId;

  private String quizDir;

  private String arcDir;

  private SimpleDateFormat df = new SimpleDateFormat("yyyyMMddHHmmss");

  protected int init(List errors) {
    int result = super.init(errors);
    if (result != RESULT_OK)
      return result;

    try {
      int pageSize = getSmsQuizContext().getQuizesPageSize();
      quizDir = getSmsQuizContext().getConfig().getString("quizmanager.dir_quiz");
      String dirWork = getSmsQuizContext().getConfig().getString("quizmanager.dir_work");
      arcDir = getSmsQuizContext().getConfig().getString("quizmanager.dir_archive");
      tableHelper.setPageSize(pageSize);
      tableHelper.setDataSource(new QuizesDataSource(quizDir, dirWork));
    } catch (Exception e) {
      logger.error(e);
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
      }

      this.tableHelper.fillTable();

    } catch (Exception e) {
      logger.error("Can't process request", e);
      return _error(new SMSCJspException("Can't create table", SMSCJspException.ERROR_CLASS_ERROR, e));
    }

    return result;
  }

  private int processDelete(HttpServletRequest request) {
    if(!isOnline()) {
      return warning("Service SmsQuiz is offline");
    }
    String warnings = "";
    System.out.println("Deleting quizes...");
    try {
      for (Iterator iter = tableHelper.getSelectedQuizesList(request).iterator(); iter.hasNext();) {
        final String quizId = (String) iter.next();
        System.out.println("Selected checkbox: " + quizId);
        String quizPath = quizDir + File.separator + quizId + ".xml";
        if (!new File(quizPath).exists()) {
          warnings += "Quiz's  file not found for id:" + quizId + System.getProperty("line.separator");
        }
        delete(quizPath);
      }
    }
    catch (Exception e) {
      logger.error(e);
      e.printStackTrace();
    }

    System.out.println("Deleting completed");
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
      System.out.println("try to rename file: " + file.getAbsolutePath());
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

}
