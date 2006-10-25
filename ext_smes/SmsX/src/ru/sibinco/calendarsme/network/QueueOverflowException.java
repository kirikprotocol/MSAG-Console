package ru.sibinco.calendarsme.network;

public class QueueOverflowException extends Exception {
  public QueueOverflowException(String message) {
    super(message);
  }
}