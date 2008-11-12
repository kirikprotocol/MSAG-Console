package mobi.eyeline.smsquiz.quizes.view;

import ru.novosoft.smsc.jsp.util.tables.impl.AbstractDataSourceImpl;
import ru.novosoft.smsc.jsp.util.tables.QueryResultSet;
import ru.novosoft.smsc.jsp.util.tables.Query;
import ru.novosoft.smsc.jsp.util.tables.EmptyResultSet;

import java.util.Date;
import java.io.File;
import java.io.FilenameFilter;

import org.apache.log4j.Category;
import mobi.eyeline.smsquiz.QuizBuilder;
import mobi.eyeline.smsquiz.QuizShortData;
import mobi.eyeline.smsquiz.quizes.view.QuizDataItem;

/**
 * author: alkhal
 * Date: 07.11.2008
 */
public class QuizesDataSource extends AbstractDataSourceImpl {

  private Category logger = Category.getInstance(this.getClass());

  private String quizDir;

  public static final String QUIZ_ID = "quiz_id";

  public static final String DATE_BEGIN = "dateBegin";

  public static final String DATE_END = "dateEnd";

  public static final String STATE = "state";

  public QuizesDataSource(String quizDir) {
    super(new String[]{QUIZ_ID, DATE_BEGIN, DATE_END, STATE});
    this.quizDir = quizDir;
  }

  public QueryResultSet query(Query query_to_run) {
    clear();
    File dir = new File(quizDir);
    if (!dir.exists()) {
      logger.error("QuizDir doesn't exist: " + dir);
      System.out.println("ERROR: QuizDir doesn't exist: " + dir);
      return new EmptyResultSet();
    }
    File[] files = dir.listFiles(new FilenameFilter() {
      public boolean accept(File dir, String name) {
        return name.endsWith(".xml") || name.endsWith(".xml.old");
      }
    });
    if (files == null) {
      return new EmptyResultSet();
    }

    try {
      for (int j = 0; j < files.length; j++) {
        String state;
        File file = files[j];
        String quizId = file.getName().substring(0, file.getName().indexOf("."));
        QuizShortData quizData = QuizBuilder.parseQuiz(file.getAbsolutePath());

        if (!new File(file.getParent() + File.separator + quizId + ".error").exists()) {
          Date now = new Date();
          if (now.before(quizData.getDateBegin())) {
            state = QuizState.INACTIVE;
          } else if (now.after(quizData.getDateEnd())) {
            state = QuizState.FINISHED;
          } else {
            state = QuizState.ACTIVE;
          }
        } else {
          state = QuizState.FINISHED_WITH_ERROR;
        }
        add(new QuizDataItem(quizId, quizData.getDateBegin(), quizData.getDateEnd(), state));
      }
    } catch (Exception e) {
      logger.error(e);
      e.printStackTrace();
    }

    return super.query(query_to_run);

  }


  public String getQuizDir() {
    return quizDir;
  }

}
