<% response.setHeader("Pragma", "no-cache");%>
<% response.setHeader("Cache-Control", "no-cache");%>
<%@ page errorPage="/common/error.jsp" %>
<%@ page import = "ru.novosoft.smsc.*"%>
<%@ page import = "ru.novosoft.smsc.admin.*"%>
<%@ page import = "ru.novosoft.smsc.admin.route.*"%>
<%@ page import = "ru.novosoft.smsc.admin.service.*"%>
<%@ page import = "ru.novosoft.smsc.jsp.SMSCAppContext" %>
<%@ page import = "ru.novosoft.smsc.util.*"%>
<%@ page import = "ru.novosoft.smsc.util.config.*"%>
<%@ page import = "javax.servlet.*"%>
<%@ page import = "javax.servlet.http.*"%>
<%@ page import = "java.util.*"%>
<%@ page import = "java.net.URLEncoder"%>
<%! final static String urlPrefix = "/smsc/smsc";%>
<%! final static String smscPrefix = "/smsc_service";%>
<%! final static String routesPrefix = "/smsc_service/routes";%>
<%! final static String aliasesPrefix = "/smsc_service/aliases";%>
<%! final static String hostsPrefix = "/common/hosts";%>
<%! final static String servicesPrefix = "/common/services";%>
<%!
  String getTriggerParamName(String trigger_name)
  {
    return "trigger_" + trigger_name;
  }
  
  boolean process_trigger(String trigger_name, HttpSession session, HttpServletRequest request)
  {
    Boolean trigger = (Boolean) session.getAttribute(getTriggerParamName(trigger_name));
    if (trigger == null)
      trigger = new Boolean(false);
    String trigger_string = request.getParameter(getTriggerParamName(trigger_name));
    if (trigger_string != null)
      trigger = Boolean.valueOf(trigger_string);
    
    session.setAttribute(getTriggerParamName(trigger_name), trigger);
    return trigger.booleanValue();
  }
  
  String switch_trigger(String trigger_url, String trigger_name, String trigger_to_true, String trigger_to_false, HttpSession session, HttpServletRequest request)
  {
    return "<a href=\"" + trigger_url + getTriggerParamName(trigger_name)+ "=" + (process_trigger(trigger_name, session, request)
              ? ("false\">" + StringEncoderDecoder.encode(trigger_to_false))
              : ("true\">" + StringEncoderDecoder.encode(trigger_to_true))) + "</a>";
  }
%>
<%
HttpSession ses = request.getSession();
SMSCAppContext appContext = (SMSCAppContext)request.getAttribute("appContext");
ServiceManager serviceManager = appContext.getServiceManager();
RouteManager routeManager = appContext.getRouteManager();
%>
<html>
<head>
  <title>SMSC Admin. <%=request.getParameter("page.title")!=null?request.getParameter("page.title"):""%></title>
  <link rel="stylesheet" type="text/css" href="<%=urlPrefix%>/common/smsc.css">