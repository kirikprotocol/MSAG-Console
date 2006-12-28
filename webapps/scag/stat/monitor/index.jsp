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
		<param name="pixPerSecond" value="<%=monPrefs.get("perfmon.pixPerSecond")%>">
		<param name="scale" value="<%=monPrefs.get("perfmon.scale")%>">
		<param name="block" value="<%=monPrefs.get("perfmon.block")%>">
		<param name="vLightGrid" value="<%=monPrefs.get("perfmon.vLightGrid")%>">
		<param name="vMinuteGrid" value="<%=monPrefs.get("perfmon.vMinuteGrid")%>">
		<param name="port" value="<%=port%>">
		<param name="host" value="<%=request.getServerName()%>">
	</applet>
</body>
</html>
