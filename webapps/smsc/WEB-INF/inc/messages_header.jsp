<jsp:useBean id="errorMessages" class="java.util.ArrayList"/><%@ 
page import="ru.novosoft.smsc.jsp.*"
%><%!
	public static final java.util.Map messages = new java.util.HashMap();
	static
	{
		messages.put(SMSCErrors.error.failed, "System failed");
		messages.put(SMSCErrors.error.unknown, "Unknown system error");
		messages.put(SMSCErrors.error.system, "System error");
	}
%>