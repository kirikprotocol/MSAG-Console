<%@page import="java.util.*"%>
<%
if (errorMessages.size() > 0)
{
	%><tr><td class=secTrace><%
	for(Iterator it = errorMessages.iterator(); it.hasNext();)
	{
		SMSCJspException exc = (SMSCJspException) it.next();
		String code = exc.getMessage();
		String msg = null;
		String param = exc.getParam();
		if (messages != null)
		{
			try { msg = messages.getString(code);}
			catch (Throwable t) {}
		}
		if(code.startsWith("error."))
		{
			%><div class=err><%=(msg == null) ? code : msg%><%=(param == null) ? "" : ": " + param%></div><%
		} else if(code.startsWith("warning."))
		{
			%><div class=warn><%=(msg == null) ? code : msg%><%=(param == null) ? "" : ": " + param%></div><%
		} else {
			%><div class=msg><%=(msg == null) ? code : msg%><%=(param == null) ? "" : ": " + param%></div><%
		}
		
	}
	%></td></tr><%
}
%>
