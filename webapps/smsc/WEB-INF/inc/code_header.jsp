<%@ page import="java.util.*,
                 java.net.URLEncoder,
                 ru.novosoft.smsc.util.StringEncoderDecoder,
                 ru.novosoft.smsc.admin.Constants"
%><%@
page pageEncoding="windows-1251"%><%@
page contentType="text/html; charset=windows-1251"%><%@
page buffer="64kb"%><%!
private String TITLE = "SMS Center Administration Console";
private String BROWSER_TITLE = null;
private String FORM_METHOD = "POST";
private String FORM_URI = "";
private String MENU0_SELECTION = "MENU0_NONE";
private String FORM_ENCTYPE = "application/x-www-form-urlencoded";
private String CPATH = "/smsc";
private String ServiceIDForShowStatus = null;
private boolean isServiceStatusColored = false;
private boolean isServiceStatusNeeded = false;
private java.util.List errorMessages = new ArrayList();
%><%
request.setAttribute(Constants.SMSC_ERROR_MESSAGES_ATTRIBUTE_NAME, errorMessages);
FORM_URI = (String)request.getAttribute("requestURI");
if (FORM_URI == null)
	FORM_URI = request.getRequestURI();
CPATH = request.getContextPath() + "/smsc";
{
%>