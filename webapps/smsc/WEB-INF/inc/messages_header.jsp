<jsp:useBean id="errorMessages" class="java.util.ArrayList"/><%@ 
page import="ru.novosoft.smsc.jsp.*"
%><%
	java.util.ResourceBundle messages = null;
	if (appContext != null)
		messages = appContext.getLocaleMessages(request.getLocale());
/*	public static final java.util.ResourceBundle messages = appContext.getLocaleMessages(request.getLocale());
	static
	{
		messages.put(SMSCErrors.error.failed, "System failed");
		messages.put(SMSCErrors.error.unknown, "Unknown system error");
		messages.put(SMSCErrors.error.system, "System error");
	}*/
%>