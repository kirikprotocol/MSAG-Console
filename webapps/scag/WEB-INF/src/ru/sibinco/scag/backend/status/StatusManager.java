/*
 * Copyright (c) 2006 SibInco Inc. All Rights Reserved.
 */

package ru.sibinco.scag.backend.status;

import org.apache.log4j.Logger;
import org.xml.sax.SAXException;

import javax.xml.parsers.ParserConfigurationException;
import java.util.*;
import java.io.*;
import java.text.SimpleDateFormat;
import java.text.ParseException;

import ru.sibinco.scag.backend.Constants;

/**
 * The <code>StatusManager</code> class represents
 * <p><p/>
 * Date: 27.01.2006
 * Time: 15:02:06
 *
 * @author &lt;a href="mailto:igor@sibinco.ru"&gt;Igor Klimenko&lt;/a&gt;
 */
public class StatusManager {

    private Logger logger = Logger.getLogger(this.getClass());
    private final static String DATE_DIR_FORMAT = "yyyy-MM";
    private final static String DATE_DAY_FORMAT = "yyyy-MM-dd";
    private final static String DATE_FILE_EXTENSION = ".log";

    private final static Calendar calendar = Calendar.getInstance(TimeZone.getTimeZone("GMT"));
    private SimpleDateFormat dateDirFormat = new SimpleDateFormat(DATE_DIR_FORMAT);
    private SimpleDateFormat dateDayFormat = new SimpleDateFormat(DATE_DAY_FORMAT);
    private SimpleDateFormat timeStatusFormat = new SimpleDateFormat(Constants.TIME_STATUS_RECORD_FORMAT);


    private static StatusManager instanse;
    private File statusFolder;
    private ArrayList statMessages = new ArrayList();
    private static long showInterval = Constants.Day;

    private StatusManager() {
    }

    public static synchronized StatusManager getInstance() {
        if (instanse == null)
            instanse = new StatusManager();
        return instanse;
    }

    public synchronized void init(String statFolder, String time) throws IOException, ParserConfigurationException, SAXException {

        dateDirFormat.setTimeZone(TimeZone.getTimeZone("GMT"));
        dateDayFormat.setTimeZone(TimeZone.getTimeZone("GMT"));
        timeStatusFormat.setTimeZone(TimeZone.getDefault());

        statusFolder = new File(statFolder);
        if (!statusFolder.exists()) {
            statusFolder.mkdirs();
        }
        showInterval = Long.decode(time).longValue();
        statMessages = getStatMessages(getCurrentFile().getAbsolutePath());
    }

    public synchronized ArrayList getStatMessages() {
        return statMessages;
    }

    public synchronized void addStatMessages(StatMessage message) {
        checkMessages(statMessages);
        statMessages.add(message);
        writeToLog(message);
    }

    private File getStatusFolder() {
        return statusFolder;
    }

    private File getCurrentFile() {
        File currentFile;
        currentFile = new File(getMonthDir(), (dateDayFormat.format(new Date()) + DATE_FILE_EXTENSION));
        if (!currentFile.exists()) {
            try {
                currentFile.createNewFile();
            } catch (IOException e) {
                logger.error("IOException: " + e.getMessage());
            }
        }
        return currentFile;
    }

    private File getMonthDir() {
        File monthDir = new File(getStatusFolder().getAbsolutePath() + File.separatorChar + dateDirFormat.format(new Date()));
        if (!monthDir.exists()) {
            monthDir.mkdirs();
        }
        return monthDir;
    }

    private File getPreviousDayFile() {
        File previousFile = null;
        Date date = new Date((new Date().getTime() - Constants.Day));
        previousFile = new File(getMonthDir(), (dateDayFormat.format(date) + DATE_FILE_EXTENSION));
        if (previousFile.exists()) {
            return previousFile;
        }
        return null;
    }

    private ArrayList getStatMessages(String filename) {
        ArrayList result = new ArrayList();
        File previousFile = getPreviousDayFile();
        if (previousFile != null) {
            getPreviousStatMessages(previousFile, result);
        }
        FileReader filereader;
        BufferedReader bufferedreader;
        String line;
        File fileToRead = new File(filename);
        if (!fileToRead.exists()) {
            logger.debug("File: " + filename + " doesn't exist.");
            try{
                if (fileToRead.createNewFile()) {
                    logger.debug("Create file: "+filename);
                } 
            } catch (IOException e){
                throw new NullPointerException("File: " + filename + " could not create!");
            }
        }
        try {
            filereader = new FileReader(fileToRead);
            bufferedreader = new BufferedReader(filereader);
            while ((line = bufferedreader.readLine()) != null) {
                LineParser lineParser = new LineParser(line);
                StatMessage statMessages = new StatMessage();
                while (lineParser.hasMoreTokens()) {
                    statMessages.setTime(lineParser.nextToken().trim());
                    statMessages.setUserLogin(lineParser.nextToken().trim());
                    statMessages.setCategory(lineParser.nextToken().trim());
                    statMessages.setMessage(lineParser.nextToken().trim());
                }
                result.add(statMessages);
            }
            filereader.close();
        } catch (FileNotFoundException e) {
            logger.error("The file not foun: " + e.getMessage());
        } catch (IOException e) {
            logger.error("IOException: " + e.getMessage());
        }
        return result;
    }

    private ArrayList getPreviousStatMessages(File fileToRead, ArrayList result) {
        FileReader filereader;
        BufferedReader bufferedreader;
        String line;

        try {
            filereader = new FileReader(fileToRead);
            bufferedreader = new BufferedReader(filereader);
            while ((line = bufferedreader.readLine()) != null) {
                LineParser lineParser = new LineParser(line);
                boolean isAdd = true;
                StatMessage statMessages = new StatMessage();
                while (lineParser.hasMoreTokens()) {
                    String time = lineParser.nextToken().trim();
                    Date date = timeStatusFormat.parse(time);
                    if (!(date.getTime() >= getIntervalTime())) {
                        isAdd = false;
                    }
                    statMessages.setTime(time);
                    statMessages.setUserLogin(lineParser.nextToken().trim());
                    statMessages.setCategory(lineParser.nextToken().trim());
                    statMessages.setMessage(lineParser.nextToken().trim());

                }
                if (isAdd) {
                    result.add(statMessages);
                }
            }
            filereader.close();
        } catch (FileNotFoundException e) {
            logger.error("The file not foun: " + e.getMessage());
        } catch (IOException e) {
            logger.error("IOException: " + e.getMessage());
        } catch (ParseException e) {
            logger.error("ParseException: " + e.getMessage());
        }
        return result;
    }

    private synchronized void checkMessages(ArrayList statMessages) {

        StatMessage[] messages = (StatMessage[]) statMessages.toArray(new StatMessage[statMessages.size()]);
        for (int i = 0; i < messages.length; i++) {
            StatMessage message = messages[i];
            Date date = null;
            try {
                date = timeStatusFormat.parse(message.getTime());
            } catch (ParseException e) {
                logger.error("ParseException: " + e.getMessage());
            }
            if ((date != null) && (date.getTime() < getIntervalTime())) {
                statMessages.remove(message);
            }
        }
    }

    private long getIntervalTime() {
        Date date = new Date();
        Calendar calendarGMT = Calendar.getInstance(TimeZone.getTimeZone("GMT"));
        SimpleDateFormat dateFormatGMT = new SimpleDateFormat(Constants.TIME_STATUS_RECORD_FORMAT);
        dateFormatGMT.setTimeZone(TimeZone.getTimeZone("GMT"));
        synchronized (calendar) {
            calendarGMT.setTimeInMillis(calendar.getTimeInMillis() - getShowInterval());
            try {
                date = timeStatusFormat.parse(dateFormatGMT.format(calendarGMT.getTime()));
            } catch (ParseException e) {
                logger.error("Parse Exception: " + e.getMessage());
            }
        }
        return date.getTime();
    }

    private synchronized void writeToLog(StatMessage message) {
        RandomAccessFile raf = null;
        try {
            raf = new RandomAccessFile(getCurrentFile(), "rw");
            Writer out = new OutputStreamWriter(new FileOutputStream(raf.getFD()), "UTF-8");
            // Seek to end of file
            raf.seek(getCurrentFile().length());
            out.write(message.stringToLog() + "\n");
            out.flush();
            out.close();
            raf.close();
        } catch (FileNotFoundException e) {
            logger.error("The file not found: " + e.getMessage());
        } catch (IOException e) {
            logger.error("IOException: " + e.getMessage());
        } finally {
            try {
                if (raf != null) {
                    raf.close();
                }
            }
            catch (IOException ex) {
                logger.error("IOException: " + ex.getMessage());
            }
        }
    }

    private static boolean equalsTime(java.util.Date aFirstDate, java.util.Date aSecondDate) {
        synchronized (calendar) {
            calendar.setTime(aFirstDate);
            int hour = calendar.get(Calendar.HOUR_OF_DAY);
            int minute = calendar.get(Calendar.MINUTE);
            calendar.setTime(aSecondDate);
            return (hour == calendar.get(Calendar.HOUR_OF_DAY))
                    && (minute == calendar.get(Calendar.MINUTE));
        }
    }

    private static class LineParser {
        private String _csvLine = null;
        private List _tokens = null;
        private int _currentToken = 0;

        protected LineParser() {
        }

        protected LineParser(String csvLine) {
            _csvLine = csvLine;
            _tokens = new ArrayList();
            tokenize();
        }

        protected boolean hasMoreTokens() {
            return (_currentToken < _tokens.size());
        }

        protected String nextToken() {
            String token = (String) _tokens.get(_currentToken);
            _currentToken++;
            return token;
        }

        protected void tokenize() {
            char[] characters = _csvLine.toCharArray();
            StringBuffer buffer = new StringBuffer();
            boolean inSingleQuote = false;
            boolean inDoubleQuote = false;

            for (int i = 0; i < characters.length; i++) {
                try {
                    char currentCharacter = characters[i];

                    if (currentCharacter == '"') {
                        char nextCharacter = characters[i + 1];
                        if (nextCharacter == '"') {
                            buffer.append(nextCharacter);
                            i += 1;
                        } else {
                            inDoubleQuote = !inDoubleQuote;
                        }
                    } else if (currentCharacter == ',') {
                        if (!inSingleQuote && !inDoubleQuote) {
                            _tokens.add(buffer.toString());
                            buffer = new StringBuffer();
                            // break;
                        } else {
                            buffer.append(currentCharacter);
                        }
                    } else {
                        buffer.append(currentCharacter);
                    }
                } catch (ArrayIndexOutOfBoundsException aioobe) {
                    _tokens.add(buffer.toString());
                    buffer = new StringBuffer();
                }
            }
            if (buffer.length() > 0) {
                _tokens.add(buffer.toString());
            }
        }
    }

    public static long getShowInterval() {
        return showInterval;
    }
}
