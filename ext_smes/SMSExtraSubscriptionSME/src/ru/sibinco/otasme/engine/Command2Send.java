package ru.sibinco.otasme.engine;

/**
 * User: artem
 * Date: 02.11.2006
 */

final class Command2Send {
  private final String wtsUserId;
  private final String wtsServiceName;
  private final String wtsRequestReference;

  public Command2Send(String wtsUserId, String wtsServiceName, String wtsRequestReference) {
    this.wtsUserId = wtsUserId;
    this.wtsServiceName = wtsServiceName;
    this.wtsRequestReference = wtsRequestReference;
  }

  public String getWtsUserId() {
    return wtsUserId;
  }

  public String getWtsServiceName() {
    return wtsServiceName;
  }

  public String getWtsRequestReference() {
    return wtsRequestReference;
  }
}
