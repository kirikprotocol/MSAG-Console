/**
 * Created by igork
 * Date: 19.03.2004
 * Time: 16:03:33
 */
package ru.sibinco.lib;

import java.util.*;


public class LocaleMessages
{
  private static LocaleMessages instance = new LocaleMessages();

  public static LocaleMessages getInstance()
  {
    return instance;
  }


  private Map bundles = new HashMap();

  public LocaleMessages()
  {
    Locale locale = new Locale("ru");
    bundles.put(locale, ResourceBundle.getBundle("locales.messages", locale));
    locale = new Locale("en");
    bundles.put(locale, ResourceBundle.getBundle("locales.messages", locale));
  }

  public String getMessage(Locale locale, String key)
  {
    ResourceBundle res = (ResourceBundle) bundles.get(locale);
    if (res != null) {
      try {
        return res.getString(key);
      } catch (MissingResourceException ex) {
      }
    }
    res = (ResourceBundle) bundles.get(new Locale(locale.getLanguage(), locale.getCountry()));
    if (res != null) {
      try {
        return res.getString(key);
      } catch (MissingResourceException ex) {
      }
    }
    res = (ResourceBundle) bundles.get(new Locale(locale.getLanguage()));
    if (res != null) {
      try {
        return res.getString(key);
      } catch (MissingResourceException ex) {
      }
    }
    return null;
  }
}
