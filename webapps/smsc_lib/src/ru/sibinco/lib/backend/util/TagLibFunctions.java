package ru.sibinco.lib.backend.util;

import ru.sibinco.lib.bean.TabledBean;

import ru.sibinco.lib.LocaleMessages;
import java.util.Locale;


/**
 * Created by igork
 * Date: 11.03.2004
 * Time: 0:28:52
 */
public class TagLibFunctions
{
  public static boolean contains(String[] array, String str)
  {
    if (array == null || array.length == 0)
      return false;
    for (int i = 0; i < array.length; i++) {
      String s = array[i];
      if ((str == null || str.length() == 0) && (s == null || s.length() == 0))
        return true;
      if (str != null && s != null && str.equals(s))
        return true;
    }
    return false;
  }

  public static final boolean checked(TabledBean bean, String id)
  {
    return bean.isChecked(id);
  }

  public static final boolean isBoolean(Object obj)
  {
    return obj instanceof Boolean;
  }

  public static final String concat(String s1, String s2)
  {
    return s1 + s2;
  }

  public static final String getErrorMessage(Locale locale, int errcode, boolean extended)
  {
    try {
      String errMessage = LocaleMessages.getInstance().getMessage(locale, "smsc.errcode."+errcode);
      if (errMessage == null) errMessage = LocaleMessages.getInstance().getMessage(locale, "smsc.errcode.unknown");
      if (errMessage == null) return "! undefined error code "+ ((extended) ? ("("+errcode+") !"):"!");
      if (extended) errMessage += " ("+errcode+")";
      return errMessage;
    } catch (Exception e) {
      e.printStackTrace();
      return null;
    }
  }
}
