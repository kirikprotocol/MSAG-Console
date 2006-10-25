package ru.sibinco.calendarsme.network;

public class UserQueueOverflowException extends Exception {
  public UserQueueOverflowException(String message) {
    super(message);
  }
}