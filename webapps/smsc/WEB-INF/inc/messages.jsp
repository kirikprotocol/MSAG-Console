<%@page import="java.util.*"%>
<%
if (errorMessages.size() > 0)
{
	%><tr><td class=secTrace><%
	for(Iterator it = errorMessages.iterator(); it.hasNext();)
	{
		SMSCJspException exc = (SMSCJspException) it.next();
		String code = exc.getMessage();
		String msg = (String) messages.get(code);
		if(code.startsWith("error."))
		{
			%><div class=err><%=(msg == null) ? code : msg%></div><%
		} else if(code.startsWith("warning."))
		{
			%><div class=warn><%=(msg == null) ? code : msg%></div><%
		} else {
			%><div class=msg><%=(msg == null) ? code : msg%></div><%
		}
		
	}
	%></td></tr><%
}
%>
