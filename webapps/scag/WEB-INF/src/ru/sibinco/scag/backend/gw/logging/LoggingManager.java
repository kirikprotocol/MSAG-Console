package ru.sibinco.scag.backend.gw.logging;

import org.apache.log4j.Logger;

import java.io.*;
import java.util.*;

import ru.sibinco.scag.backend.installation.HSDaemon;
import ru.sibinco.scag.backend.installation.SavingStrategy;
import ru.sibinco.lib.SibincoException;

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
      String appender;
      OutputStreamWriter fout = null;
      try {
          appender = extractAppender(destination);
          fout = new OutputStreamWriter(new FileOutputStream(destination));
          PrintWriter pw = new PrintWriter(fout, true);
          pw.write("root=" +cats.get("") + ", default"  + "\n");
          for(Object o : cats.entrySet()){
              final Map.Entry entry = (Map.Entry) o;
              final String catName = (String) entry.getKey();
              final String catPriority = (String) entry.getValue();
              if (catName != null && !catName.trim().equals("") && !catPriority.equalsIgnoreCase("NOTSET")) {
                  pw.write(CAT_PREFIX + catName + "=" + catPriority + "\n");
              }
          }
          pw.write(appender);
          pw.flush();
          pw.close();
      } catch (FileNotFoundException e) {
          logger.error("Cannot find logger.properties file ", e);
      } finally {
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

    public synchronized Map<String, String> readFromLogFile() throws SibincoException{
      logger.debug( "LoggingManager:readFromLogFile()");
      return read( loggerFile );
    }

    public static int PROP_SEPARATOR = '=';

    private Map<String, String> read( File loggerFile ) throws SibincoException {
        logger.info("LoggingManager.read() start");
        Map<String, String> map = new TreeMap<String, String>();
        BufferedReader br;
        try {
            br = new BufferedReader(new FileReader(loggerFile));
        } catch (FileNotFoundException e) {
            logger.error( "LoggingManager.read() FileNotFoundException '" + loggerFile + "'" );
            throw new SibincoException(e);
        }

        try {
            String str;
            String key;
            String value;
            while( (str = br.readLine()) != null ) {
                if( str.startsWith(CAT_PREFIX) ) {
                    key = str.substring( CAT_PREFIX.length(), str.indexOf(PROP_SEPARATOR) );
                    value = str.substring( str.indexOf(PROP_SEPARATOR)+1 );
                    logger.error( "LoggingManager.read() put key='" + key + "' value='" + value + "'" );
                    map.put( key, value );
                }
            }
            logger.info("LoggingManager.read() finish");
        } catch (IOException io) {
            logger.error("LoggingManager.read() IOException. Cannot read from "+loggerFile.getAbsolutePath(),io);
            throw new SibincoException(io);
        } finally {
            try {
                br.close();
            } catch (IOException e) {
                logger.error( "LoggingManager.read() IOException while close BR" );
            }
        }
        return map;
    }

    private String extractAppender(final File destination) throws FileNotFoundException {
        BufferedReader br = new BufferedReader(new FileReader(destination));
        StringBuilder buf = new StringBuilder();
        String str;
        try {
            br.readLine();
            while ((str = br.readLine())!=null) {
                if (!str.startsWith(CAT_PREFIX)) buf.append(str).append("\n");
            }
        } catch (IOException io) {
            logger.error("Cannot read from "+destination.getAbsolutePath(),io);
        } finally {
            try {
                br.close();
            } catch (IOException io) {
                logger.error("Cannot close input stream.", io);
            }
        }
        return buf.toString();
    }

}
