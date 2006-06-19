/*
 * Copyright (c) 2006 SibInco Inc. All Rights Reserved.
 */

package ru.sibinco.scag.util;

import javax.servlet.jsp.jstl.core.Config;
import javax.servlet.jsp.jstl.fmt.LocalizationContext;
import javax.servlet.http.HttpSession;
import java.util.*;
import java.text.MessageFormat;


public class LocaleMessages
{
  private static LocaleMessages instance = new LocaleMessages();

  public static LocaleMessages getInstance()
  {
    return instance;
  }


  private LocaleMessages()
  {
  }

  public Locale getLocale(HttpSession session) {
    Object locale=Config.get(session,Config.FMT_LOCALE);
    if (locale==null) locale = Config.get(session.getServletContext(),Config.FMT_LOCALE);
    if (locale instanceof String) return new Locale((String)locale);
    else return (Locale)locale;
  }

  public String getMessage(HttpSession session,String key) {
      try {
        return getBundle(session).getString(key);
      } catch(MissingResourceException ex) {
        return "???"+key+"???";
      }
  }

  public String getMessage(HttpSession session,String key, Object[] values) {
      try {
          return  MessageFormat.format(getMessage(session, key), values);
      } catch(MissingResourceException ex) {
        return "???"+key+"???";
      }
  }

  public ResourceBundle getBundle(HttpSession session) {
    Object lc = Config.get(session,Config.FMT_LOCALIZATION_CONTEXT);
    if (lc==null) lc = Config.get(session.getServletContext(),Config.FMT_LOCALIZATION_CONTEXT);
    if (lc instanceof String) return ResourceBundle.getBundle((String)lc,getLocale(session));
    else return ((LocalizationContext)lc).getResourceBundle();
  }

  public String getLocaleLanguage(HttpSession session) {
    return getLocale(session).getLanguage();
  }
}
