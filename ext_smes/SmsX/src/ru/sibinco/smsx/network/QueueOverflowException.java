package ru.sibinco.smsx.network;

public class QueueOverflowException extends Exception {
  public QueueOverflowException(String message) {
    super(message);
  }
}