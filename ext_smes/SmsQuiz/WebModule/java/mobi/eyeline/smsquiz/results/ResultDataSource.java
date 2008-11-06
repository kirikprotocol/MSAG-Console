package mobi.eyeline.smsquiz.results;

import ru.novosoft.smsc.jsp.util.tables.impl.AbstractDataSourceImpl;
import ru.novosoft.smsc.jsp.util.tables.impl.QueryResultSetImpl;
import ru.novosoft.smsc.jsp.util.tables.QueryResultSet;
import ru.novosoft.smsc.jsp.util.tables.Query;
import ru.novosoft.smsc.util.RandomAccessFileReader;
import org.apache.log4j.Category;

import java.text.SimpleDateFormat;
import java.text.ParseException;
import java.util.*;
import java.io.*;


/**
 * author: alkhal
 * Date: 05.11.2008
 */
public class ResultDataSource extends AbstractDataSourceImpl {

  private static final Category log = Category.getInstance(ResultDataSource.class);

  public static final SimpleDateFormat DATE_IN_FILE_FORMAT = new SimpleDateFormat("dd.MM.yy HH:mm:ss");


  private final String resultDir;

  public ResultDataSource(String resultDir) {
    super(new String[]{"replyDate", "deliveryDate", "msisdn", "category", "message"});
    this.resultDir = resultDir;
  }

public QueryResultSet query(Query query_to_run) {
    final ResultFilter filter = (ResultFilter)query_to_run.getFilter();

    final QueryResultSetImpl rs = new QueryResultSetImpl(columnNames, "");

    // Prepare files list


  int total = 0;

      File dir = new File(resultDir);
      if(log.isDebugEnabled()) {
        log.debug("resultDir: "+dir.getAbsolutePath());
      }
      File[] files = dir.listFiles(new FilenameFilter() {
        public boolean accept(File dir, String name) {
          return (name.endsWith(".res"))&&(name.lastIndexOf(filter.getQuizId())>=0);
        }
      });

      if ((files != null)&&(files.length>0)) {
        File file = files[0];
        System.out.println("Analisys file: "+file.getAbsolutePath());
        String encoding = System.getProperty("file.encoding");
        RandomAccessFile f = null;
        if (log.isDebugEnabled()) {
          log.debug("Start reading results from file: " + file.getName());
        }
        int j=0;
        try {
          f = new RandomAccessFile(file, "r");

          RandomAccessFileReader is = new RandomAccessFileReader(f);

          String line;
          while(true) {
            line = is.readLine(encoding);
            if (line == null)
              break;

            StringTokenizer st = new StringTokenizer(line, ",");

            String msisdn = st.nextToken().trim();
            Date deliveryDate = DATE_IN_FILE_FORMAT.parse(st.nextToken().trim());
            Date replyDate = DATE_IN_FILE_FORMAT.parse(st.nextToken().trim());
            String category = st.nextToken();
            String message = st.nextToken();
            while(st.hasMoreTokens()) {
              message+=","+st.nextToken();
            }
            j++;
            final ResultDataItem di = new ResultDataItem(filter.getQuizId(),replyDate,deliveryDate,msisdn,category,message);
            if (log.isDebugEnabled())
              log.debug(line);
            if (filter.isItemAllowed(di)) {
              if (log.isDebugEnabled())
                log.debug("allowed");
              total++;
              if (rs.size() < query_to_run.getExpectedResultsQuantity())
                rs.add(di);
            }
          }
        } catch (FileNotFoundException e) {
        } catch (EOFException e) {
        } catch (IOException e) {
          e.printStackTrace();
        } catch (ParseException e) {
          e.printStackTrace();
        } finally {
          if (f != null)
            try {
              f.close();
            } catch (IOException e) {
            }
        }
        if (log.isDebugEnabled())
          log.debug(j + " messages have readed from file: " + file.getName());

      }

  rs.setTotalSize(total);
  return rs;
}

  public String getResultDir() {
    return resultDir;
  }

  public Collection getAllQuizes() {
    Collection quizes = new LinkedList();
    System.out.println("Result dir:"+resultDir);
    File dir = new File(resultDir);
    File[] files = dir.listFiles(new FilenameFilter() {
      public boolean accept(File dir, String name) {
        return name.endsWith(".res");
      }
    });
    if(files==null) {
      return quizes;
    }
    for(int j=0;j<files.length;j++) {
      String fileName = files[j].getName();
      quizes.add(fileName.substring(0,fileName.indexOf(".")));
    }
    return quizes;
  }

}
