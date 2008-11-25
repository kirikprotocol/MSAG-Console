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

  private String arcDir;

  private String quizRes;

  protected int init(List errors) {
    int result = super.init(errors);
    if (result != RESULT_OK)
      return result;

    try {
      int pageSize = getSmsQuizContext().getQuizesPageSize();
      maxTotalSize = getSmsQuizContext().getMaxQuizTotalSize();
      quizDir = getSmsQuizContext().getConfig().getString("quizmanager.dir_quiz");
      dirWork = getSmsQuizContext().getConfig().getString("quizmanager.dir_work");
      arcDir = getSmsQuizContext().getConfig().getString("quizmanager.dir_archive");
      quizRes = getSmsQuizContext().getConfig().getString("quizmanager.dir_result");
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
        delete(quizId, quizPath, quizData.getAbFile());
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

  private void delete(String quizId, String path, String abFile) {
    File file =(new File(path)).getParentFile();
    if(!file.exists()) {
      file.mkdirs();
    }
    System.out.println("Deleting quiz: " + quizId);

    renameFile(new File(abFile));
    renameFile(new File(path));

    String parentSlashQuizId = dirWork + File.separator + quizId;

    renameFile(new File(parentSlashQuizId + ".status"));
    renameFile(new File(parentSlashQuizId + ".status.old"));
    deleteFile(new File(parentSlashQuizId + ".xml.bin"));
    deleteFile(new File(parentSlashQuizId + ".xml.bin.j"));
    renameFile(new File(parentSlashQuizId + ".error"));
    renameFile(new File(parentSlashQuizId + ".distr.error"));
    renameFile(new File(parentSlashQuizId + "(SmsQuiz).mod"));
    renameFile(new File(parentSlashQuizId + ".mod"));
    renameFile(new File(parentSlashQuizId + ".mod.processed"));
    renameFile(new File(parentSlashQuizId + "(SmsQuiz).mod.processed"));

    file = new File(quizRes);
    File files[] = file.listFiles();
    if(files!=null) {
      for(int j=0;j<files.length;j++) {
        file = files[j];
        if((file.isFile())&&(file.getName().startsWith(quizId))) {
          break;
        }
      }
    }
    deleteFile(file);
  }

  private void renameFile(File file) {
    try{
      System.out.println("try to rename file: " + file.getAbsolutePath());
      String name = file.getName();
      file.renameTo(new File(arcDir+File.separator+name));
    }catch(Exception e) {
      logger.error(e,e);
    }
  }
  private void deleteFile(File file) {
    try{
      System.out.println("try to delete file: " + file.getAbsolutePath());
      file.delete();
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
