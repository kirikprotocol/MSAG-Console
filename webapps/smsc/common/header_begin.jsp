<% response.setHeader("Pragma", "no-cache");%>
<% response.setHeader("Cache-Control", "no-cache");%>
<%@ page import = "ru.novosoft.smsc.*"%>
<%@ page import = "ru.novosoft.smsc.admin.*"%>
<%@ page import = "ru.novosoft.smsc.admin.alias.*"%>
<%@ page import = "ru.novosoft.smsc.admin.route.*"%>
<%@ page import = "ru.novosoft.smsc.admin.service.*"%>
<%@ page import = "ru.novosoft.smsc.admin.service.*"%>
<%@ page import = "ru.novosoft.smsc.admin.smsc_service.Smsc"%>
<%@ page import = "ru.novosoft.smsc.jsp.SMSCAppContext"%>
<%@ page import = "ru.novosoft.smsc.util.*"%>
<%@ page import = "ru.novosoft.smsc.util.config.*"%>
<%@ page import = "javax.servlet.*"%>
<%@ page import = "javax.servlet.http.*"%>
<%@ page import = "java.util.*"%>
<%@ page import = "java.net.URLEncoder"%>
<%! final static String urlPrefix = "/smsc/smsc";%>
<%! final static String smscPrefix = "/smsc_service";%>
<%! final static String routesPrefix = "/smsc_service/routes";%>
<%! final static String subjectsPrefix = "/smsc_service/subjects";%>
<%! final static String aliasesPrefix = "/smsc_service/aliases";%>
<%! final static String profilesPrefix = "/smsc_service/profiles";%>
<%! final static String hostsPrefix = "/common/hosts";%>
<%! final static String servicesPrefix = "/common/services";%>
<%
HttpSession ses = request.getSession();
SMSCAppContext appContext = (SMSCAppContext)request.getAttribute("appContext");
ServiceManager serviceManager = appContext.getServiceManager();
Smsc smsc = appContext.getSmsc();
%>
<%@ include file="/common/utils.jsp"%>
<html>
<head>
  <title>SMSC Admin. <%=request.getParameter("page.title")!=null?request.getParameter("page.title"):""%></title>
  <link rel="stylesheet" type="text/css" href="<%=urlPrefix%>/common/smsc.css">