package mobi.eyeline.smsquiz.quizmanager.filehandler;

import com.eyeline.utils.jmx.mbeans.AbstractDynamicMBean;
import mobi.eyeline.smsquiz.quizmanager.QuizException;
import org.apache.log4j.Logger;

import java.io.File;
import java.io.FilenameFilter;
import java.text.SimpleDateFormat;
import java.util.*;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

public class QuizFileHandler extends Observable implements Runnable {


  private static final Logger logger = Logger.getLogger(QuizFileHandler.class);
  private AbstractDynamicMBean monitor;
  private Map<String, QuizFile> filesMap;
  private String quizDir;
  private String dirWork;
  private String dirArchive;
  private String dirResult;
  private FilenameFilter fileFilter;
  private static final Pattern PATTERN = Pattern.compile("(.*\\.xml)");

  private SimpleDateFormat df = new SimpleDateFormat("yyyyMMddHHmmss");

  public QuizFileHandler(final String quizDir, final String dirWork,
                     final String dirArchive, final String dirResult) throws QuizException {
    if (quizDir == null) {
      throw new QuizException("Some arguments are null", QuizException.ErrorCode.ERROR_WRONG_REQUEST);
    }
    this.quizDir = quizDir;
    this.dirWork = dirWork;
    this.dirArchive = dirArchive;
    this.dirResult = dirResult;
    File file = new File(quizDir);
    if (!file.exists()) {
      logger.info("Create quizDir");
      file.mkdirs();
    }
    filesMap = new HashMap<String, QuizFile>();
    fileFilter = new XmlFileFilter();
    monitor = new QuizFIleHandlerMBean(this);
  }

  public void run() {
    logger.info("Running QuizFileHandler...");
    File dirQuiz = new File(quizDir);
    try {
      Collection<Notification> notifications = new LinkedList<Notification>();
      final File[] files = dirQuiz.listFiles(fileFilter);
      for (File f : files) {
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
      if(!notifications.isEmpty()) {
          setChanged();
          notifyObservers(notifications);
      }
    } catch (Throwable e) {
      logger.error("Error construct quiz file or notification", e);
    }
    logger.info("QuizFileHandler finished...");
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

  public void delete(String quizId, String quizName, String abFile) throws QuizException {
    if ((quizId == null) || (abFile == null)) {
      logger.error("Some arguments are null");
      throw new QuizException("Some arguments are null");
    }
    File file = new File(dirArchive);
    if (!file.exists()) {
      file.mkdirs();
    }

    file = new File(abFile);
    if (file.exists()) {
      renameFile(new File(abFile));
    } else {
      file = new File(quizDir + File.separator + file.getName());
      renameFile(file);
    }

    String parentSlashQuizId = dirWork + File.separator + quizId;

    renameFile(new File(parentSlashQuizId + ".status"));
    deleteFile(new File(parentSlashQuizId + ".xml.bin"));
    deleteFile(new File(parentSlashQuizId + ".xml.bin.j"));
    renameFile(new File(parentSlashQuizId + ".error"));
    try{
      int code = (quizName+"(SmsQuiz)").hashCode();
      renameFile(new File(dirWork + File.separator + code + ".csv"));
      renameFile(new File(dirWork + File.separator + code + ".csv.processed"));
    } catch (Exception e) {
      logger.error(e,e);
      throw new QuizException(e);
    }
    file = new File(dirResult);
    File files[] = file.listFiles();
    file = null;
    if (files != null) {
      for (File file1 : files) {
        if ((file1.isFile()) && (file1.getName().startsWith(quizId + "."))) {         //todo
          file = file1;
          break;
        }
      }
    }
    if (file != null) {
      renameFile(file);
    }
  }

  private void renameFile(File file) {
    try {
      String name = file.getName();
      System.out.println("Try to rename: "+file.getAbsolutePath());
      file.renameTo(new File(dirArchive + File.separator + name + "." + df.format(new Date())));
    } catch (Exception e) {
      logger.error(e, e);
    }
  }

  private void deleteFile(File file) {
    try {
      file.delete();
    } catch (Exception e) {
      logger.error(e);
    }
  }

}
 
