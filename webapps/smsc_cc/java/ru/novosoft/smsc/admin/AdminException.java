package ru.novosoft.smsc.admin;

public class AdminException extends Exception {

  public AdminException(String s) {
    super(s);
  }

  public AdminException(String s, Throwable cause) {
    super(s, cause);
  }
}
