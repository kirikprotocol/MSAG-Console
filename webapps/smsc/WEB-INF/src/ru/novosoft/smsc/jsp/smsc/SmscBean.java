package ru.novosoft.smsc.jsp.smsc;

/*
 * Created by igork
 * Date: 04.11.2002
 * Time: 18:57:09
 */

import ru.novosoft.smsc.admin.smsc_service.RouteSubjectManager;
import ru.novosoft.smsc.admin.smsc_service.SmeManager;
import ru.novosoft.smsc.admin.smsc_service.Smsc;
import ru.novosoft.smsc.jsp.PageBean;

import java.util.List;

public class SmscBean extends PageBean
{
  protected Smsc smsc = null;
  protected RouteSubjectManager routeSubjectManager = null;
  protected SmeManager smeManager = null;

  protected int init(List errors)
  {
    int result = super.init(errors);
    if (result != RESULT_OK)
      return result;

    smsc = appContext.getSmsc();
    routeSubjectManager = appContext.getRouteSubjectManager();
    smeManager = appContext.getSmeManager();
    return RESULT_OK;
  }
}
