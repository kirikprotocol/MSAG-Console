package mobi.eyeline.informer.web.controllers;

import mobi.eyeline.informer.web.WebContext;
import mobi.eyeline.informer.web.auth.Authenticator;

import javax.faces.context.FacesContext;
import java.util.Map;

/**
 * @author Aleksandr Khalitov
 */
public class LoginPageController {

  private Authenticator.Error message;

  private boolean error;

  public LoginPageController() {
    Map<String, String> request = FacesContext.getCurrentInstance().getExternalContext().getRequestParameterMap();
    String error = request.get("loginError");
    String login = request.get("j_username");
    if(error != null) {
      this.error = true;
      if(login != null) {
        message = WebContext.getInstance().getAuthenticator().getError(login);
      }
    }
  }

  public Authenticator.Error getMessage() {
    return message;
  }

  public boolean isError() {
    return error;
  }
}
