package ru.novosoft.smsc.emailsme.backend.tables.stat;

import ru.novosoft.smsc.jsp.util.tables.Query;
import ru.novosoft.smsc.jsp.util.tables.QueryResultSet;
import ru.novosoft.smsc.jsp.util.tables.impl.AbstractDataSourceImpl;
import ru.novosoft.smsc.jsp.util.tables.impl.QueryResultSetImpl;

import java.io.*;
import java.text.SimpleDateFormat;
import java.text.ParseException;
import java.util.Calendar;
import java.util.Date;

/**
 * User: artem
 * Date: 04.09.2008
 */

public class StatisticsDataSource extends AbstractDataSourceImpl {

  private static final SimpleDateFormat dirNameFormat = new SimpleDateFormat("yyyyMMdd");
  private static final SimpleDateFormat fileNameFormat = new SimpleDateFormat("HH");
  private static final SimpleDateFormat itemDateFormat = new SimpleDateFormat("yyyy-MM-dd HH:mm");

  private final String storeDir;

  public StatisticsDataSource(String storeDir) {
    super(new String[] {"hour", "sms_received_ok", "sms_received_fail", "sms_transmitted_ok", "sms_transmitted_fail"});
    this.storeDir = storeDir;
  }

  public QueryResultSet query(Query query_to_run) {
    StatisticsFilter filter = (StatisticsFilter)query_to_run.getFilter();

    Calendar cal = Calendar.getInstance();
    cal.setTime(filter.getFrom());

    Date endDate = filter.getTill();
    if (endDate == null)
      endDate = new Date();

    final QueryResultSetImpl rs = new QueryResultSetImpl(columnNames, "");

    while (cal.getTime().before(endDate)) {

      final String dirName = dirNameFormat.format(cal.getTime());

      final String fileNamePrefix = fileNameFormat.format(cal.getTime());

      File f = new File(storeDir, dirName + "/" + fileNamePrefix + ".txt");

      if (f.exists()) {
        System.out.println("processFile " + f.getAbsolutePath());

        BufferedReader is = null;

        try {
          is = new BufferedReader(new FileReader(f));

          while(true) {
            String dateLine = is.readLine();
            if (dateLine == null)
              break;

            Date itemDate = itemDateFormat.parse(dateLine);

            StatisticDataItem item = new StatisticDataItem(itemDate, cal.get(Calendar.HOUR_OF_DAY));
            for (int i=0; i<4; i++) {
              String line = is.readLine();
              int ind = line.indexOf(',');
              String param = line.substring(0, ind);
              String val = line.substring(ind + 1);
              item.setValue(param, Integer.decode(val));
            }
            if (filter.isItemAllowed(item))
              rs.add(item);
          }

        } catch (EOFException e) {
        } catch (IOException e) {
          e.printStackTrace();
        } catch (ParseException e) {
          e.printStackTrace();
        } finally {
          if (is != null)
            try {
              is.close();
            } catch (IOException e) {
            }
        }
      }
      cal.set(Calendar.HOUR, cal.get(Calendar.HOUR) + 1);
    }

    return rs;
  }
}
