<%!
private String TITLE = "SMS Center Administration Console";
private String BROWSER_TITLE = null;
private StringBuffer STATUS = new StringBuffer("");
private String FORM_METHOD = "POST";
private String FORM_URI = "";
private String MENU0_SELECTION = "MENU0_NONE";
private String FORM_ENCTYPE = "application/x-www-form-urlencoded";
private String CPATH = "/smsc";
private java.security.Principal loginedUserPrincipal = null;
private ru.novosoft.smsc.jsp.SMSCAppContext appContext = null;
private String ServiceIDForShowStatus = null;
private boolean isServiceStatusColored = false;
private boolean isServiceStatusNeeded = false;
%><%@
page pageEncoding="windows-1251"%><%@
page errorPage="/error.jsp"%><%@
page contentType="text/html; charset=windows-1251"%><%@
page session="true"%><%
FORM_URI = (String)request.getAttribute("requestURI");
if (FORM_URI == null)
	FORM_URI = request.getRequestURI();
CPATH = request.getContextPath() + "/smsc";
loginedUserPrincipal = request.getUserPrincipal();
appContext = (ru.novosoft.smsc.jsp.SMSCAppContext)request.getAttribute("appContext");
{
%><%@ 
page import="java.util.*, 
             java.net.URLEncoder,
			 ru.novosoft.smsc.util.StringEncoderDecoder"
%><%@ 
include file="/WEB-INF/inc/messages_header.jsp"%><%@ 
include file="/WEB-INF/inc/page_menu.jsp"%><%@ 
include file="/WEB-INF/inc/buttons.jsp"%>