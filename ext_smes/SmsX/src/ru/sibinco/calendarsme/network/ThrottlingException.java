package ru.sibinco.calendarsme.network;

public class ThrottlingException extends Exception {

  public ThrottlingException(String message) {
    super(message);
  }
}