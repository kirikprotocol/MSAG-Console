package mobi.eyeline.smsquiz.beans;

import mobi.eyeline.smsquiz.quizes.view.QuizFullData;
import mobi.eyeline.smsquiz.QuizBuilder;

import javax.servlet.http.HttpServletRequest;
import java.util.List;
import java.io.File;

/**
 * author: alkhal
 * Date: 10.11.2008
 */
public class QuizView extends SmsQuizBean {

  private QuizFullData quizData = new QuizFullData();

  public static final int RESULT_DONE = SmsQuizBean.PRIVATE_RESULT + 1;
  public static final int RESULT_EDIT = SmsQuizBean.PRIVATE_RESULT + 2;

  private String quiz;

  private String quizDir;

  private String mbDone = null;

  private String mbEdit = null;

  protected int init(List errors) {
    int result = super.init(errors);
    if (result != RESULT_OK)
      return result;
    try {
      quizDir = getSmsQuizContext().getConfig().getString("quizmanager.dir.quiz");
    } catch (Exception e) {
      logger.error(e);
      e.printStackTrace();
      result = error(e.getMessage());
    }
    if (!buildData()) {
      logger.error("Can't read quiz: quizId doesn't exist with id=" + quiz);
      System.out.println("Can't read quiz: quizId doesn't exist with id=" + quiz);
      return error("Can't read quiz: quizId doesn't exist with id=" + quiz);
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
      return RESULT_DONE;
    }
    if(mbEdit!=null) {
      mbEdit = null;
      return RESULT_EDIT;
    }
    return result;
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
}
