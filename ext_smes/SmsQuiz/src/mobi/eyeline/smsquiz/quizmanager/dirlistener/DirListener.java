package mobi.eyeline.smsquiz.quizmanager.dirlistener;

import mobi.eyeline.smsquiz.quizmanager.dirlistener.Notification;
import mobi.eyeline.smsquiz.quizmanager.dirlistener.QuizFile;
import mobi.eyeline.smsquiz.quizmanager.QuizException;

import java.io.File;
import java.io.FilenameFilter;
import java.util.Observable;
import java.util.Map;
import java.util.HashMap;
import java.util.regex.Pattern;
import java.util.regex.Matcher;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;
import java.util.concurrent.locks.Condition;

import org.apache.log4j.Logger;

public class DirListener extends Observable implements Runnable{


    private static Logger logger = Logger.getLogger(DirListener.class);
	private Map<String, QuizFile> filesMap;
    private String quizDir;
    private FilenameFilter fileFilter;

    public DirListener(final String quizDir) throws QuizException {
        if(quizDir==null) {
            throw new QuizException("Some arguments are null", QuizException.ErrorCode.ERROR_WRONG_REQUEST);
        }
        this.quizDir = quizDir;
        File file = new File(quizDir);
        if(!file.exists()) {
            logger.info("Create quizDir");
            file.mkdirs();
        }
        filesMap = new HashMap<String,QuizFile>();
        fileFilter = new XmlFileFilter();
    }

    public synchronized void run() {
        logger.info("Running DirListener...");
        File dirQuiz = new File(quizDir);

        File[] files = dirQuiz.listFiles(fileFilter);

        for(File f:files) {

            String fileName = f.getAbsolutePath();
            long lastModified = f.lastModified();

            try {
                QuizFile existFile;

                if((existFile = filesMap.get(fileName))!=null) {
                    if(existFile.getLastModified()<lastModified) {
                        notifyObservers( new Notification ( fileName, Notification.FileStatus.MODIFIED ) );
                        existFile.modifyDate(lastModified);
                        if(logger.isInfoEnabled()) {
                            logger.info("Quiz file modified: "+fileName);
                        }
                    }
                }
                else {
                    filesMap.put(fileName, new QuizFile(fileName,lastModified) );
                    setChanged();
                    notifyObservers( new Notification( fileName, Notification.FileStatus.CREATED ) );
                    if(logger.isInfoEnabled()) {
                        logger.info("Quiz file created: "+fileName);
                    }
                }

            } catch (QuizException e) {
                logger.error("Error construct quiz file or notification", e);
            }

        }
        logger.info("DirListener finished...");
    }

    public synchronized void remove(String fileName, boolean rename) throws QuizException{
        if(fileName == null) {
            throw new QuizException("Some arguments are null", QuizException.ErrorCode.ERROR_WRONG_REQUEST);
        }
        if(rename) {
            File file = new File(fileName);
            file.renameTo(new File(fileName+".old"));
        }
        filesMap.remove(fileName);
    }

    public int countFiles() {
        return filesMap.size();
    }

    private class XmlFileFilter implements FilenameFilter {

        public boolean accept(File dir, String name) {
            Pattern p = Pattern.compile("(.*\\.xml)");
            Matcher matcher = p.matcher(name);
            return matcher.matches();
        }
    }
	 
}
 
