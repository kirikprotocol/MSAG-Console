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

  public String toString() {
    StringBuilder builder = new StringBuilder();
    builder.append("\nstatus=").append(status).append("\n")
        .append("success=").append(success);
    if (lines != null) {
      for (String l : lines) {
        builder.append("\n").append(l);
      }
    }
    return builder.toString();
  }

}
