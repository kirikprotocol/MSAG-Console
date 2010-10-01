<%@ taglib prefix="c" uri="http://java.sun.com/jsp/jstl/core" %>
<%@ taglib prefix="fmt" uri="http://java.sun.com/jsp/jstl/fmt" %>
<%@ page import="ru.sibinco.scag.backend.SCAGAppContext,
                 ru.sibinco.scag.backend.users.UserPreferences,
                 ru.sibinco.scag.backend.users.ScagUser,
                 java.util.Map"%>
<%@ page import="ru.sibinco.scag.Constants"%>
<html>
<head>
    <link rel="STYLESHEET" type="text/css" href="content/styles/common.css">
</head>
<body style="margin: 0px 0px 0px 0px;" onLoad="if (opener!=null) opener.document.opForm.submit()">
<%
	SCAGAppContext appContext = (SCAGAppContext) request.getAttribute(Constants.APP_CONTEXT);
	long port = 6695;
  UserPreferences prefs = new UserPreferences();
	if (appContext != null)
	{
		try {
      if (request.getUserPrincipal()!=null)
        prefs = ((ScagUser)appContext.getUserManager().getUsers().get(request.getUserPrincipal().getName())).getPrefs();
			port = appContext.getConfig().getInt("perfmon.appletport");
		} catch (Throwable e) {
			e.printStackTrace();
		}
	}
  Map monPrefs = prefs.getMonPrefs();
%>
	<applet code="ru.sibinco.scag.perfmon.applet.PerfMon.class" width="100%" height="100%" archive="perfmon.jar">
        <param name="resource_servlet_uri" value="<%=request.getContextPath()+"/resource/?"%>">
		<param name="http.pixPerSecond" value="<%=monPrefs.get("perfmon.http.pixPerSecond")%>">
		<param name="http.scale" value="<%=monPrefs.get("perfmon.http.scale")%>">
		<param name="http.block" value="<%=monPrefs.get("perfmon.http.block")%>">
        <param name="smpp.pixPerSecond" value="<%=monPrefs.get("perfmon.smpp.pixPerSecond")%>">
		<param name="smpp.scale" value="<%=monPrefs.get("perfmon.smpp.scale")%>">
		<param name="smpp.block" value="<%=monPrefs.get("perfmon.smpp.block")%>">
		<param name="vLightGrid" value="<%=monPrefs.get("perfmon.vLightGrid")%>">
		<param name="vMinuteGrid" value="<%=monPrefs.get("perfmon.vMinuteGrid")%>">
		<param name="port" value="<%=port%>">
		<param name="host" value="<%=request.getServerName()%>">
        <param name="locale" value="${locale}">
	</applet>
</body>
</html>
