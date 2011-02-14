package ru.sibinco.smsx.engine.service;

/**
 * User: artem
 * Date: 16.07.2008
 */

public class Command {
  // Error codes
  public static final int ERR_SYS_ERROR = 1;
  protected static final int ERR_INT = 2;

  // Sources
  public static final int SOURCE_SMPP = 0;
  public static final int SOURCE_SOAP = 1;

  private int sourceId;
  private boolean systemMessage = false;

  public int getSourceId() {
    return sourceId;
  }

  public void setSourceId(int sourceId) {
    this.sourceId = sourceId;
  }

  public boolean isSystemMessage() {
    return systemMessage;
  }

  public void setSystemMessage(boolean systemMessage) {
    this.systemMessage = systemMessage;
  }
}
