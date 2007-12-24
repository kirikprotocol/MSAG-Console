/*
 * Copyright (c) 2006 SibInco Inc. All Rights Reserved.
 */

package ru.sibinco.scag.backend.gw.logging;

import org.apache.log4j.Logger;

import java.io.*;
import java.util.*;

import ru.sibinco.scag.backend.installation.HSDaemon;
import ru.sibinco.scag.backend.installation.SavingStrategy;
import ru.sibinco.lib.SibincoException;

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
    private static final String CAT_PREFIX = "cat.";
    private final HSDaemon hsDaemon;
    private File loggerFile;

    public LoggingManager(final String lfile, HSDaemon hsDaemon){
        loggerFile = new File(lfile);
        if(!loggerFile.exists()){
            logger.error("Cannot find file: " + loggerFile.getAbsolutePath());
        }
        this.hsDaemon = hsDaemon;
    }

    private void write(final Map cats , final File destination) throws SibincoException{
      String appender = null;
      OutputStreamWriter fout = null;
      try {
          appender = extractAppender(destination);
          fout = new OutputStreamWriter(new FileOutputStream(destination));
          PrintWriter pw = new PrintWriter(fout, true);
          pw.write("root=" +cats.get("") + ", default"  + "\n");
          for (Iterator iterator = cats.entrySet().iterator(); iterator.hasNext();) {
              final Map.Entry entry = (Map.Entry) iterator.next();
              final String catName = (String) entry.getKey();
              final String catPriority = (String) entry.getValue();
              if (catName != null && !catName.trim().equals("") && !catPriority.equalsIgnoreCase("NOTSET")) {
                  pw.write(CAT_PREFIX + catName + "=" + catPriority  + "\n");
              }
          }
          pw.write(appender);
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

    public synchronized void writeToLog(final Map cats) throws SibincoException{
      write(cats, loggerFile);
      hsDaemon.store(loggerFile, new SavingStrategy() {
          public void storeToMirror(File mirrorFile) throws SibincoException {
           write(cats, mirrorFile);
          }
      });
    }

    public synchronized Map readFromLogFile() throws SibincoException{
        logger.debug( "LoggingManager:readFromLogFile()");
      return read( loggerFile );
    }

    public static int PROP_SEPARATOR = '=';

    private Map read( File loggerFile ){
        Map map = new TreeMap();
        BufferedReader br = null;
        try {
            br = new BufferedReader(new FileReader(loggerFile));
        } catch (FileNotFoundException e) {
            logger.error( "" );
        }
        StringBuffer buf = new StringBuffer();
            String str;
        try {
            br.readLine();
            String key;
            String value;
            while ((str = br.readLine())!=null) {
                if (str.startsWith(CAT_PREFIX)) {
                    key = str.substring( CAT_PREFIX.length(), str.indexOf(PROP_SEPARATOR) );
                    value = str.substring( str.indexOf(PROP_SEPARATOR)+1);
                    map.put( key, value );
                }
          }
        } catch (IOException io) {
            logger.error("Cannot read from "+loggerFile.getAbsolutePath(),io);
        } finally {
            return map;
        }
    }

    private String extractAppender(final File destination) throws FileNotFoundException {
      BufferedReader br = new BufferedReader(new FileReader(destination));
      StringBuffer buf = new StringBuffer();
      String str;
      try {
        br.readLine();
        while ((str = br.readLine())!=null) {
          if (!str.startsWith(CAT_PREFIX)) buf.append(str).append("\n");
        }
      } catch (IOException io) {
        logger.error("Cannot read from "+destination.getAbsolutePath(),io);
      } finally {
        if (br!=null)
          try {
           br.close();
          } catch (IOException io) {
             logger.error("Cannot close input stream.", io);
          }
      }
      return buf.toString();
    }
}
