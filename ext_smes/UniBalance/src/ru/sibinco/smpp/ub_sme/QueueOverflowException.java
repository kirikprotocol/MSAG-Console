package ru.sibinco.smpp.ub_sme;

public class QueueOverflowException extends Exception {
  public QueueOverflowException(String message) {
    super(message);
  }
}