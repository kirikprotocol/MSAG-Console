package ru.novosoft.smsc.infosme.backend.tables.stat;

import ru.novosoft.smsc.infosme.backend.StatQuery;
import ru.novosoft.smsc.jsp.util.tables.DataItem;
import ru.novosoft.smsc.jsp.util.tables.EmptyResultSet;
import ru.novosoft.smsc.jsp.util.tables.Query;
import ru.novosoft.smsc.jsp.util.tables.QueryResultSet;
import ru.novosoft.smsc.jsp.util.tables.impl.AbstractDataSourceImpl;
import ru.novosoft.smsc.jsp.util.tables.impl.QueryResultSetImpl;

import java.io.*;
import java.text.ParseException;
import java.text.SimpleDateFormat;
import java.util.*;

/**
 * User: artem
 * Date: 02.06.2008
 */

public class StatisticsDataSource extends AbstractDataSourceImpl {

  private static String DIR_DATE_FORMAT = "yyyyMMdd";
  private static String FILE_DATE_FORMAT = "HH";

  private final String storeDir;

  public StatisticsDataSource(String storeDir) {
    super(new String[] {"period", "taskId", "taskName", "generated", "delivered", "retried", "failed"});
    this.storeDir = storeDir;
  }

  public QueryResultSet query(Query query_to_run) {
    final StatQuery filter = (StatQuery)query_to_run.getFilter();

    Date fromDate = filter.getFromDate();
    Date endDate = filter.getTillDate();

    try {
      List files = getFiles(fromDate, endDate);
      if (files.isEmpty())
        return new EmptyResultSet();

      final QueryResultSetImpl rs = new QueryResultSetImpl(columnNames, "");

      final SimpleDateFormat fileDateFormat = new SimpleDateFormat(DIR_DATE_FORMAT + '/' + FILE_DATE_FORMAT);

      for (Iterator iter = files.iterator(); iter.hasNext();) {
        File f = (File)iter.next();

        Date fileDate = fileDateFormat.parse(f.getParentFile().getName() + '/' + f.getName());

        BufferedReader is = null;

        try {
          is = new BufferedReader(new FileReader(f));

          String line = is.readLine(); // Skip first line

          while((line = is.readLine()) != null) {
            StringTokenizer st = new StringTokenizer(line, ",");
            String taskId = st.nextToken();
            String taskName = st.nextToken();
            String minute = st.nextToken();
            String generated = st.nextToken();
            String delivered = st.nextToken();
            String retried = st.nextToken();
            String failed = st.nextToken();

            final DataItem item = new StatisticDataItem(fileDate, taskId, taskName, Integer.valueOf(generated),
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

      return rs;

    } catch (ParseException e) {
      e.printStackTrace();
    }

    return new EmptyResultSet();
  }

  private List getFiles (Date from, Date till) throws ParseException {

    List files = new LinkedList();

    File dir = new File(storeDir);
    if(dir.exists()) {

      final SimpleDateFormat dirNameFormat = new SimpleDateFormat(DIR_DATE_FORMAT);
      final SimpleDateFormat fileNameFormat = new SimpleDateFormat(DIR_DATE_FORMAT + '/' + FILE_DATE_FORMAT);
      final SimpleDateFormat fileDateFormat = new SimpleDateFormat(DIR_DATE_FORMAT + FILE_DATE_FORMAT);

      final Date fromDir = from == null ? null : dirNameFormat.parse(dirNameFormat.format(from));
      final Date fromFile = from == null ? null : fileDateFormat.parse(fileDateFormat.format(from));
      final Date tillDir = till == null ? null : dirNameFormat.parse(dirNameFormat.format(till));
      final Date tillFile = till == null ? null : fileDateFormat.parse(fileDateFormat.format(till));

      // Fetch directories
      File[] dirArr = dir.listFiles(new FileFilter() {
        public boolean accept(File file) {
          if (!file.isDirectory())
            return false;
          try {
            Date dirDate = dirNameFormat.parse(file.getName());
            return (fromDir == null || dirDate.compareTo(fromDir) >= 0) && (tillDir == null || dirDate.compareTo(tillDir) <= 0);
          } catch (ParseException e) {
            return false;
          }
        }
      });

      // Fetch files
      for (int i=0;i<dirArr.length;i++) {
        File directory = dirArr[i];
        String dirName = directory.getName();
        File[] fileArr = directory.listFiles();

        for(int j=0;j<fileArr.length;j++) {
          File f = fileArr[j];
          if (!f.isFile())
            continue;
          String name = f.getName();
          if (name.lastIndexOf(".csv") < 0)
            continue;

          Date fileDate = fileNameFormat.parse(dirName + '/' + name.substring(0, 2));
          if ((tillFile == null || fileDate.compareTo(tillFile) <= 0) && (fromFile == null || fileDate.compareTo(fromFile) >= 0))
            files.add(f);
        }
      }
    }
    return files;
  }
}
