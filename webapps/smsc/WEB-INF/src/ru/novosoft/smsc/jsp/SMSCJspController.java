package ru.novosoft.smsc.jsp;

import ru.novosoft.util.jsp.JspController;

import javax.servlet.http.*;
import javax.servlet.*;
import java.io.*;


public class SMSCJspController extends JspController
{
  SMSCAppContext appContext;
  String         defaultEncoding = "ISO-8859-1";

  public void service(HttpServletRequest httpServletRequest, HttpServletResponse httpServletResponse) throws IOException, ServletException {
    httpServletRequest.setCharacterEncoding(defaultEncoding);
    super.service(httpServletRequest, httpServletResponse);
  }

  protected ru.novosoft.util.jsp.AppContext initAppContext()
	{
    if( System.getProperty("file.encoding") != null ) {
      defaultEncoding = System.getProperty("file.encoding");
    }
		appContext = new SMSCAppContextImpl(getInitParameter("config"));
        return appContext;
	}

    public void destroy() {
        appContext.destroy();
        super.destroy();
    }
}

