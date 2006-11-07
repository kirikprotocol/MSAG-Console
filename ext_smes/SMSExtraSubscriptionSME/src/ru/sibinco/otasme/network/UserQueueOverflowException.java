package ru.sibinco.otasme.network;

public class UserQueueOverflowException extends Exception {
  public UserQueueOverflowException(String message) {
    super(message);
  }
}