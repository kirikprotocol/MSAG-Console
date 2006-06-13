package ru.sibinco.scag.util;
import ru.sibinco.lib.bean.TabledBean;

import javax.servlet.jsp.PageContext;
import javax.servlet.http.HttpSession;

/**
 * Created by IntelliJ IDEA.
 * User: dym
 * Date: 07.06.2006
 * Time: 14:34:55
 * To change this template use File | Settings | File Templates.
 */
public class TagLibFunctions {
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

  public static final boolean isString(Object obj)
  {
    return obj instanceof String;
  }

  public static final boolean isBoolean(Object obj)
   {
     return obj instanceof Boolean;
   }

  public static final String concat(String s1, String s2)
  {
    return s1 + s2;
  }

  public static final String getErrorMessage(HttpSession session, int errcode, boolean extended)
  {
    try {
      String errMessage = LocaleMessages.getInstance().getMessage(session,"smsc.errcode."+errcode);
      if (errMessage == null) errMessage = LocaleMessages.getInstance().getMessage(session,"smsc.errcode.unknown");
      if (errMessage == null) return "! undefined error code "+ ((extended) ? ("("+errcode+") !"):"!");
      if (extended) errMessage += " ("+errcode+")";
      return errMessage;
    } catch (Exception e) {
      e.printStackTrace();
      return null;
    }
  }


}
