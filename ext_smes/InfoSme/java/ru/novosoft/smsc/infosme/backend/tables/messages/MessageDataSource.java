package ru.novosoft.smsc.infosme.backend.tables.messages;

import ru.novosoft.smsc.jsp.util.tables.impl.AbstractDataSourceImpl;
import ru.novosoft.smsc.jsp.util.tables.impl.QueryResultSetImpl;
import ru.novosoft.smsc.jsp.util.tables.QueryResultSet;
import ru.novosoft.smsc.jsp.util.tables.Query;
import ru.novosoft.smsc.infosme.backend.Message;
import ru.novosoft.smsc.util.AdvancedStringTokenizer;
import ru.novosoft.smsc.util.RandomAccessFileReader;

import java.io.*;
import java.util.*;
import java.text.SimpleDateFormat;
import java.text.ParseException;

/**
 * User: artem
 * Date: 29.05.2008
 */

public class MessageDataSource extends AbstractDataSourceImpl {

  private static final SimpleDateFormat dirNameFormat = new SimpleDateFormat("yyMMdd");
  private static final SimpleDateFormat msgDateFormat = new SimpleDateFormat("yyMMddhhmmss");

  private final String storeDir;

  public MessageDataSource(String storeDir) {
    super(new String[]{"state", "date", "msisdn", "region", "message"});
    this.storeDir = storeDir;
  }

  public QueryResultSet query(Query query_to_run) {
    MessageFilter filter = (MessageFilter)query_to_run.getFilter();

    final QueryResultSetImpl rs = new QueryResultSetImpl(columnNames, "");

    // Prepare files list
    Date fromDate = filter.getFromDate();
    Date tillDate = filter.getTillDate();
    if (tillDate == null)
      tillDate = new Date();

    Calendar cal = Calendar.getInstance();
    cal.setTime(fromDate);

    int total = 0;
    while (cal.getTime().before(tillDate)) {

      String dirName = dirNameFormat.format(cal.getTime());
      final String fileNamePrefix = String.valueOf(cal.get(Calendar.HOUR_OF_DAY));

      File dir = new File(storeDir, filter.getTaskId() + '/' + dirName);
      File[] files = dir.listFiles(new FilenameFilter() {
        public boolean accept(File dir, String name) {
          return name.equals(fileNamePrefix + "processed.csv") || name.equals(fileNamePrefix + ".csv");
        }
      });

      if (files != null) {
        long y1 = cal.get(Calendar.YEAR) % 10;
        long y2 = (cal.get(Calendar.YEAR) % 100) / 10;
        long m1 = (cal.get(Calendar.MONTH) + 1) % 10;
        long m2 = (cal.get(Calendar.MONTH) + 1) / 10;
        long d1 = cal.get(Calendar.DAY_OF_MONTH) % 10;
        long d2 = cal.get(Calendar.DAY_OF_MONTH) / 10;
        long h1 = cal.get(Calendar.HOUR_OF_DAY) % 10;
        long h2 = cal.get(Calendar.HOUR_OF_DAY) / 10;

        long idbase = y2 << 60 | y1 << 56 | m2 << 52 | m1 << 48 | d2 << 44 | d1 << 40 | h2 << 36 | h1 << 32;
        String encoding = System.getProperty("file.encoding");
			    
        for (int i=0; i< files.length; i++) {
          File file = files[i];          

          RandomAccessFile f = null;
          try {
            f = new RandomAccessFile(file, "r");

            RandomAccessFileReader is = new RandomAccessFileReader(f);

            String line = is.readLine(); // Skip first string

            while(true) {
              long offset =is.getFilePointer();
              line = is.readLine();
              if (line == null)
                break;


              // recode read line accourding to file.encoding
              line = new String(line.getBytes(), encoding);
//              byte[] buff = new byte[line.length()];
//              int lsz = line.length();
//              for( int k = 0; k < lsz; k++)
//                buff[k] = (byte)line.charAt(k);
//              line = new String(buff, encoding);
			  		
              AdvancedStringTokenizer st = new AdvancedStringTokenizer(line, ",");
              int state = Integer.parseInt(st.nextToken().trim());
              if (state == Message.State.DELETED.getId())
                continue;

              Date date = msgDateFormat.parse(st.nextToken().trim());
              String msisdn = st.nextToken().trim();
              String region = st.nextToken();
              String message = prepareMessage(st.nextToken());

              long id = idbase | ((int)offset);

              final MessageDataItem di = new MessageDataItem(id, filter.getTaskId(), state, date, msisdn, region, message);

              if (filter.isItemAllowed(di)) {
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
        }
      }
      cal.set(Calendar.HOUR, cal.get(Calendar.HOUR) + 1);
    }

    rs.setTotalSize(total);

    return rs;
  }

  private static String prepareMessage(String m) {
    String message = m;
    message = message.trim();
    // remove braces
    message = message.substring(1, message.length() - 1);

    message = message.replaceAll("\\\\n", "\n");
    message = message.replaceAll("\\\"", "\"");

//    message.replaceAll("\\\\", "\\");

    return message;
  }

}
