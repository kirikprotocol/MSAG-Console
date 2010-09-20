package mobi.eyeline.informer.web.auth;

/**
 * author: alkhal
 */
public interface Authenticator {

  public SmscPrincipal authenticate(String login, String password);
  
}
