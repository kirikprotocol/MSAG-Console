package ru.novosoft.smsc.util;

import java.util.*;

/**
 * Created by Serge Lugovoy.
 * Date: Mar 13, 2003
 * Time: 2:57:39 PM
 */
public class LocaleMessages {
  private Map localeMessages = new HashMap();

  public LocaleMessages() {
    Locale locale = new Locale("ru");
    localeMessages.put(locale, ResourceBundle.getBundle("locales.messages", locale));
    locale = new Locale("en");
    localeMessages.put(locale, ResourceBundle.getBundle("locales.messages", locale));
  }

  public String getString( Locale locale, String key ) {
    ResourceBundle res = (ResourceBundle) localeMessages.get( locale );
    if( res != null ) {
      try {
        return res.getString(key);
      } catch (MissingResourceException ex) {
      }
    }
    return null;
  }

}
