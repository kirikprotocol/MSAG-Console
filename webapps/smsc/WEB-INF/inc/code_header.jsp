<%
String TITLE = "SMS Center Administration Console";
String BROWSER_TITLE = null;
StringBuffer STATUS = new StringBuffer("");
String FORM_METHOD = "GET";
String FORM_URI = (String)request.getAttribute("requestURI");
if (FORM_URI == null)
	FORM_URI = request.getRequestURI();
String MENU0_SELECTION = "MENU0_HOME";
String FORM_ENCTYPE = "application/x-www-form-urlencoded";
final String CPATH = request.getContextPath() + "/smsc";
{
%><%@ page import="java.util.*, 
                   java.net.URLEncoder,
				   ru.novosoft.smsc.util.StringEncoderDecoder"
%><%@ include file="/WEB-INF/inc/messages_header.jsp"%>