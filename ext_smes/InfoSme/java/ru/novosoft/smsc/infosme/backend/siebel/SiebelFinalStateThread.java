package ru.novosoft.smsc.infosme.backend.siebel;

import org.apache.log4j.Category;
import ru.novosoft.smsc.infosme.backend.Message;
import ru.novosoft.smsc.util.LocaleMessages;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileReader;
import java.io.IOException;
import java.text.ParseException;
import java.text.SimpleDateFormat;
import java.util.*;
import java.util.regex.Pattern;


/**
 * Created by IntelliJ IDEA.
 * User: bukind
 * Date: 31.08.2009
 * Time: 15:45:33
 * To change this template use File | Settings | File Templates.
 */
public class SiebelFinalStateThread {

  private static final Category log_ = Category.getInstance(SiebelFinalStateThread.class);
  private static final String rollingExtension = ".csv";
  private final String path_;
  private final Pattern recSep_;
  private final String processedPath_;
  private final DataSource dataSource_;
  private boolean started_;
  private Thread currentThread;

  public SiebelFinalStateThread(String path, String processedPath, SiebelDataProvider provider) throws IOException {
    this.path_ = path;
    this.recSep_ = Pattern.compile(",");
    this.processedPath_ = processedPath;
    this.dataSource_ = new DataSourceAdapter(provider, log_);
    checkPath();
  }

  /** @noinspection EmptyCatchBlock*/
  public void shutdown() {
    started_ = false;
    if (currentThread != null) {
      currentThread.interrupt();
      try {
        currentThread.join();
      } catch (InterruptedException e) {
      }
    }
  }

  public void start() {
    if (!started_) {
      currentThread = new Thread(new Runnable() {
        public synchronized void run() {
          started_ = true;
          log_.info("final state thread is started");
          while (started_) {
            try {
              File dir = checkPath();
              String[] files = dir.list();

              for (int i = 0; i < files.length; ++i) {
                if (files[i].endsWith(rollingExtension)) {
                  processFile(files[i]);
                  archivateFile(files[i]);
                }
              }

            } catch (Exception exc) {
              log_.error("exc in run: " + exc.getMessage(), exc);
            }

            try {
              Thread.sleep(10000);
            } catch (InterruptedException ignored) {
            }
          }
          log_.info("final state thread is shutdowned");
        }
      });
      currentThread.start();
    }
  }

  void archivateFile(String fileName) {
    File fd = new File(path_, fileName);
    try {
      Date fileDate = sdf.parse(fd.getName().substring(0, fd.getName().length() - rollingExtension.length()));
      Calendar c = new GregorianCalendar();
      c.setTime(fileDate);
      String dirStr = new StringBuffer(processedPath_.length() + 15).append(processedPath_).append(File.separatorChar).append(c.get(Calendar.YEAR)).
          append(File.separatorChar).append(c.get(Calendar.MONTH) + 1).append(File.separatorChar).
          append(c.get(Calendar.DAY_OF_MONTH)).append(File.separatorChar).append(c.get(Calendar.HOUR_OF_DAY)).toString();
      File dir = new File(dirStr);
      if (!dir.exists() && !dir.mkdirs()) {
        log_.error("Can't create dir: " + dir.getAbsolutePath());
      } else if (!fd.renameTo(new File(dir, fileName))) {
        log_.error("Can't rename file " + fd.getAbsolutePath() + " to processed");
      }

    } catch (ParseException e) {
      log_.error(e,e);
    }
  }


  void processFile(String fileName) throws SiebelException {
    File fd = new File(path_, fileName);
    BufferedReader is = null;
    try {
      is = new BufferedReader(new FileReader(fd));
      String line = is.readLine();
      if (line == null)
        return;

      int totalRecords = 0;
      int processedRecords = 0;
      TreeMap map = new TreeMap();
      while (started_ && (line = is.readLine()) != null) {
        // parse line: date,state,taskId,msgId,smppStatus,abnt,userData,taskName
        ++totalRecords;
        String[] fields = recSep_.split(line, 8);

        if (fields.length < 8) {
          // not all fields specified
          if (log_.isDebugEnabled()) {
            log_.error("message " + line + " cannot be parsed");
          }
          continue;
        }
        if (fields[7] == null || !fields[7].startsWith("siebel_")) {
          if (log_.isDebugEnabled()) {
            log_.debug("message " + line + " is not for siebel task");
          }
          continue;
        }
        if (fields[6].length() != 0) {
          Message.State state = Message.State.getById(Integer.parseInt(fields[1]));
          SiebelMessage.State siebelState = stateToSiebelState(state);
          String smppCodeDescription = LocaleMessages.getString(Locale.ENGLISH, "smsc.errcode." + fields[4]);
          map.put(fields[6], new SiebelMessage.DeliveryState(siebelState, fields[4], smppCodeDescription));
          if ((totalRecords - processedRecords) > 100) {
            dataSource_.saveFinalStates(map);
            map.clear();
            processedRecords = totalRecords;
          }
        } else if (fields[1].equals("0") && fields[3].equals("0")) {
          // end of messages in the task
          if (map.size() > 0) {
            dataSource_.saveFinalStates(map);
            map.clear();
            processedRecords = totalRecords;
          }
          String taskName = fields[7];
          String wave = taskName.substring(7);                           // "siebel_"
          int i = wave.indexOf('(');
          if (i >= 0) {
            wave = wave.substring(0, i);
          }
          if (!dataSource_.hasUnfinished(wave)) {
            if (log_.isDebugEnabled()) {
              log_.debug("Siebel: Task has finished waveId=" + wave);
            }
            dataSource_.taskHasFinished(wave);
          }
        }
      }
      if (map.size() > 0) {
        dataSource_.saveFinalStates(map);
      }

      if (log_.isInfoEnabled()) {
        log_.info("file processed " + fileName + ", msg records=" + totalRecords);
      }

    } catch (IOException e) {
      log_.error("exc processing file " + fileName + ": " + e.getMessage(), e);
    } finally {
      if (is != null) {
        try {
          is.close();
        } catch (IOException e) {
          log_.error("exc: " + e.getMessage());
        }
      }
    }
  }

  private static final SimpleDateFormat sdf = new SimpleDateFormat("yyyyMMddHHmmss");

  private File checkPath() throws IOException {
    File dir = new File(path_);
    if (!dir.exists()) {
      throw new IOException("path " + path_ + " does not exist");
    } else if (!dir.isDirectory()) {
      throw new IOException("path " + path_ + " is not a directory");
    }
    return dir;
  }

  private SiebelMessage.State stateToSiebelState(Message.State state) {
    SiebelMessage.State siebelState;
    if (state == Message.State.DELIVERED) {
      siebelState = SiebelMessage.State.DELIVERED;
    } else if (state == Message.State.EXPIRED) {
      siebelState = SiebelMessage.State.EXPIRED;
    } else if (state == Message.State.DELETED) {
      siebelState = SiebelMessage.State.DELETED;
    } else if (state == Message.State.FAILED) {
      siebelState = SiebelMessage.State.ERROR;
    } else {
      siebelState = SiebelMessage.State.UNKNOWN;
    }
    return siebelState;
  }

}
