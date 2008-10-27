package mobi.eyeline.smsquiz.distribution.smscconsole;

/**
 * author: alkhal
 */
public class SmscConsoleResponse {

  private boolean success;
  private String status;
  private String[] lines;

  public SmscConsoleResponse(boolean success, String status, String[] lines) {
    this.success = success;
    this.status = status;
    this.lines = lines;
  }

  public boolean isSuccess() {
    return success;
  }

  public String getStatus() {
    return status;
  }

  public String[] getLines() {
    return lines;
  }
}
