package ru.sibinco.otasme.network;

public class QueueOverflowException extends Exception {
  public QueueOverflowException(String message) {
    super(message);
  }
}