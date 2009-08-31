package ru.novosoft.smsc.infosme.backend.siebel;

import org.apache.log4j.Category;

import java.io.*;
import java.util.LinkedList;
import java.util.regex.Pattern;

import ru.novosoft.smsc.infosme.backend.Message;


/**
 * Created by IntelliJ IDEA.
 * User: bukind
 * Date: 31.08.2009
 * Time: 15:45:33
 * To change this template use File | Settings | File Templates.
 */
public class FinalStateThread extends Thread {

    private static final Category log_ = Category.getInstance(FinalStateThread.class);
    private static final String rollingExtension = ".csv";
    private String path_;
    private Pattern  recSep_;
    private String processedPath_;
    private DataSource dataSource_;
    private boolean started_;

    public FinalStateThread( String path, String processedPath, DataSource dataSource ) throws IOException {
        this.path_ = path;
        this.recSep_ = Pattern.compile(",");
        this.processedPath_ = processedPath;
        this.dataSource_ = dataSource;
        checkPath();
    }

    public void shutdown() {
        started_ = false;
    }

    public void run() {
        started_ = true;
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
            String line;
            int totalRecords = 0;
            LinkedList list = new LinkedList();
            while ( started_ && ( line = is.readLine()) != null ) {
                ++totalRecords;
                // parse line: date,state,taskId,msgId,abnt,userData,taskName
                String[] fields = recSep_.split(line,7);
                if ( fields.length < 7 ) {
                    // not all fields specified
                    if (log_.isDebugEnabled() ) {
                        log_.debug("message " + line + " cannot be parsed");
                    }
                    continue;
                }
                if ( fields[5].length() == 0 ) {
                    if (log_.isDebugEnabled() ) {
                        log_.debug("message " + line + ": userData is empty" );
                    }
                    continue;
                }
                Message.State state = Message.State.getById(new Integer(fields[1]).intValue());
                list.add( new DataSource.FinalStateItem(state.getName(),fields[5]) );
                if ( (totalRecords % 100) == 0 ) {
                    dataSource_.saveFinalStates( list );
                    list.clear();
                }
            }
            if ( list.size() > 0 ) {
                dataSource_.saveFinalStates( list );
            }
            is.close();
            is = null;
            // renaming the file
            fd.renameTo( new File(processedPath_,fileName) );
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

}
