package ru.novosoft.smsc.infosme.backend.tables.stat;

import ru.novosoft.smsc.jsp.util.tables.impl.AbstractDataSourceImpl;
import ru.novosoft.smsc.jsp.util.tables.impl.QueryResultSetImpl;
import ru.novosoft.smsc.jsp.util.tables.QueryResultSet;
import ru.novosoft.smsc.jsp.util.tables.Query;
import ru.novosoft.smsc.jsp.util.tables.DataItem;
import ru.novosoft.smsc.jsp.util.tables.EmptyResultSet;
import ru.novosoft.smsc.infosme.backend.StatQuery;
import ru.novosoft.smsc.infosme.backend.Task;
import ru.novosoft.smsc.util.config.Config;

import java.util.Calendar;
import java.util.Date;
import java.util.StringTokenizer;
import java.text.SimpleDateFormat;
import java.io.*;

/**
 * User: artem
 * Date: 02.06.2008
 */

public class StatisticsDataSource extends AbstractDataSourceImpl {

  private static final SimpleDateFormat dirNameFormat = new SimpleDateFormat("yyyyMMdd");
  private static final SimpleDateFormat fileNameFormat = new SimpleDateFormat("HH");

  private final String storeDir;
  private final Config config;

  public StatisticsDataSource(Config config, String storeDir) {
    super(new String[] {"period", "taskId", "taskName", "generated", "delivered", "retried", "failed"});
    this.storeDir = storeDir;
    this.config = config;
  }

  public QueryResultSet query(Query query_to_run) {
    final StatQuery filter = (StatQuery)query_to_run.getFilter();

    Date fromDate = filter.getFromDate();
    if (fromDate == null) {
      try {
        Task t = new Task(config, filter.getTaskId());
        fromDate = t.getStartDateDate();
      } catch (Exception e) {
        e.printStackTrace();
        return new EmptyResultSet();
      }
    }

    Calendar cal = Calendar.getInstance();
    cal.setTime(fromDate);

    Date endDate = filter.getTillDate();
    if (endDate == null)
      endDate = new Date();

    final QueryResultSetImpl rs = new QueryResultSetImpl(columnNames, "");


    while (cal.getTime().before(endDate)) {

      final String dirName = dirNameFormat.format(cal.getTime());

      final String fileNamePrefix = fileNameFormat.format(cal.getTime());

      File f = new File(storeDir, dirName + "/" + fileNamePrefix + ".csv");

      if (f.exists()) {
        System.out.println("processFile " + f.getAbsolutePath());

        BufferedReader is = null;

        try {
          is = new BufferedReader(new FileReader(f));

          String line = is.readLine();

          while((line = is.readLine()) != null) {
            StringTokenizer st = new StringTokenizer(line, ",");
            String taskId = st.nextToken();
            String taskName = st.nextToken();
            String minute = st.nextToken();
            String generated = st.nextToken();
            String delivered = st.nextToken();
            String retried = st.nextToken();
            String failed = st.nextToken();

            final DataItem item = new StatisticDataItem(cal.getTime(), taskId, taskName, Integer.valueOf(generated),
                Integer.valueOf(delivered), Integer.valueOf(retried), Integer.valueOf(failed));

            if (filter.isItemAllowed(item))
              rs.add(item);
          }

        } catch (EOFException e) {
        } catch (IOException e) {
          e.printStackTrace();
        } finally {
          if (is != null)
            try {
              is.close();
            } catch (IOException e) {
            }
        }
      }
      cal.add(Calendar.HOUR, 1);
     // cal.set(Calendar.HOUR, cal.get(Calendar.HOUR) + 1);    
    }

    return rs;
  }
}
