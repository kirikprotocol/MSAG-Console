package ru.novosoft.smsc.jsp;

import ru.novosoft.smsc.AppContext;
import ru.novosoft.util.jsp.JspController;


public class SMSCJspController extends JspController
{
  AppContext appContext = null;


  protected ru.novosoft.util.jsp.AppContext initAppContext()
  {
    return new SMSCAppContextImpl(getInitParameter("config"));
  }
}

