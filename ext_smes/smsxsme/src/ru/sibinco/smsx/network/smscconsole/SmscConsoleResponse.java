package ru.sibinco.smsx.network.smscconsole;

/**
 * User: artem
 * Date: 17.07.2007
 */

/**
 * Response of SMSC console client
 */
public class SmscConsoleResponse {
  private boolean success;
  private String status;
  private String[] lines;

  SmscConsoleResponse(boolean success, String status, String[] responseLines) {
    this.success=success;
    this.status=status;
    this.lines=responseLines;
  }

  public String[] getResponseLines() {
    return lines;
  }

  public boolean isSuccess() {
    return success;
  }

  public String getStatus() {
    return status;
  }

}
