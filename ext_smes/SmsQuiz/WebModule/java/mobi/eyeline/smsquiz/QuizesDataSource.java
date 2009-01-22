package mobi.eyeline.smsquiz;

import ru.novosoft.smsc.jsp.util.tables.impl.AbstractDataSource;
import ru.novosoft.smsc.jsp.util.tables.QueryResultSet;
import ru.novosoft.smsc.jsp.util.tables.Query;
import ru.novosoft.smsc.util.config.Config;

import java.io.*;
import java.util.Map;
import java.util.HashMap;
import java.util.Iterator;

import org.apache.log4j.Category;
import mobi.eyeline.smsquiz.QuizBuilder;
import mobi.eyeline.smsquiz.quizes.view.QuizDataItem;
import mobi.eyeline.smsquiz.quizes.view.QuizData;

/**
 * author: alkhal
 * Date: 07.11.2008
 */
public class QuizesDataSource extends AbstractDataSource {

  private static final Category logger = Category.getInstance(QuizesDataSource.class);

  private static Config config;

  private static QuizesDataSource ds;

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

  private Map quizes;

  public static void initDs(Config config) {
    if(config == null) {
      logger.error("Some argument are null");
      throw new IllegalArgumentException("Some argument are null");
    }
    QuizesDataSource.config = config;
  }

  public static synchronized QuizesDataSource getInstance() throws IllegalAccessException{
    if(config == null) {
      logger.error("Please init first!");
      throw new IllegalAccessException("Please init first!");
    }
    if(ds == null) {
     ds = new QuizesDataSource();
    }
    return ds;
  }

  private QuizesDataSource() {
    super(new String[]{QUIZ_ID, DATE_BEGIN, DATE_END, STATE});
    quizes = new HashMap();
    try{
      refreshQuizes();
    }catch(Exception e) {
      logger.error(e,e);
      e.printStackTrace();
    }
  }

  public QueryResultSet query(Query query_to_run) {
    init(query_to_run);

    try {
      Iterator iter = quizes.entrySet().iterator();
      while(iter.hasNext()) {
        Map.Entry e = (Map.Entry)iter.next();
        String quizId = (String)e.getKey();
        QuizData quizData = (QuizData)e.getValue();
        add(new QuizDataItem(quizData, quizId));
      }
    } catch (Exception e) {
      logger.error(e,e);
      e.printStackTrace();
    }
    return getResults();
  }

  public synchronized void refreshQuizes()  throws Exception{
    if(logger.isDebugEnabled()) {
      logger.debug("Refresh quizes...");
    }
    quizes.clear();
    String quizDir = config.getString("quizmanager.dir_quiz");
    File dir = new File(quizDir);
    if (!dir.exists()) {
      logger.error("QuizDir doesn't exist: " + dir);
      System.out.println("ERROR: QuizDir doesn't exist: " + dir);
    }
    File[] files = dir.listFiles(new FilenameFilter() {
      public boolean accept(File dir, String name) {
        return name.endsWith(".xml");
      }
    });

    for (int j = 0; j < files.length; j++) {
      File file = files[j];
      String quizId = file.getName().substring(0, file.getName().indexOf("."));
      QuizData quizData = QuizBuilder.parseAll(file.getAbsolutePath());
      quizes.put(quizId, quizData);
    }

    if(logger.isDebugEnabled()) {
      logger.debug("Refresh quizes complete");
    }
  }

  public synchronized QuizData refreshQuiz(String quizId) throws Exception{
    if(logger.isDebugEnabled()) {
      logger.debug("Refresh quiz with id: "+quizId);
    }
    String quizDir = config.getString("quizmanager.dir_quiz");
    String fileName = quizDir + File.separator + quizId + ".xml";
    File f = new File(fileName);
    QuizData quizData = null;
    if(f.exists()) {
      quizData = QuizBuilder.parseAll(f.getAbsolutePath());
      quizes.put(quizId, quizData);
    } else {
      Object obj = quizes.remove(quizId);
      if(obj!=null) {
        quizData = (QuizData)obj;
      }
    }
    if(logger.isDebugEnabled()) {
      logger.debug("Refresh quizes complete");
    }
    return quizData;
  }

}
