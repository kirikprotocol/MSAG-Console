package mobi.eyeline.smsquiz.quizmanager.dirlistener;

import com.eyeline.utils.jmx.mbeans.AbstractDynamicMBean;
import mobi.eyeline.smsquiz.quizmanager.QuizException;
import org.apache.log4j.Logger;

import java.io.File;
import java.io.FilenameFilter;
import java.util.HashMap;
import java.util.Map;
import java.util.Observable;
import java.util.concurrent.locks.Condition;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

public class DirListener extends Observable implements Runnable {


  private static final Logger logger = Logger.getLogger(DirListener.class);
  private AbstractDynamicMBean monitor;
  private Map<String, QuizFile> filesMap;
  private String quizDir;
  private FilenameFilter fileFilter;
  private Lock lock;
  private Condition notRemove;
  private Condition notRun;
  private static final Pattern PATTERN = Pattern.compile("(.*\\.xml)");
  private int run = -1;
  private int remove = -1;

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
    monitor = new DirListenerMBean(this);
  }

  public void run() {
    logger.info("Running DirListener...");
    File dirQuiz = new File(quizDir);
    try {
      lock.lock();
      if (remove == 1) {
        notRemove.await();
      }
      run = 1;

      final File[] files = dirQuiz.listFiles(fileFilter);
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
      }
      for(String fileN: filesMap.keySet()) {
        if(!(new File(fileN)).exists()) {
          setChanged();
          notifyObservers(new Notification(fileN, Notification.FileStatus.DELETED));
          filesMap.remove(fileN);
        }
      }
    } catch (Exception e) {
      logger.error("Error construct quiz file or notification", e);
    } finally {
      run = 0;
      notRun.signal();
      lock.unlock();
    }

    logger.info("DirListener finished...");
  }

  public void remove(String fileName, boolean rename) throws QuizException {
    if (fileName == null) {
      throw new QuizException("Some arguments are null", QuizException.ErrorCode.ERROR_WRONG_REQUEST);
    }
    try {
      lock.lock();
      if(logger.isInfoEnabled()) {
        logger.info("Remove file:"+fileName);
      }
      if (run == 1) {
        notRun.await();
      }
      remove = 1;
      if (rename) {
        if(logger.isInfoEnabled()) {
          logger.info("Rename file: "+fileName);
        }
        File file = new File(fileName);
        file.renameTo(new File(fileName + ".old"));
      }
      filesMap.remove(fileName);
    } catch (InterruptedException e) {
      logger.error("Error during remove file from storage: " + fileName, e);
    } finally {
      remove = 0;
      notRemove.signal();
      lock.unlock();
    }
  }

  public int countFiles() {
    return filesMap.size();
  }

  public String getFilesList() {
    return filesMap.keySet().toString();
  }

  private class XmlFileFilter implements FilenameFilter {

    public boolean accept(File dir, String name) {
      Matcher matcher = PATTERN.matcher(name);
      return matcher.matches();
    }
  }

  public AbstractDynamicMBean getMonitor() {
    return monitor;
  }

}
 
