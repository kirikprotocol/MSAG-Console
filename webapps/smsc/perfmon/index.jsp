<%@ page import="ru.novosoft.smsc.admin.preferences.UserPreferences,
                 java.util.HashMap"%><html>
<head>
	<link rel="STYLESHEET" type="text/css" href="/styles/common.css">
</head>
<body style="margin: 0px 0px 0px 0px;">
<%
	ru.novosoft.smsc.jsp.SMSCAppContext appContext = (ru.novosoft.smsc.jsp.SMSCAppContext)request.getAttribute("appContext");
	int port = 6695;
	UserPreferences prefs = new UserPreferences();
	if (appContext != null)
	{
		try {
			prefs = appContext.getUserManager().getLoginedUser().getPrefs();
			port = appContext.getConfig().getInt("perfmon.appletport");
		} catch (Throwable e) {
			e.printStackTrace();
		}
	}
	HashMap perfmonPrefs = prefs.getPerfmonPrefs();
%>
	<applet code="ru.novosoft.smsc.perfmon.applet.PerfMon.class" width=100% height=100% archive="perfmon.jar">
		<param name="locale.country" value=<%=prefs.getLocale().getLanguage()%>>
		<param name="locale.language" value=<%=prefs.getLocale().getLanguage()%>>
		<param name="pixPerSecond" value=<%=perfmonPrefs.get("perfmon.pixPerSecond")%>>
		<param name="scale" value=<%=perfmonPrefs.get("perfmon.scale")%>>
		<param name="block" value=<%=perfmonPrefs.get("perfmon.block")%>>
		<param name="vLightGrid" value=<%=perfmonPrefs.get("perfmon.vLightGrid")%>>
		<param name="vMinuteGrid" value=<%=perfmonPrefs.get("perfmon.vMinuteGrid")%>>
		<param name="port" value="<%=port%>">
		<param name="host" value="<%=request.getServerName()%>">
	</applet>
</body>
</html>
