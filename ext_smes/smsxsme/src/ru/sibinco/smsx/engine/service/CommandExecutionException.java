package ru.sibinco.smsx.engine.service;

/**
 * User: artem
 * Date: 16.07.2008
 */

public class CommandExecutionException extends Exception {

  private final int errCode;

  public CommandExecutionException(String message, int errCode) {
    super(message);
    this.errCode = errCode;
  }

  public int getErrCode() {
    return errCode;
  }
}
