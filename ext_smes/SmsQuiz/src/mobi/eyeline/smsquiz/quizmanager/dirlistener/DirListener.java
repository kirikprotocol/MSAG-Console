package mobi.eyeline.smsquiz.quizmanager.dirlistener;

import com.eyeline.utils.jmx.mbeans.AbstractDynamicMBean;
import mobi.eyeline.smsquiz.quizmanager.QuizException;
import org.apache.log4j.Logger;

import java.io.File;
import java.io.FilenameFilter;
import java.util.*;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

@SuppressWarnings({"ResultOfMethodCallIgnored"})
public class DirListener extends Observable implements Runnable {


  private static final Logger logger = Logger.getLogger(DirListener.class);
  private AbstractDynamicMBean monitor;
  private Map<String, QuizFile> filesMap;
  private String quizDir;
  private FilenameFilter fileFilter;
  private static final Pattern PATTERN = Pattern.compile("(.*\\.xml)");

  private Lock lock = new ReentrantLock();

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
    monitor = new DirListenerMBean(this);
  }

  public void refreshFile(String id){
    try {
      lock.lock();
      if (logger.isInfoEnabled()) {
        logger.info("Refresh single file with id: " + id);
      }
      Collection<Notification> ns = new LinkedList<Notification>();
      String fileName = quizDir + File.separator + id + ".xml";
      File f = new File(fileName);

      if (f.exists()) {
        _refreshFile(f, ns);
      } else {
        if (filesMap.remove(f.getAbsolutePath()) != null) {
          ns.add(new Notification(f.getAbsolutePath(), Notification.FileStatus.DELETED));
        }
      }
      if(!ns.isEmpty()) {
        setChanged();
        notifyObservers(ns);
      }
    } finally {
      lock.unlock();
    }
  }

  public void refreshFileList() {
    try {
      lock.lock();
      File dirQuiz = new File(quizDir);
      Collection<Notification> notifications = new LinkedList<Notification>();
      final File[] files = dirQuiz.listFiles(fileFilter);
      for (File f : files) {
        _refreshFile(f, notifications);
      }
      LinkedList<String> toRemove = new LinkedList<String>();
      for (String fileN : filesMap.keySet()) {
        if (!(new File(fileN)).exists()) {
          notifications.add(new Notification(fileN, Notification.FileStatus.DELETED));
          toRemove.add(fileN);
        }
      }
      for (String r : toRemove) {
        filesMap.remove(r);
      }
      if (!notifications.isEmpty()) {
        setChanged();
        notifyObservers(notifications);
      }
    } catch (Throwable e) {
      logger.error("Error construct quiz file or notification", e);
    } finally {
      lock.unlock();
    }
  }

  public void run() {
    logger.info("Running DirListener...");
    refreshFileList();
    logger.info("DirListener finished...");
  }


  private void _refreshFile(File f, Collection<Notification> notifications) {
    String fileName = f.getAbsolutePath();
    long lastModified = f.lastModified();
    QuizFile existFile;
    if ((existFile = filesMap.get(fileName)) != null) {
      if (existFile.getLastModified() < lastModified) {
        notifications.add(new Notification(fileName, Notification.FileStatus.MODIFIED));
        existFile.modifyDate(lastModified);
        if (logger.isInfoEnabled()) {
          logger.info("Quiz file modified: " + fileName);
        }
      }
    } else {
      filesMap.put(fileName, new QuizFile(fileName, lastModified));
      notifications.add(new Notification(fileName, Notification.FileStatus.CREATED));
      if (logger.isInfoEnabled()) {
        logger.info("Quiz file created: " + fileName);
      }
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
 
