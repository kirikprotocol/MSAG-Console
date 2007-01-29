package ru.sibinco.smsx.network;

public class UserQueueOverflowException extends Exception {
  public UserQueueOverflowException(String message) {
    super(message);
  }
}