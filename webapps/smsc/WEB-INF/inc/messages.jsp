<%@page import="java.util.*,
                ru.novosoft.smsc.jsp.SMSCJspException,
                ru.novosoft.smsc.jsp.SMSCAppContext,
                org.apache.catalina.Logger,
                org.apache.log4j.Category"%><%
if (errorMessages.size() > 0)
{
  SMSCAppContext appContext = (SMSCAppContext) request.getAttribute("appContext");
	%><div class=content><%
	for(Iterator it = errorMessages.iterator(); it.hasNext();)
	{
		SMSCJspException exc = (SMSCJspException) it.next();
    Category.getInstance("jsp errors").warn("JSP Exception occured", exc);
		String code = exc.getMessage();
    String nested = exc.getCause() == null ? "" : "<br>Nested: \"" + exc.getCause().getMessage() + "\" (" + exc.getCause().getClass().getName() + ")";
    String param = exc.getParam() == null ? "" : ": " + exc.getParam();
		String msg = appContext.getLocaleString(appContext.getUserPreferences(request.getUserPrincipal()).getLocale(), code);
    if (msg == null) msg = code;

    String text = msg + param + nested;
    switch (exc.getErrorClass())
    {
      case SMSCJspException.ERROR_CLASS_ERROR:
        {%><div class=error><div class=error_header>Error:</div><%=text%></div><%}
        break;
      case SMSCJspException.ERROR_CLASS_WARNING:
        {%><div class=warning><div class=warning_header>Warning:</div><%=text%></div><%}
        break;
      case SMSCJspException.ERROR_CLASS_MESSAGE:
        {%><div class=message><div class=message_header>Attention!</div><%=text%></div><%}
        break;
      default:
        if(code.startsWith("error."))
          {%><div class=error><div class=error_header>Error:</div><%=text%></div><%}
        else if(code.startsWith("warning."))
          {%><div class=warning><div class=warning_header>Warning:</div><%=text%></div><%}
        else
          {%><div class=message><div class=message_header>Attention!</div><%=text%></div><%}
        break;
    }
	}
	%></div><%
}
%>
