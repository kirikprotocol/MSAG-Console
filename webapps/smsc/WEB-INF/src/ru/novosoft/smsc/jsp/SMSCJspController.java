package ru.novosoft.smsc.jsp;

import javax.servlet.ServletException;
import javax.servlet.http.HttpServlet;
import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;

import ru.novosoft.smsc.AppContext;
import ru.novosoft.util.jsp.JspController;
import ru.novosoft.util.jsp.AppContextImpl;

public class SMSCJspController extends JspController
{
  AppContext appContext = null;


  protected ru.novosoft.util.jsp.AppContext initAppContext() {
    return new SMSCAppContextImpl(getInitParameter("config"));
  }
}

