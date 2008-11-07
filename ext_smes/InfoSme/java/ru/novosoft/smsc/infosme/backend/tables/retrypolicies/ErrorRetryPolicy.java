package ru.novosoft.smsc.infosme.backend.tables.retrypolicies;

/**
 * User: artem
 * Date: 01.11.2008
 */
public class ErrorRetryPolicy {
  
  private String errorCode;
  private int policy;

  public ErrorRetryPolicy(String errorCode, int policy) {
    this.errorCode = errorCode;
    this.policy = policy;
  }

  public String getErrorCode() {
    return errorCode;
  }

  public void setErrorCode(String errorCode) {
    this.errorCode = errorCode;
  }

  public int getPolicy() {
    return policy;
  }

  public void setPolicy(int policy) {
    this.policy = policy;
  }
}
