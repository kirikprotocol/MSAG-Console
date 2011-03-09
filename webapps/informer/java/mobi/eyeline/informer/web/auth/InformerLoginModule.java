package mobi.eyeline.informer.web.auth;

import mobi.eyeline.informer.web.WebContext;

import javax.security.auth.Subject;
import javax.security.auth.callback.Callback;
import javax.security.auth.callback.CallbackHandler;
import javax.security.auth.callback.NameCallback;
import javax.security.auth.callback.PasswordCallback;
import javax.security.auth.login.LoginException;
import javax.security.auth.spi.LoginModule;
import java.util.Map;

/**
 * JAAS Login module
 *
 * @author Aleksandr Khalitov
 */
public class InformerLoginModule implements LoginModule {
  private Subject subject;
  private CallbackHandler callbackHandler;

  private String password;
  private String name;


  public boolean abort() throws LoginException {
    return true;
  }

  public boolean commit() throws LoginException {
    InformerPrincipal p = WebContext.getInstance().getAuthenticator().authenticate(name, password);
    if (p == null) {
      return false;
    }
    subject.getPrincipals().add(new User(name));
    for (String r : p.getRoles()) {
      subject.getPrincipals().add(new Role(r));
    }
    return true;
  }


  public void initialize(Subject subject, CallbackHandler callbackHandler,
                         Map<String, ?> sharedState, Map<String, ?> options) {
    this.subject = subject;
    this.callbackHandler = callbackHandler;
  }


  public boolean login() throws LoginException {
    NameCallback name = new NameCallback("User name");
    PasswordCallback password = new PasswordCallback("Password", true);
    try {
      this.callbackHandler.handle(new Callback[]{name, password});
      this.name = name.getName();
      this.password = new String(password.getPassword());
    } catch (Exception e) {
      throw new LoginException(e.getMessage());
    }

    return true;
  }

  private static boolean isBlank(String s) {
    return s == null || s.length() == 0;
  }


  public boolean logout() throws LoginException {
    return true;
  }

}
