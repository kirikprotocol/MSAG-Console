package ru.novosoft.smsc.infosme.backend.siebel;

import org.apache.log4j.Category;

import java.io.*;
import java.util.TreeMap;
import java.util.regex.Pattern;

import ru.novosoft.smsc.infosme.backend.Message;


/**
 * Created by IntelliJ IDEA.
 * User: bukind
 * Date: 31.08.2009
 * Time: 15:45:33
 * To change this template use File | Settings | File Templates.
 */
public class SiebelFinalStateThread extends Thread {

  private static final Category log_ = Category.getInstance(SiebelFinalStateThread.class);
  private static final String rollingExtension = ".csv";
  private String path_;
  private Pattern  recSep_;
  private String processedPath_;
  private DataSource dataSource_;
  private boolean started_;

  public SiebelFinalStateThread( String path, String processedPath, SiebelDataProvider provider ) throws IOException {
    this.path_ = path;
    this.recSep_ = Pattern.compile(",");
    this.processedPath_ = processedPath;
    this.dataSource_ = new DataSourceAdapter( provider, log_ );
    checkPath();
  }

  public void shutdown() {
    started_ = false;
  }

  public boolean isOnline() {
    return started_;
  }

  public void start() {
    if(!started_) {
      super.start();
    }
  }

  public synchronized void run() {
    started_ = true;
    log_.info("final state thread is started");
    try {
      while ( started_ ) {
        File dir = checkPath();
        String[] files = dir.list();
        for ( int i = 0; i < files.length; ++i ) {
          if ( files[i].endsWith( rollingExtension ) ) {
            processFile(files[i]);
          }
        }
        Thread.sleep(10000);
      }
      log_.info("final state thread is shutdowned");
    } catch ( Exception exc ) {
      log_.error("exc in run: " + exc.getMessage() );
      started_ = false;
    }
  }

  public void processFile( String fileName ) {
    File fd = new File(path_,fileName);
    BufferedReader is = null;
    try {
      is = new BufferedReader( new FileReader(fd) );
      String line = is.readLine();
      if ( line == null ) throw new IOException("heading line is not found");
      int totalRecords = 0;
      int processedRecords = 0;
      TreeMap map = new TreeMap();
      while ( started_ && ( line = is.readLine()) != null ) {
        // parse line: date,state,taskId,msgId,smppStatus,abnt,userData,taskName
        ++totalRecords;
        String[] fields = recSep_.split(line,8);
          /*
        if ( log_.isDebugEnabled() ) {
          StringBuffer sb = new StringBuffer();
          for ( int i = 0; i < fields.length; ++i ) {
            if ( sb.length() > 0 ) sb.append(",");
            sb.append("<" + fields[i] + ">");
          }
          log_.debug("line parsed into " + fields.length + " pieces:" + sb.toString());
        }
        */
        if ( fields.length < 8 ) {
          // not all fields specified
          if (log_.isDebugEnabled() ) {
            log_.debug("message " + line + " cannot be parsed");
          }
          continue;
        }
        if ( fields[6].length() != 0 ) {
          Message.State state = Message.State.getById(new Integer(fields[1]).intValue());
          SiebelMessage.State siebelState = stateToSiebelState(state);
          String smppCodeDescription = ""; // FIXME: ask Artem where to take smpp description from smpp code
          map.put( fields[6], new SiebelMessage.DeliveryState(siebelState,fields[4],smppCodeDescription) );
          if ( (totalRecords - processedRecords) > 100 ) {
            dataSource_.saveFinalStates( map );
            map.clear();
            processedRecords = totalRecords;
          }
        } else if ( fields[1].equals("0") && fields[3].equals("0") ) {
          // end of messages in the task
          if (log_.isInfoEnabled()) {
            log_.info("all messages have been processed for task " + fields[7]);
          }
          if (map.size() > 0) {
            dataSource_.saveFinalStates(map);
            map.clear();
            processedRecords = totalRecords;
          }
          dataSource_.taskHasFinished( fields[7] );
        } else {
          // if (log_.isDebugEnabled() ) {
          //   log_.debug("message " + line + ": userData is empty" );
          // }
        }
      }
      if ( map.size() > 0 ) {
        dataSource_.saveFinalStates( map );
      }
      is.close();
      is = null;
      // renaming the file
      fd.renameTo( new File(processedPath_,fileName) );
      if ( log_.isInfoEnabled() ) {
        log_.info("file processed " + fileName + ", msg records=" + totalRecords );
      }
    } catch ( IOException e ) {
      log_.error( "exc processing file " + fileName + ": " + e.getMessage() );
    } finally {
      if ( is != null ) {
        try {
          is.close();
        } catch ( IOException e ) {
          log_.error( "exc: " + e.getMessage());
        }
      }
    }
  }

  private File checkPath() throws IOException {
    File dir = new File(path_);
    if ( !dir.exists() ) {
      throw new IOException( "path " + path_ + " does not exist" );
    } else if ( ! dir.isDirectory() ) {
      throw new IOException( "path " + path_ + " is not a directory" );
    }
    return dir;
  }

  private SiebelMessage.State stateToSiebelState( Message.State state )
  {
    SiebelMessage.State siebelState;
    if ( state == Message.State.DELIVERED ) {
      siebelState = SiebelMessage.State.DELIVERED;
    } else if ( state == Message.State.EXPIRED ) {
      siebelState = SiebelMessage.State.EXPIRED;
    } else if ( state == Message.State.DELETED ) {
      siebelState = SiebelMessage.State.DELETED;
    } else if ( state == Message.State.FAILED ) {
      siebelState = SiebelMessage.State.ERROR;
    } else {
      siebelState = SiebelMessage.State.UNKNOWN;
    }
    return siebelState;
  }

}
