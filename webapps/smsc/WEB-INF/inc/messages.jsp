<%@page import="java.util.*,
                ru.novosoft.smsc.jsp.SMSCJspException,
                ru.novosoft.smsc.jsp.SMSCAppContext,
                java.io.PrintStream"%><%
if (errorMessages.size() > 0)
{
	%><div class=content><%
	for(Iterator it = errorMessages.iterator(); it.hasNext();)
	{
		SMSCJspException exc = (SMSCJspException) it.next();
		exc.printStackTrace(new PrintStream(System.err));
		String code = exc.getMessage();
		String nested = exc.getCause() == null ? "" : "<br>Nested: \"" + exc.getCause().getMessage() + "\" (" + exc.getCause().getClass().getName() + ")";
		String param = exc.getParam() == null ? "" : ": " + exc.getParam();
		String prefix = exc.getPrefix() == null ? "" : exc.getPrefix();
		String msg = getLocString(code);
		if (msg == null) msg = code;

		String text = prefix + msg + param + nested;
		switch (exc.getErrorClass())
		{
			case SMSCJspException.ERROR_CLASS_ERROR:
				{%><div class=error><div class=error_header><%=getLocString("common.errors.error")%></div><%=text%></div><%}
				break;
			case SMSCJspException.ERROR_CLASS_WARNING:
				{%><div class=warning><div class=warning_header><%=getLocString("common.errors.warning")%></div><%=text%></div><%}
				break;
			case SMSCJspException.ERROR_CLASS_MESSAGE:
				{%><div class=message><div class=message_header><%=getLocString("common.errors.attention")%></div><%=text%></div><%}
				break;
			default:
				if(code.startsWith("error."))
					{%><div class=error><div class=error_header><%=getLocString("common.errors.error")%></div><%=text%></div><%}
					else if(code.startsWith("warning."))
						{%><div class=warning><div class=warning_header><%=getLocString("common.errors.warning")%></div><%=text%></div><%}
					else
				{%><div class=message><div class=message_header><%=getLocString("common.errors.attention")%></div><%=text%></div><%}
				break;
		}
	}
	%></div><%
}
%>
