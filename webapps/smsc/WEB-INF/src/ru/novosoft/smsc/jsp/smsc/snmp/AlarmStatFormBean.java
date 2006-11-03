package ru.novosoft.smsc.jsp.smsc.snmp;

import ru.novosoft.smsc.jsp.SMSCErrors;
import ru.novosoft.smsc.jsp.smsc.IndexBean;
import ru.novosoft.smsc.jsp.smsc.snmp.tables.*;
import ru.novosoft.smsc.jsp.smsc.snmp.tables.fields.StringCell;
import ru.novosoft.smsc.jsp.smsc.snmp.tables.fields.DateCell;
import ru.novosoft.smsc.jsp.smsc.snmp.tables.fields.IntCell;
import ru.novosoft.smsc.util.Functions;
import ru.novosoft.smsc.util.config.Config;

import javax.servlet.http.HttpServletRequest;
import java.io.*;
import java.text.ParseException;
import java.text.SimpleDateFormat;
import java.util.*;

public class AlarmStatFormBean extends IndexBean {
  private final static String DATE_FORMAT = "dd.MM.yyyy";
  private final static String DATE_FILE_FORMAT = "yyyyMMdd";
  private final static String ROW_SUBMIT_TIME_FORMAT = "dd.MM.yyyy HH:mm:ss";

  public static final String ALL_STATS = "all_stats";
  public static final String OPENED_STATS = "opened_stats";
  public static final String CLOSED_STATS = "closed_stats";

//  private Calendar calendar = Calendar.getInstance(TimeZone.getTimeZone("UTC"));
  private Calendar localCalendar = Calendar.getInstance(TimeZone.getDefault());
  private SimpleDateFormat dateFileFormat = new SimpleDateFormat(DATE_FILE_FORMAT);
  private SimpleDateFormat rowSubmitTimeFormat = new SimpleDateFormat(ROW_SUBMIT_TIME_FORMAT);

  private static final String ALARM_STAT_CSV_FILE_PARAM = "snmp.csvFileDir";

  private final StatsTable allStatsTable = new StatsTable(AllStatsTableScheme.SCHEME);
  private final StatsTable openedStatsTable = new StatsTable(OpenedStatsTableScheme.SCHEME);
  private final StatsTable closedStatsTable = new StatsTable(ClosedStatsTableScheme.SCHEME);

  private StatsTable tableToShow = allStatsTable;
  private boolean isTableFilterModified = false;
  private boolean hasResults = false;

  private Date date = Functions.truncateTime(new Date());

  private String mbQuery = null;
  private String mbClear = null;
  private String tableFilter = ALL_STATS;

  public int process(HttpServletRequest request) {
    int result = super.process(request);
    if (result != RESULT_OK) return result;

    if (pageSize == 0) pageSize = 20;

    if (sort != null) {
      if (sort.startsWith("-"))
        tableToShow.orderBy(sort.substring(1), false);
      else
        tableToShow.orderBy(sort, true);
    }

    if (mbQuery != null)
      result = processQuery(appContext.getConfig());
    else if (mbClear != null)
      result = clearQuery();

    if (tableFilter != null)
      changeTableFilter();

    mbQuery = null;
    mbClear = null;

    return result;
  }

  private int changeTableFilter() {
    if (tableFilter.equals(ALL_STATS))
      tableToShow = allStatsTable;
    else if (tableFilter.equals(OPENED_STATS))
      tableToShow = openedStatsTable;
    else if (tableFilter.equals(CLOSED_STATS))
      tableToShow = closedStatsTable;

    if (isTableFilterModified) {
      startPosition = 0;
      isTableFilterModified = false;
    }

    totalSize = tableToShow.getSize();
    return RESULT_OK;
  }

  private int processQuery(Config config) {
    clearQuery();
    try {
      String fileDirName = config.getString(ALARM_STAT_CSV_FILE_PARAM);
      rowSubmitTimeFormat.setTimeZone(TimeZone.getTimeZone("UTC"));
      String dateFilePrefix = dateFileFormat.format(date);
      localCalendar.setTime(date);
      localCalendar.add( Calendar.DAY_OF_MONTH, -1);
      String dateFilePrefix_prev = dateFileFormat.format(localCalendar.getTime());
      logger.info("Querying snmp files for : "+dateFilePrefix+" and "+dateFilePrefix_prev);
      File dirNameFile = new File(fileDirName);
      File[] dirFiles = dirNameFile.listFiles();
      if (dirFiles == null || dirFiles.length == 0) return RESULT_OK;

      for (int j = 0; j < dirFiles.length; j++) {
        final String fileName = dirFiles[j].getName();
        if (fileName == null || fileName.length() <= 0 ||
          !(fileName.toLowerCase().startsWith(dateFilePrefix) || fileName.toLowerCase().startsWith(dateFilePrefix_prev))) continue;
        processFile(new File(dirNameFile, dirFiles[j].getName()));
      }

      processClosedStatsTable();

      startPosition = 0;
      totalSize = tableToShow.getSize();
      hasResults = true;
      return RESULT_OK;
    }
    catch (ParseException e) {
      e.printStackTrace();
      return error(SMSCErrors.error.snmp.CSVisInvalid, e.getMessage());
    }
    catch (Config.ParamNotFoundException e) {
      e.printStackTrace();
      return error(SMSCErrors.error.snmp.InitFailed, e.getMessage());
    }
    catch (Config.WrongParamTypeException e) {
      e.printStackTrace();
      return error(SMSCErrors.error.snmp.InitFailed, e.getMessage());
    }
    catch (Exception e) {
      e.printStackTrace();
      return error(SMSCErrors.error.snmp.InitFailed, e.getMessage());
    }
  }

  private void processFile(File src) throws ParseException {
    logger.debug("Processing file "+src);
    localCalendar.setTime( date );

    BufferedReader br = null;
    try {
      br = new BufferedReader(new FileReader(src));
      br.readLine(); // строка заголовка
      StreamTokenizer st = new StreamTokenizer(br);
      st.resetSyntax();
      st.whitespaceChars(',', ',');
      st.wordChars('0', '9');
      st.wordChars('.', '.');
      st.wordChars(' ', ' ');
      st.wordChars(':', ':');
      st.quoteChar('"');
      st.nextToken();

      Date submitTime;
      String alarmId;
      String alarmCategory;
      int severity;
      String text;

      while (st.ttype != StreamTokenizer.TT_EOF) {
        // Read string
        submitTime = rowSubmitTimeFormat.parse(st.sval);
        st.nextToken();
        alarmId = st.sval;
        st.nextToken();
        alarmCategory = st.sval;
        st.nextToken();
        try {
          severity = Integer.valueOf(st.sval).intValue();
        } catch (NumberFormatException e) {
          severity = 0;
        }
        st.nextToken();
        text = st.sval;

        // Fill tables
        addStatToAllStatsTable(submitTime, alarmId, alarmCategory, severity, text); // All stats table
        addStatToOpenedStatsTable(submitTime, alarmId, alarmCategory, severity, text); // Opened stats table
        addStatToClosedStatsTable(submitTime, alarmId, alarmCategory, severity, text); // Closed stats table

        st.nextToken();
        if (st.ttype == StreamTokenizer.TT_EOL) st.nextToken();
      }
    } catch (FileNotFoundException e) {
      e.printStackTrace();
    } catch (IOException e) {
      e.printStackTrace();
    } finally {
      try {
        if (br != null)
          br.close();
      } catch (IOException e) {
        e.printStackTrace();
      }
    }
  }

  private void addStatToAllStatsTable(final Date submitTime, final String alarmId, final String alarmCategory,
                                      final int severity, final String text) {
    final StatsTableRow row = allStatsTable.createRow();
    row.addFieldValue(AllStatsTableScheme.SCHEME.SUBMIT_TIME, new DateCell(submitTime));
    row.addFieldValue(AllStatsTableScheme.SCHEME.ALARM_ID, new StringCell(alarmId));
    row.addFieldValue(AllStatsTableScheme.SCHEME.ALARM_CATEGORY, new StringCell(alarmCategory));
    row.addFieldValue(AllStatsTableScheme.SCHEME.SEVERITY, new IntCell(severity));
    row.addFieldValue(AllStatsTableScheme.SCHEME.TEXT, new StringCell(text));
  }

  private void addStatToOpenedStatsTable(final Date submitTime, final String alarmId, final String alarmCategory,
                                         final int severity, final String text) {
    if (severity != 1) { // add new row
      StatsTableRow row = getOpenStatsTableRow(alarmId, alarmCategory);
      if (row == null)
        row = openedStatsTable.createRow();
      else if (((Date)row.getValue(OpenedStatsTableScheme.SCHEME.SUBMIT_TIME)).after(submitTime))
        return;
      
      row.addFieldValue(OpenedStatsTableScheme.SCHEME.SUBMIT_TIME, new DateCell(submitTime));
      row.addFieldValue(OpenedStatsTableScheme.SCHEME.ALARM_ID, new StringCell(alarmId));
      row.addFieldValue(OpenedStatsTableScheme.SCHEME.ALARM_CATEGORY, new StringCell(alarmCategory));
      row.addFieldValue(OpenedStatsTableScheme.SCHEME.SEVERITY, new IntCell(severity));
      row.addFieldValue(OpenedStatsTableScheme.SCHEME.TEXT, new StringCell(text));

    } else { // remove row
      final StatsTableRow row = getOpenStatsTableRow(alarmId, alarmCategory);
      if (row != null)
        openedStatsTable.deleteRow(row);
    }
  }

  private StatsTableRow getOpenStatsTableRow(final String alarmId, final String alarmCategory) {
    for (Iterator iter = openedStatsTable.getRows(); iter.hasNext();) {
      final StatsTableRow row = (StatsTableRow)iter.next();
      if (row.getValueAsString(OpenedStatsTableScheme.SCHEME.ALARM_ID).equals(alarmId) &&
          row.getValueAsString(OpenedStatsTableScheme.SCHEME.ALARM_CATEGORY).equals(alarmCategory))
        return row;
    }
    return null;
  }

  private void addStatToClosedStatsTable(final Date submitTime, final String alarmId, final String alarmCategory,
                                         final int severity, final String text) {
    if (severity != 1) { // add new row
      final StatsTableRow row = closedStatsTable.createRow();
      row.addFieldValue(ClosedStatsTableScheme.SCHEME.SUBMIT_TIME, new DateCell(submitTime));
      row.addFieldValue(ClosedStatsTableScheme.SCHEME.ALARM_ID, new StringCell(alarmId));
      row.addFieldValue(ClosedStatsTableScheme.SCHEME.ALARM_CATEGORY, new StringCell(alarmCategory));
      row.addFieldValue(ClosedStatsTableScheme.SCHEME.SEVERITY, new IntCell(severity));
      row.addFieldValue(ClosedStatsTableScheme.SCHEME.TEXT, new StringCell(text));
      row.addFieldValue(ClosedStatsTableScheme.SCHEME.CLOSE_FLAG, new IntCell(0));
    } else { // change rows value
      final StatsTableRow row = getCloseStatsTableRow(alarmId, alarmCategory);

      if (row != null) {
        row.addFieldValue(ClosedStatsTableScheme.SCHEME.CLOSE_FLAG, new IntCell(1));
        final StatsTableRow closeRow = closedStatsTable.insertRow(row);
        closeRow.addFieldValue(ClosedStatsTableScheme.SCHEME.SUBMIT_TIME, new DateCell(submitTime));
        closeRow.addFieldValue(ClosedStatsTableScheme.SCHEME.ALARM_ID, new StringCell(alarmId));
        closeRow.addFieldValue(ClosedStatsTableScheme.SCHEME.ALARM_CATEGORY, new StringCell(alarmCategory));
        closeRow.addFieldValue(ClosedStatsTableScheme.SCHEME.SEVERITY, null);
        closeRow.addFieldValue(ClosedStatsTableScheme.SCHEME.TEXT, new StringCell(text));
        closeRow.addFieldValue(ClosedStatsTableScheme.SCHEME.CLOSE_FLAG, new IntCell(1));
      }
    }
  }

  private StatsTableRow getCloseStatsTableRow(final String alarmId, final String alarmCategory) {
    for (Iterator iter = closedStatsTable.getRows(); iter.hasNext();) {
      final StatsTableRow row = (StatsTableRow)iter.next();
      if (row.getValueAsString(ClosedStatsTableScheme.SCHEME.ALARM_ID).equals(alarmId) &&
          row.getValueAsString(ClosedStatsTableScheme.SCHEME.ALARM_CATEGORY).equals(alarmCategory))
        return row;
    }
    return null;
  }

  private void processClosedStatsTable() {
    StatsTableRow row;
    final ArrayList rows2Remove = new ArrayList();
    for (Iterator iter = closedStatsTable.getRows(); iter.hasNext(); ) {
      row = (StatsTableRow)iter.next();
      if (row.getValueAsString(ClosedStatsTableScheme.SCHEME.CLOSE_FLAG).equals("0"))
        rows2Remove.add(row);
    }

    for (Iterator iter = rows2Remove.iterator(); iter.hasNext();)
      closedStatsTable.deleteRow((StatsTableRow)iter.next());
  }

  private int clearQuery() {
//    rows.clean();
    allStatsTable.clear();
    openedStatsTable.clear();
    closedStatsTable.clear();
    startPosition = 0;
    totalSize = 0;

    return RESULT_OK;
  }

  private Date convertStringToDate(String date) {
    Date converted = null;
    try {
      SimpleDateFormat formatter = new SimpleDateFormat(DATE_FORMAT);
      converted = formatter.parse(date);
    } catch (ParseException e) {
      logger.error("Invalid date format", e);
    }
    return converted;
  }

  private String convertDateToString(Date date) {
    SimpleDateFormat formatter = new SimpleDateFormat(DATE_FORMAT);
    return formatter.format(date);
  }

  public void setDate(String dateStr) {
    this.date = (dateStr != null && dateStr.trim().length() > 0) ?
            convertStringToDate(dateStr) : null;
  }

  public String getDate() {
    return ((date != null) ? convertDateToString(date) : "");
  }

  public String getMbQuery() {
    return mbQuery;
  }

  public void setMbQuery(String mbQuery) {
    this.mbQuery = mbQuery;
  }

  public String getMbClear() {
    return mbClear;
  }

  public void setMbClear(String mbClear) {
    this.mbClear = mbClear;
  }

  public String getTableFilter() {
    return tableFilter;
  }

  public void setTableFilter(String tableFilter) {
    isTableFilterModified = !tableFilter.equals(this.tableFilter);
    this.tableFilter = tableFilter;
  }

  public StatsTable getTableToShow() {
    return tableToShow;
  }

  public boolean isHasResults() {
    return hasResults;
  }

}
