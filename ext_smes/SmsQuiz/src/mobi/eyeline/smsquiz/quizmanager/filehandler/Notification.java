package mobi.eyeline.smsquiz.quizmanager.filehandler;

public class Notification {

  public static enum FileStatus {
    CREATED, MODIFIED, DELETED
  }

  private String fileName;

  private FileStatus status;

  public Notification(String fileName, FileStatus status) {
    this.fileName = fileName;
    this.status = status;
  }

  public String getFileName() {
    return fileName;
  }

  public FileStatus getStatus() {
    return status;
  }

}
 
