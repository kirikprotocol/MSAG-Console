package mobi.eyeline.smsquiz.distribution;

import ru.novosoft.smsc.jsp.util.tables.QueryResultSet;
import ru.novosoft.smsc.jsp.util.tables.Query;
import ru.novosoft.smsc.jsp.util.tables.EmptyResultSet;
import ru.novosoft.smsc.jsp.util.tables.EmptyFilter;
import ru.novosoft.smsc.jsp.util.tables.impl.AbstractDataSourceImpl;
import ru.novosoft.smsc.infosme.backend.Message;
import ru.novosoft.smsc.infosme.backend.tables.messages.MessageFilter;
import ru.novosoft.smsc.infosme.backend.tables.messages.MessageDataSource;
import ru.novosoft.smsc.infosme.backend.tables.messages.MessageDataItem;
import ru.novosoft.smsc.infosme.backend.tables.messages.MessageQuery;
import ru.novosoft.smsc.util.RandomAccessFileReader;
import ru.novosoft.smsc.util.config.Config;

import java.io.*;
import java.util.*;
import java.text.ParseException;
import java.text.SimpleDateFormat;

import mobi.eyeline.smsquiz.QuizShortData;
import mobi.eyeline.smsquiz.QuizBuilder;
import org.apache.log4j.Category;

/**
 * User: alkhal
 * Date: 12.11.2008
 */

public class InfoSmeMessagesDataSource extends AbstractDataSourceImpl {

  private String workDir;
  protected static final Category log = Category.getInstance(InfoSmeMessagesDataSource.class);

  protected static final SimpleDateFormat dirNameFormat = new SimpleDateFormat("yyMMdd");
  public static final SimpleDateFormat msgDateFormat = new SimpleDateFormat("yyMMddHHmmss");

  private final String storeDir;

  private final String quizDir;

  private final Config config;

  public InfoSmeMessagesDataSource(Config config, String storeDir, String workDir, String quizDir) {
    super(new String[]{MessageDataSource.STATE, MessageDataSource.DATE,
        MessageDataSource.MSISDN, MessageDataSource.REGION, MessageDataSource.MESSAGE});
    this.storeDir = storeDir;
    this.workDir = workDir;
    this.quizDir = quizDir;
    this.config = config;
  }

  private String getTaskId(String quizName) {
    String res = null;
    String path = workDir + File.separator + quizName + ".status";
    File file = new File(path);
    if (!file.exists()) {
      path += ".old";
      if (!(file = new File(path)).exists()) {
        log.warn("Status file doen't exist for quiz: " + quizName);
        System.err.println("WARNING: Status file doen't exist for quiz: " + quizName);
        return null;
      }
    }
    InputStream stream = null;
    try {
      stream = new FileInputStream(file);
      Properties prop = new Properties();
      prop.load(stream);
      String id = prop.getProperty("distribution.id");
      if ((id != null) && (!id.trim().equals(""))) {
        res = id;
      } else {
        log.warn("Property id doesn't exist in " + path);
        System.err.println("WARNING: Property id doesn't exist in " + path);
      }
    } catch (IOException e) {
      log.error(e);
      e.printStackTrace();
    } finally {
      if (stream != null) {
        try {
          stream.close();
        } catch (IOException e) {
        }
      }
    }
    return res;
  }

  public QueryResultSet query(Query query_to_run) {
    clear();
    MessageFilter filter = (MessageFilter) query_to_run.getFilter();
    String quizPath = quizDir+File.separator+filter.getTaskId()+".xml";
    File file = new File(quizPath);
    if(!file.exists()) {
      quizPath+=".old";
      file = new File(quizPath);
    }
    System.out.println("Quiz path: " + quizPath);
    String quizName = file.getName().substring(0, file.getName().indexOf(".xml"));
    String taskId = getTaskId(quizName);

    if (taskId != null) {
      filter.setTaskId(taskId);
      MessageDataSource messageDataSource = new MessageDataSource(config, storeDir);
      return messageDataSource.query(new MessageQuery(query_to_run.getExpectedResultsQuantity(),
        filter, (String) query_to_run.getSortOrder().get(0), query_to_run.getStartPosition()));
    }
    else {
      return new EmptyResultSet();
    }
  }

}
