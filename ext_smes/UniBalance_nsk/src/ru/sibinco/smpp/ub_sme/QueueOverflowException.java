package ru.sibinco.smpp.network;

public class QueueOverflowException extends Exception {
  public QueueOverflowException(String message) {
    super(message);
  }
}