package mobi.eyeline.smsquiz.quizmanager.dirlistener;

/**
 * author: alkhal
 */
class QuizFile {
  private String fileName;
  private long lastModified;

  public QuizFile(String fileName, long lastModified) {
    this.lastModified = lastModified;
    this.fileName = fileName;
  }

  public String getFileName() {
    return fileName;
  }

  public long getLastModified() {
    return lastModified;
  }

  public void modifyDate(long lastModified) {
    this.lastModified = lastModified;
  }
}
