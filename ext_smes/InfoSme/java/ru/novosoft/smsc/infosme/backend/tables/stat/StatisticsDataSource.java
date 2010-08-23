package ru.novosoft.smsc.infosme.backend.tables.stat;

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.jsp.util.tables.EmptyResultSet;
import ru.novosoft.smsc.jsp.util.tables.Query;
import ru.novosoft.smsc.jsp.util.tables.QueryResultSet;
import ru.novosoft.smsc.jsp.util.tables.impl.AbstractDataSource;

import java.io.*;
import java.text.ParseException;
import java.text.SimpleDateFormat;
import java.util.*;

/**
 * User: artem
 * Date: 02.06.2008
 */

public class StatisticsDataSource extends AbstractDataSource {

  private static String DIR_DATE_FORMAT = "yyyyMMdd";
  private static String FILE_DATE_FORMAT = "HH";

  private final String storeDir;

  public StatisticsDataSource(String storeDir) {
    super(new String[] {"period", "taskId", "taskName", "generated", "delivered", "retried", "failed"});
    this.storeDir = storeDir;
  }

  public void visit(StatVisitor visitor, final StatQuery filter) {

    Date fromDate = filter.getFromDate();
    Date endDate = filter.getTillDate();

    try {
      List files = getFiles(fromDate, endDate);
      if (files.isEmpty())
        return;

      final SimpleDateFormat fileDateFormat = new SimpleDateFormat(DIR_DATE_FORMAT + '/' + FILE_DATE_FORMAT);

      for (Iterator iter = files.iterator(); iter.hasNext();) {
        File f = (File)iter.next();
        System.out.println("Handle file: "+f);

        Date fileDate = fileDateFormat.parse(f.getParentFile().getName() + '/' + f.getName());

        BufferedReader is = null;

        try {
          is = new BufferedReader(new FileReader(f));

          String line = is.readLine(); // Skip first line

          while((line = is.readLine()) != null) {
            StringTokenizer st = new StringTokenizer(line, ",");
            String taskId = st.nextToken();
            String taskName = st.nextToken().trim();
            while (taskName.charAt(taskName.length() -1) != '"')
              taskName += st.nextToken();
            taskName = taskName.substring(1, taskName.length() - 1);
            String minute = st.nextToken();
            String generated = st.nextToken();
            String delivered = st.nextToken();
            String retried = st.nextToken();
            String failed = st.nextToken();

            final StatisticDataItem item = new StatisticDataItem(fileDate, taskId, taskName, Integer.valueOf(generated),
                Integer.valueOf(delivered), Integer.valueOf(retried), Integer.valueOf(failed));

            if (filter == null || filter.isItemAllowed(item)) {
              if (!visitor.visit(item))
                return;
            }
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

    } catch (ParseException e) {
      e.printStackTrace();
    }
  }

  public QueryResultSet query(Query query_to_run) {
    init(query_to_run);
    final boolean[] err = new boolean[]{true};
    visit(new StatVisitor() {
      public boolean visit(StatisticDataItem item) {
        try {
          add(item);
          err[0] = false;
        } catch (AdminException e) {
          err[0] = true;
          return false;
        }
        return true;
      }
    }, (StatQuery)query_to_run.getFilter());

    return (err[0]) ? new EmptyResultSet() : getResults();
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
