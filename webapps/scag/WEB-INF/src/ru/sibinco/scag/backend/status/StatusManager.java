/*
 * Copyright (c) 2006 SibInco Inc. All Rights Reserved.
 */

package ru.sibinco.scag.backend.status;

import org.apache.log4j.Logger;
import org.xml.sax.SAXException;

import javax.xml.parsers.ParserConfigurationException;
import java.util.*;
import java.io.IOException;
import java.io.File;
import java.text.SimpleDateFormat;

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

    private Calendar calendar = Calendar.getInstance(TimeZone.getTimeZone("GMT"));
    private Calendar localCaledar = Calendar.getInstance(TimeZone.getDefault());
    private SimpleDateFormat dateDirFormat = new SimpleDateFormat(DATE_DIR_FORMAT);
    private SimpleDateFormat dateDayFormat = new SimpleDateFormat(DATE_DAY_FORMAT);
    private SimpleDateFormat dateDayLocalFormat = new SimpleDateFormat(DATE_DAY_FORMAT);


    private static StatusManager instanse;
    private File statusFolder;
    private File currentFile;
    private final Map statMessages = Collections.synchronizedMap(new HashMap());

    private StatusManager() {
    }

    public static synchronized StatusManager getInstance() {
        if (instanse == null)
            instanse = new StatusManager();
        return instanse;
    }

    public synchronized void init(File statFolder) throws IOException, ParserConfigurationException, SAXException {

        dateDirFormat.setTimeZone(TimeZone.getTimeZone("GMT"));
        dateDayFormat.setTimeZone(TimeZone.getTimeZone("GMT"));
        dateDayLocalFormat.setTimeZone(TimeZone.getDefault());

        statusFolder = new File(statFolder.getAbsolutePath() + File.separatorChar + dateDirFormat.format(new Date()));
        if (!statusFolder.exists()) {
            statusFolder.mkdirs();
        }

        currentFile = new File(statusFolder, (dateDayFormat.format(new Date()) + DATE_FILE_EXTENSION));
        if(!currentFile.exists()){
            currentFile.createNewFile();
        }
    }

    public synchronized Map getStatMessages() {
        return statMessages;
    }

    public synchronized void addStatMessages(StatMessage message) {
        statMessages.put(message.getTime(), message);
    }

    public File getStatusFolder() {
        return statusFolder;
    }

    public File getCurrentFile() {
        return currentFile;
    }

}
