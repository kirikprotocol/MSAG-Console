package ru.sibinco.scag.util;
import ru.sibinco.lib.bean.TabledBean;
import ru.sibinco.scag.backend.rules.Rule;
import ru.sibinco.scag.backend.SCAGAppContext;

import javax.servlet.http.HttpSession;
import javax.servlet.http.HttpServletRequest;

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
     return ((obj instanceof Boolean) || (obj.toString().equals("true")) || (obj.toString().equals("false")));
   }

  public static final String concat(String s1, String s2)
  {
    return s1 + s2;
  }

  public static final String getErrorMessage(HttpSession session, int errcode, boolean extended, String statMode)
  {
    try {
      String errMessage = LocaleMessages.getInstance().getMessage(session,"smsc.errcode."+statMode+"."+errcode);
      if (errMessage.startsWith("???") && errMessage.endsWith("???")) {
         errMessage = LocaleMessages.getInstance().getMessage(session,"smsc.errcode.unknown");
         if (statMode.equals("http") && (errcode>99 && errcode<600)) {
           byte firstFigure = (byte)((errcode-errcode%100)/100);
           errMessage = LocaleMessages.getInstance().getMessage(session,"smsc.errcode."+statMode+"."+firstFigure+"xx");
         }
      }
      if (extended) errMessage += " ("+errcode+")";
      return errMessage;
    } catch (Exception e) {
      e.printStackTrace();
      return null;
    }
  }

  public static final String getComplexRuleId(String ruleId, String transport) {
    return Rule.composeComplexId(ruleId,transport);
  }

  public static final String getHttpRouteNameById(HttpServletRequest req,String id) {
    return ((SCAGAppContext)req.getAttribute("appContext")).getHttpRoutingManager().getHttpRouteNameById(id);
  }
}
