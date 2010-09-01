package ru.novosoft.smsc.web.util;

import javax.faces.context.FacesContext;
import java.util.Locale;

/**
 * @author Artem Snopkov
 */
public class WebUtils {
  private static Locale calculateLocale() {
    if (FacesContext.getCurrentInstance().getViewRoot() != null)
      return FacesContext.getCurrentInstance().getViewRoot().getLocale();
    return FacesContext.getCurrentInstance().getExternalContext().getRequestLocale();
  }

  public static Locale getLocale() {
    Locale l = calculateLocale();
    return l == null ? Locale.ENGLISH : l;
  }
}
