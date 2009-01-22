package mobi.eyeline.smsquiz.quizes.view;

import ru.novosoft.smsc.jsp.util.tables.impl.AbstractDataSourceImpl;
import ru.novosoft.smsc.jsp.util.tables.impl.AbstractDataSource;
import ru.novosoft.smsc.jsp.util.tables.QueryResultSet;
import ru.novosoft.smsc.jsp.util.tables.Query;
import ru.novosoft.smsc.jsp.util.tables.EmptyResultSet;
import ru.novosoft.smsc.admin.AdminException;

import java.util.Properties;
import java.util.Date;
import java.util.StringTokenizer;
import java.io.*;
import java.text.SimpleDateFormat;

import org.apache.log4j.Category;
import mobi.eyeline.smsquiz.QuizBuilder;
import mobi.eyeline.smsquiz.SmsQuiz;
import mobi.eyeline.smsquiz.quizes.view.QuizDataItem;

/**
 * author: alkhal
 * Date: 07.11.2008
 */
public class QuizesDataSource extends AbstractDataSource {

  private static final Category logger = Category.getInstance(QuizesDataSource.class);

  private String quizDir;

  public static final String QUIZ_ID = "quiz_id";

  public static final String QUIZ_NAME = "quizName";

  public static final String DATE_BEGIN = "dateBegin";

  public static final String DATE_END = "dateEnd";

  public static final String STATE = "state";

  public static final String ACTIVE_DAYS = "activeDays";

  public static final String AB_FILE = "abFile";

  public static final String QUESTION = "question";

  public static final String REPEAT_QUESTION = "repeatQuestion";

  public static final String TIME_BEGIN = "timeBegin";

  public static final String TIME_END = "timeEnd";

  public static final String TX_MODE = "txmode";

  public static final String SA = "sa";

  public static final String DA = "da";

  public static final String MAX_REPEAT = "maxrepeat";

  public static final String DEFAULT_CAT = "defaultCat";

  public static final String CATEGORIES = "categories";

  public static final String DISTR_DATE_END = "distrDateEnd";

  public QuizesDataSource(String quizDir) {
    super(new String[]{QUIZ_ID, DATE_BEGIN, DATE_END, STATE});
    this.quizDir = quizDir;
  }

  public QueryResultSet query(Query query_to_run) {
    init(query_to_run);

    File dir = new File(quizDir);
    if (!dir.exists()) {
      logger.error("QuizDir doesn't exist: " + dir);
      System.out.println("ERROR: QuizDir doesn't exist: " + dir);
      return new EmptyResultSet();
    }
    File[] files = dir.listFiles(new FilenameFilter() {
      public boolean accept(File dir, String name) {
        return name.endsWith(".xml");
      }
    });
    if (files == null) {
      return new EmptyResultSet();
    }

    try {
      for (int j = 0; j < files.length; j++) {
        File file = files[j];
        String quizId = file.getName().substring(0, file.getName().indexOf("."));
        QuizData quizData = QuizBuilder.parseAll(file.getAbsolutePath());
        add(new QuizDataItem(quizData, quizId));
      }
    } catch (Exception e) {
      logger.error(e,e);
      e.printStackTrace();
    }
    return getResults();
  }

  public String getQuizDir() {
    return quizDir;
  }

}
