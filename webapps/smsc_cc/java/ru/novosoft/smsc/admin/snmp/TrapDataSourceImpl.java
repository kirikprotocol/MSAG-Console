package ru.novosoft.smsc.admin.snmp;

import org.apache.log4j.Logger;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.filesystem.FileSystem;
import ru.novosoft.smsc.util.Functions;

import java.io.*;
import java.text.ParseException;
import java.text.SimpleDateFormat;
import java.util.Arrays;
import java.util.Comparator;
import java.util.Date;
import java.util.TimeZone;

/**
 * @author Aleksandr Khalitov
 */
class TrapDataSourceImpl implements TrapDataSource {

  protected static final TimeZone TRAPS_TIMEZONE=TimeZone.getTimeZone("UTC");

  protected static final TimeZone LOCAL_TIMEZONE=TimeZone.getDefault();

  private static final Logger logger = Logger.getLogger(TrapDataSourceImpl.class);

  private File snmpDir;

  private FileSystem fileSystem;

  private final static String fileFormat = "yyyyMMdd_HHmmss'.ucs.csv'";

  TrapDataSourceImpl(File snmpDir, FileSystem fileSystem) {
    this.snmpDir = snmpDir;
    this.fileSystem = fileSystem;
  }

  private File[] getFiles(final Date startDate, final Date endDate) {

    final long oneDay = 24*60*60*1000;

    final SimpleDateFormat sdf= new SimpleDateFormat(fileFormat);

    File[] fs = snmpDir.listFiles(new FileFilter() {
      @Override
      public boolean accept(File pathname) {
        if(!pathname.isFile()) {
          return false;
        }
        Date fileDate;
        try {
          fileDate = sdf.parse(pathname.getName());
        } catch (ParseException e) {
          return false;
        }
        if(startDate != null && fileDate.getTime() < startDate.getTime() - oneDay) {
          return false;
        }
        if(endDate != null && fileDate.getTime() > endDate.getTime() + oneDay) {
          return false;
        }
        return true;
      }
    });

    Arrays.sort(fs, new Comparator<File>() {
      @Override
      public int compare(File o1, File o2) {
        try{
          Date d1 = sdf.parse(o1.getName());
          Date d2 = sdf.parse(o2.getName());
          return d1.compareTo(d2);
        }catch (ParseException e){
          logger.error(e,e);
        }
        return 0;
      }
    });

    return fs;
  }


  public void getTraps(Date from, Date till, SnmpTrapVisitor visitor) throws AdminException {

    if(!snmpDir.exists()) {
      return;
    }

    final Date startDate = from == null ? null : Functions.convertTime(from, LOCAL_TIMEZONE, TRAPS_TIMEZONE);
    final Date endDate = till == null ? null : Functions.convertTime(till, LOCAL_TIMEZONE, TRAPS_TIMEZONE);

    File[] dirFiles = getFiles(startDate, endDate);

    if(dirFiles == null || dirFiles.length == 0) {
      return;
    }

    for (File dirFile : dirFiles) {
      if(!processFile(dirFile, startDate, endDate, visitor)) {
        return;
      }
    }
  }

  private boolean processFile(File file, Date startDate, Date endDate, SnmpTrapVisitor visitor) throws AdminException{

    if(logger.isDebugEnabled()) {
      logger.debug("Start to process file: "+file.getAbsolutePath());
    }

    SimpleDateFormat rowSubmitTimeFormat = new SimpleDateFormat("dd.MM.yyyy HH:mm:ss");

    BufferedReader br = null;
    try {
      br = new BufferedReader(new InputStreamReader(fileSystem.getInputStream(file)));
      br.readLine();
      StreamTokenizer st = new StreamTokenizer(br);
      st.resetSyntax();
      st.whitespaceChars(',', ',');
      st.wordChars('0', '9');
      st.wordChars('.', '.');
      st.wordChars(' ', ' ');
      st.wordChars(':', ':');
      st.quoteChar('"');
      st.nextToken();

      Date date;
      String alarmId, alarmCategory, text;
      SnmpTrap r;
      int severity;
      while (st.ttype != StreamTokenizer.TT_EOF) {
        // Read string
        try {

          date = rowSubmitTimeFormat.parse(st.sval);

          if(endDate != null && date.compareTo(endDate) >= 0) {
            return false;
          }

          st.nextToken();

          alarmId = st.sval;

          st.nextToken();

          alarmCategory = st.sval;

          st.nextToken();

          try {
            severity = Integer.valueOf(st.sval);
          } catch (NumberFormatException e) {
            severity = 0;
          }

          st.nextToken();

          text = st.sval;

          if(startDate == null || date.compareTo(startDate)>=0) {
            r = new SnmpTrap();
            r.setSeverity(severity);
            r.setText(text);
            r.setAlarmId(alarmId);
            r.setAlarmCategory(alarmCategory);
            r.setSubmitDate(Functions.convertTime(date, TRAPS_TIMEZONE, LOCAL_TIMEZONE));
            if(!visitor.visit(r)) {
              return false;
            }
          }

          st.nextToken();

          if (st.ttype == StreamTokenizer.TT_EOL) st.nextToken();
        } catch (Exception e) {
          logger.warn("Invalid snmp log line: " + e.getMessage());
          while (st.ttype != StreamTokenizer.TT_EOL) st.nextToken();
          st.nextToken();
        }
      }
      return true;
    } catch (IOException e) {
      logger.error(e,e);
      throw new SnmpException("internal_error");
    } finally {
      if (br != null) {
        try {
          br.close();
        } catch (IOException ignored) {}
      }
    }
  }
}
