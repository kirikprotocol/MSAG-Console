<%! final static String urlPrefix = "/smsc/smsc";%>
<%! final static String smscPrefix = "/smsc_service";%>
<%! final static String hostsPrefix = "/common/hosts";%>
<%! final static String servicesPrefix = "/common/services";%>
<%@ page errorPage="/common/error.jsp" %>
<%@ page import = "ru.novosoft.smsc.*"%>
<%@ page import = "ru.novosoft.smsc.admin.*"%>
<%@ page import = "ru.novosoft.smsc.admin.service.*"%>
<%@ page import = "ru.novosoft.smsc.util.config.*"%>
<%@ page import = "javax.servlet.*"%>
<%@ page import = "javax.servlet.http.*"%>
<%@ page import = "java.util.*"%>
<%@ page import = "java.net.URLEncoder"%>
<%
HttpSession ses = request.getSession();
AppContext appContext = (AppContext)request.getAttribute("appContext");
ServiceManager serviceManager = appContext.getServiceManager();
%>
<html>
<head>
	<title>SMSC Admin. <%=request.getParameter("page.title")!=null?request.getParameter("page.title"):""%></title>
	<link rel="stylesheet" type="text/css" href="<%=urlPrefix%>/common/smsc.css">
