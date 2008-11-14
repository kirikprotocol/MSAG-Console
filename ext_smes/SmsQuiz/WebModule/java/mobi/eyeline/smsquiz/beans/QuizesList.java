package mobi.eyeline.smsquiz.beans;

import ru.novosoft.smsc.jsp.SMSCJspException;
import ru.novosoft.smsc.jsp.util.helper.statictable.PagedStaticTableHelper;

import javax.servlet.http.HttpServletRequest;
import java.util.List;
import java.util.Iterator;
import java.util.Date;
import java.io.File;

import mobi.eyeline.smsquiz.quizes.view.QuizesDataSource;
import mobi.eyeline.smsquiz.quizes.view.QuizesStaticTableHelper;
import mobi.eyeline.smsquiz.QuizBuilder;
import mobi.eyeline.smsquiz.QuizShortData;

/**
 * author: alkhal
 * Date: 07.11.2008
 */
public class QuizesList extends SmsQuizBean {

  public static final int RESULT_VIEW = SmsQuizBean.PRIVATE_RESULT + 1;
  public static final int RESULT_ADD = SmsQuizBean.PRIVATE_RESULT + 2;

  private String mbAdd = null;
  private String mbDelete = null;


  private int maxTotalSize;

  private QuizesStaticTableHelper tableHelper = new QuizesStaticTableHelper("quizesTable");

  private String selectedQuizId;

  private String quizDir;

  private String dirWork;

  private String quizRes;

  protected int init(List errors) {
    int result = super.init(errors);
    if (result != RESULT_OK)
      return result;

    try {
      int pageSize = getSmsQuizContext().getQuizesPageSize();
      maxTotalSize = getSmsQuizContext().getMaxQuizTotalSize();
      quizDir = getSmsQuizContext().getConfig().getString("quizmanager.dir.quiz");
      dirWork = getSmsQuizContext().getConfig().getString("quizmanager.dir.work");
      quizRes = getSmsQuizContext().getConfig().getString("quizmanager.dir.result");
      tableHelper.setMaxTotalSize(maxTotalSize);
      tableHelper.setPageSize(pageSize);
      tableHelper.setQuizesDataSource(new QuizesDataSource(quizDir));
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
      if (this.tableHelper.getTotalSize() >= maxTotalSize)
        return _error(new SMSCJspException("Results size is more than " + maxTotalSize + ". Show first " + String.valueOf(maxTotalSize + 1) + " results.", SMSCJspException.ERROR_CLASS_WARNING));

    } catch (Exception e) {
      logger.error("Can't process request", e);
      return _error(new SMSCJspException("Can't create table", SMSCJspException.ERROR_CLASS_ERROR, e));
    }

    return result;
  }

  private int processDelete(HttpServletRequest request) {
    String warnings = "";
    System.out.println("Deleting quizes...");
    try {
      for (Iterator iter = tableHelper.getSelectedQuizesList(request).iterator(); iter.hasNext();) {
        final String quizId = (String) iter.next();
        System.out.println("Selected checkbox: " + quizId);
        String quizPath = quizDir + File.separator + quizId + ".xml";
        if (!new File(quizPath).exists()) {
          quizPath = quizDir + File.separator + quizId + ".xml.old";
          if (!new File(quizPath).exists()) {
            warnings += "Quiz's  file not found for id:" + quizId + System.getProperty("line.separator");
          }
        }
        QuizShortData quizData = QuizBuilder.parseQuiz(quizPath);
        Date now = new Date();
        if (now.before(quizData.getDateEnd()) && now.after(quizData.getDateBegin())) {
          warnings += "Quiz is active, it can't be deleted: " + quizId;
        } else {
          delete(quizId, quizPath);
        }
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

  private void delete(String quizId, String path) {
    System.out.println("Deleting quiz: " + quizId);
    File file;
    try {
      file = new File(path);
      System.out.println("try to delete file: " + file.getName());
      file.delete();
    } catch (Exception e) {
      logger.error(e);
    }
    String parentSlashQuizId = dirWork + File.separator + quizId;

    try {
      file = new File(parentSlashQuizId + ".status");
      if (!file.exists()) {
        file = new File(parentSlashQuizId + ".status.old");
      }
      System.out.println("try to delete file: " + file.getName());
      file.delete();
    } catch (Exception e) {
      logger.error(e);
    }
    try {
      file = new File(parentSlashQuizId + ".xml.bin");
      System.out.println("try to delete file: " + file.getName());
      file.delete();
    } catch (Exception e) {
      logger.error(e);
    }

    try {
      file = new File(parentSlashQuizId + ".xml.bin.j");
      System.out.println("try to delete file: " + file.getName());
      file.delete();
    } catch (Exception e) {
      logger.error(e);
    }

    try {
      file = new File(parentSlashQuizId + ".mod");
      if (!file.exists()) {
        file = new File(parentSlashQuizId + ".mod.processed");
      }
      System.out.println("try to delete file: " + file.getName());
      file.delete();
    } catch (Exception e) {
      logger.error(e);
    }

    try{
      file = new File(quizRes);
      File files[] = file.listFiles();
      if(files!=null) {
        for(int j=0;j<files.length;j++) {
          file = files[j];
          if((file.isFile())&&(file.getName().startsWith(quizId))) {
            file.delete();
            break;
          }
        }
      }
    } catch(Exception e) {
      logger.error(e);
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

}
