package ru.sibinco.smsx.network;

public class ThrottlingException extends Exception {

  public ThrottlingException(String message) {
    super(message);
  }
}