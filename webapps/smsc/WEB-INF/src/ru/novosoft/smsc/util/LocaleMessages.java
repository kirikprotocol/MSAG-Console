package ru.novosoft.smsc.util;

import java.util.*;

/**
 * Created by Serge Lugovoy.
 * Date: Mar 13, 2003
 * Time: 2:57:39 PM
 */

public class LocaleMessages
{
  private Map localeMessages = new HashMap();

  public LocaleMessages()
  {
    Locale locale = new Locale("ru");
    localeMessages.put(locale, ResourceBundle.getBundle("locales.messages", locale));
    locale = new Locale("en");
    localeMessages.put(locale, ResourceBundle.getBundle("locales.messages", locale));
  }

  public Set getStrings(Locale locale, String prefix)
  {
    Set result = new HashSet();
    final int prefixLength = prefix.length();
    final ResourceBundle res = getResourceBundle(locale);
    if (res != null) {
      final Enumeration keys = res.getKeys();
      while (keys.hasMoreElements()) {
        String key = (String) keys.nextElement();
        if (key.startsWith(prefix)) {
          result.add(key.substring(prefixLength));
        }
      }
    }
    return result;
  }

  public String getString(Locale locale, String key)
  {
    ResourceBundle res = getResourceBundle(locale);
    if (res != null) {
      try {
        return res.getString(key);
      } catch (MissingResourceException ex) {
      }
    }
    return null;
  }

  private ResourceBundle getResourceBundle(Locale locale)
  {
    ResourceBundle res = (ResourceBundle) localeMessages.get(locale);
    if (res == null)
      res = (ResourceBundle) localeMessages.get(new Locale(locale.getLanguage(), locale.getCountry()));
    if (res == null)
      res = (ResourceBundle) localeMessages.get(new Locale(locale.getLanguage()));
    return res;
  }
}