package ru.sibinco.otasme.network;

public class ThrottlingException extends Exception {

  public ThrottlingException(String message) {
    super(message);
  }
}