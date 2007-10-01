package ru.sibinco.smsx.engine.service.secret.datasource;

/**
 * User: artem
 * Date: 29.06.2007
 */

public class SecretUser{

  private String address;
  private String password;
  private boolean exists = false;

  public SecretUser(String number, String password) {
    this.address = number;
    this.password = password;
  }

  public String getAddress() {
    return address;
  }

  public String getPassword() {
    return password;
  }

  public void setPassword(String password) {
    this.password = password;
  }

  public boolean isExists() {
    return exists;
  }

  void setExists(boolean exists) {
    this.exists = exists;
  }

}
