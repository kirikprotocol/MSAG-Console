<%@ page import="java.util.*" %>
<%
if (errorMessages.size() > 0)
{
	for(Iterator it = errorMessages.iterator(); it.hasNext();)
	{
		SMSCJspException exc = (SMSCJspException) it.next();
		String code = exc.getMessage();
		String msg = (String) messages.get(code);
		%><%=(msg == null) ? code : msg%><BR><%
	}
}
%>