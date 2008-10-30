package mobi.eyeline.smsquiz.quizmanager.quiz;

import mobi.eyeline.smsquiz.quizmanager.QuizException;
import org.apache.log4j.Logger;

import java.io.*;
import java.util.Properties;

/**
 * author: alkhal
 */

class Status {


  private String ident;
  private String statusFileName;
  private static final Logger logger = Logger.getLogger(Status.class);

  Status(String statusFileName) throws QuizException {
    this.statusFileName = statusFileName;
    File stFile = new File(statusFileName);
    File parentFile = stFile.getParentFile();
    if ((parentFile != null) && (!parentFile.exists())) {
      parentFile.mkdirs();
    }
    if (stFile.exists()) {
      Properties prop = new Properties();
      try {
        prop.load(new FileInputStream(stFile));
        String ident;
        if ((ident = prop.getProperty("distribution.id")) != null) {  // if property exist => distribution already created
          this.ident = ident;
        }
      } catch (IOException e) {
        logger.error("Unable to open status file", e);
        throw new QuizException("Unable to open status file", e);
      }
    }
  }

  public String getId() {
    return ident;
  }

  void setId(String ident) throws QuizException {
    this.ident = ident;
    writeStatusFile(new File(statusFileName), ident);
  }

  private void writeStatusFile(File file, String id) throws QuizException {
    if (file == null) {
      return;
    }
    File parentFile;
    if ((parentFile = file.getParentFile()) != null) {
      if (!parentFile.exists()) {
        parentFile.mkdirs();
      }
    }
    PrintWriter printWriter = null;
    try {
      printWriter = new PrintWriter(new BufferedWriter(new FileWriter(file)));
      printWriter.print("distribution.id=");
      printWriter.println(id);
      printWriter.flush();

    } catch (IOException e) {
      logger.error("Unable to open status file", e);
      throw new QuizException("Unable to open status file", e);
    } finally {
      if (printWriter != null) {
        printWriter.close();
      }
    }
  }

  public String getStatusFileName() {
    return statusFileName;
  }
}
