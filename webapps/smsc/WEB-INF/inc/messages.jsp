<%@page import="java.util.*"%>
<%
if (errorMessages.size() > 0)
{
	%><div class=content><%
	for(Iterator it = errorMessages.iterator(); it.hasNext();)
	{
		SMSCJspException exc = (SMSCJspException) it.next();
		String code = exc.getMessage();
		String msg = null;
		String param = exc.getParam();
		msg = appContext.getLocaleString(appContext.getUserPreferences(loginedUserPrincipal).getLocale(), code);
		if(code.startsWith("error."))
		{
			%><div class=error><%=(msg == null) ? code : msg%><%=(param == null) ? "" : ": " + param%></div><%
		} else if(code.startsWith("warning."))
		{
			%><div class=warning><%=(msg == null) ? code : msg%><%=(param == null) ? "" : ": " + param%></div><%
		} else {
			%><div class=message><%=(msg == null) ? code : msg%><%=(param == null) ? "" : ": " + param%></div><%
		}

	}
	%></div><%
}
%>
