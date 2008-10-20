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
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;
import java.util.concurrent.locks.Condition;

import org.apache.log4j.Logger;

public class DirListener extends Observable implements Runnable {


  private static final Logger logger = Logger.getLogger(DirListener.class);
  private Map<String, QuizFile> filesMap;
  private String quizDir;
  private FilenameFilter fileFilter;
  private Lock lock;
  private Condition notRemove;
  private Condition notRun;
  private static final Pattern PATTERN = Pattern.compile("(.*\\.xml)");
  private int run = -1;
  private int remove =-1;

  public DirListener(final String quizDir) throws QuizException {
    if (quizDir == null) {
      throw new QuizException("Some arguments are null", QuizException.ErrorCode.ERROR_WRONG_REQUEST);
    }
    this.quizDir = quizDir;
    File file = new File(quizDir);
    if (!file.exists()) {
      logger.info("Create quizDir");
      file.mkdirs();
    }
    filesMap = new HashMap<String, QuizFile>();
    fileFilter = new XmlFileFilter();

    lock = new ReentrantLock();
    notRemove = lock.newCondition();
    notRun = lock.newCondition();
  }

  public void run() {
    logger.info("Running DirListener...");
    File dirQuiz = new File(quizDir);

    try {
      lock.lock();
      if(remove==1) {
        notRemove.await();
      }
      run=1;

      File[] files = dirQuiz.listFiles(fileFilter);
      for (File f : files) {

        String fileName = f.getAbsolutePath();
        long lastModified = f.lastModified();
        QuizFile existFile;

        if ((existFile = filesMap.get(fileName)) != null) {
          if (existFile.getLastModified() < lastModified) {
            notifyObservers(new Notification(fileName, Notification.FileStatus.MODIFIED));
            existFile.modifyDate(lastModified);
            if (logger.isInfoEnabled()) {
              logger.info("Quiz file modified: " + fileName);
            }
          }
        } else {
          filesMap.put(fileName, new QuizFile(fileName, lastModified));
          setChanged();
          notifyObservers(new Notification(fileName, Notification.FileStatus.CREATED));
          if (logger.isInfoEnabled()) {
            logger.info("Quiz file created: " + fileName);
          }
        }
        run=0;
        notRun.signal();
      }
    } catch (Exception e) {
      logger.error("Error construct quiz file or notification", e);
    } finally {
      lock.unlock();
    }

    logger.info("DirListener finished...");
  }

  public void remove(String fileName, boolean rename) throws QuizException {
    if (fileName == null) {
      throw new QuizException("Some arguments are null", QuizException.ErrorCode.ERROR_WRONG_REQUEST);
    }
    try{
      lock.lock();
      if(run==1) {
        notRun.await();
      }
      remove=1;
      if (rename) {
        File file = new File(fileName);
        file.renameTo(new File(fileName + ".old"));
      }
      filesMap.remove(fileName);
      remove=0;
      notRemove.signal();
    } catch (InterruptedException e) {
        logger.error("Error during remove file from storage: "+fileName, e);
    } finally {
      lock.unlock();
    }
  }

  public int countFiles() {
    return filesMap.size();
  }

  private class XmlFileFilter implements FilenameFilter {

    public boolean accept(File dir, String name) {
      Matcher matcher = PATTERN.matcher(name);
      return matcher.matches();
    }
  }

}
 
