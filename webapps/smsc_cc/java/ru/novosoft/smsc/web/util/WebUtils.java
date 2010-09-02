package ru.novosoft.smsc.web.util;

import org.apache.log4j.Logger;
import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.users.User;
import ru.novosoft.smsc.web.WebContext;

import javax.faces.context.FacesContext;
import java.security.Principal;
import java.util.Locale;

/**
 * @author Artem Snopkov
 */
public class WebUtils {

  private static final Logger logger = Logger.getLogger(WebUtils.class);

  public static Locale getLocale() {
    return getLocale(FacesContext.getCurrentInstance());
  }

  public static Locale getLocale(FacesContext context) {
    Locale l;
    Principal p = context.getExternalContext().getUserPrincipal();
    if(p != null) {
      User u = null;
      try {
        u = WebContext.getInstance().getAppliableConfiguration().getUsersSettings().getUser(p.getName());
      } catch (AdminException e) {
        logger.error(e,e);
      }
      if(u != null) {
        return u.getPrefs().getLocale();
      }
    }
    l = FacesContext.getCurrentInstance().getExternalContext().getRequestLocale();
    if(l != null) {
      return l;
    }
    return Locale.ENGLISH;
  }
}
