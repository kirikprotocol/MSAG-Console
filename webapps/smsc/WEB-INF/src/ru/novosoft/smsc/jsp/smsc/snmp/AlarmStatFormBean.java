package ru.novosoft.smsc.jsp.smsc.snmp;

import ru.novosoft.smsc.jsp.SMSCErrors;
import ru.novosoft.smsc.jsp.smsc.IndexBean;
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

    private Calendar calendar = Calendar.getInstance(TimeZone.getTimeZone("UTC"));
    private Calendar localCalendar = Calendar.getInstance(TimeZone.getDefault());
    private SimpleDateFormat dateFileFormat = new SimpleDateFormat(DATE_FILE_FORMAT);
    private SimpleDateFormat rowSubmitTimeFormat = new SimpleDateFormat(ROW_SUBMIT_TIME_FORMAT);

    private static final String ALARM_STAT_CSV_FILE_PARAM = "snmp.csvFileDir";

    private AlarmStatSet rows = new AlarmStatSet();
    private Date date = Functions.truncateTime(new Date());

    private String mbQuery = null;
    private String mbClear = null;

    public AlarmStatRow getRow(int index) {
        return rows == null ? null : rows.getRow(index);
    }

    public int process(HttpServletRequest request) {
        int result = super.process(request);
        if (result != RESULT_OK) return result;

        if (pageSize == 0) pageSize = 20;
        if (sort == null || sort.length() == 0)
            sort = "submit_time";

        Config webConfig = appContext.getConfig();

        if (mbQuery != null)
            result = processQuery(webConfig);
        else if (mbClear != null)
            result = clearQuery();

        mbQuery = null;
        mbClear = null;

        rows.sort(sort);

        return result;
    }

    public int processQuery(Config config) {
        rows.clean();
        startPosition = 0;
        totalSize = 0;
        try {
            String fileDirName = config.getString(ALARM_STAT_CSV_FILE_PARAM);
            localCalendar.setTime(date);
            localCalendar.setTimeZone(calendar.getTimeZone());
            Date utcTime = localCalendar.getTime();
            String dateFilePrefix = dateFileFormat.format(utcTime);

            File dirNameFile = new File(fileDirName);
            File[] dirFiles = dirNameFile.listFiles();
            if (dirFiles == null || dirFiles.length == 0) return RESULT_OK;

            for (int j = 0; j < dirFiles.length; j++) {
                String fileName = dirFiles[j].getName();
                if (fileName == null || fileName.length() <= 0 ||
                        !fileName.toLowerCase().startsWith(dateFilePrefix)) continue;

                rows.addAll(processFile(new File(dirNameFile, fileName)));
            }
            startPosition = 0;
            totalSize = (rows == null) ? 0 : rows.getRowsCount();
            return RESULT_OK;
        }
        catch (ParseException e) {
            e.printStackTrace();
            return error(SMSCErrors.error.snmp.CSVisInvalid, e.getMessage());
        }
        catch (FileNotFoundException e) {
            e.printStackTrace();
            return error(SMSCErrors.error.snmp.InitFailed, e.getMessage());
        }
        catch (IOException e) {
            e.printStackTrace();
            return error(SMSCErrors.error.snmp.InitFailed, e.getMessage());
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

    private Collection processFile(File src) throws FileNotFoundException, IOException, ParseException {
        Collection result = new ArrayList();
        BufferedReader br = new BufferedReader(new FileReader(src));
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
        while (st.ttype != StreamTokenizer.TT_EOF) {
            AlarmStatRow row = new AlarmStatRow();
            row.setSubmit_time(rowSubmitTimeFormat.parse(st.sval));
            st.nextToken();
            row.setAlarm_id(st.sval);
            st.nextToken();
            row.setAlarm_category(st.sval);
            st.nextToken();
            row.setSeverity((int) st.nval);
            st.nextToken();
            row.setText(st.sval);
            result.add(row);
            st.nextToken();
            if (st.ttype == StreamTokenizer.TT_EOL) st.nextToken();
        }
        return result;
    }

    public int clearQuery() {
        rows.clean();
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
}
