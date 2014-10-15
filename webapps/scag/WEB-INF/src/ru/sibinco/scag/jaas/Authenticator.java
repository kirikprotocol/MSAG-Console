package ru.sibinco.scag.jaas;

public interface Authenticator {

  public MSAGPrincipal authenticate(String login, String password);

}
