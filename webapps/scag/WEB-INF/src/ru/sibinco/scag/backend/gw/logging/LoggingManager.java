/*
 * Copyright (c) 2006 SibInco Inc. All Rights Reserved.
 */

package ru.sibinco.scag.backend.gw.logging;

import org.apache.log4j.Logger;

import java.io.*;
import java.util.Iterator;
import java.util.Map;

/**
 * The <code>LoggingManager</code> class represents
 * <p><p/>
 * Date: 23.08.2006
 * Time: 12:30:36
 *
 * @author &lt;a href="mailto:igor@sibinco.ru"&gt;Igor Klimenko&lt;/a&gt;
 */
public class LoggingManager {

    private Logger logger = Logger.getLogger(this.getClass());

    private File loggerFile;

    public LoggingManager(final String lfile) throws IOException {
        loggerFile = new File(lfile);
    }

    public synchronized void writeToLog(final Map cats) {
        OutputStreamWriter fout = null;
        try {
            fout = new OutputStreamWriter(new FileOutputStream(loggerFile));
            PrintWriter pw = new PrintWriter(fout, true);
            pw.write("root=" +cats.get("") + ", default"  + "\n");
            for (Iterator iterator = cats.entrySet().iterator(); iterator.hasNext();) {
                final Map.Entry entry = (Map.Entry) iterator.next();
                final String catName = (String) entry.getKey();
                final String catPriority = (String) entry.getValue();
                if (catName != null && !catName.trim().equals("") && !catPriority.equalsIgnoreCase("NOTSET")) {
                    pw.write("cat." + catName + "=" + catPriority  + "\n");
                }
            }
            pw.write("\n");
            printAppender(pw);
            pw.flush();
            pw.close();
        } catch (FileNotFoundException e) {
            logger.error("Cannot find logger.properties file ", e);
        }
        finally {
            try {
                if (fout != null) {
                    fout.close();
                }
            }
            catch (IOException ex) {
                logger.error("Cannot close output stream.", ex);
            }
        }
    }

    private synchronized void printAppender(PrintWriter pw) {
        pw.write("appender.default.file.name=logs/scag.log" + "\n");
        pw.write("appender.default.file.maxsize=50Mb" + "\n");
        pw.write("appender.default.file.maxindex=4" + "\n");
    }
}
