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
			port = appContext.getConfig().getInt("topmon.appletport");
		} catch (Throwable e) {
			e.printStackTrace();
		}
	}
	HashMap topmonPrefs = prefs.getTopmonPrefs();
%>
	<applet code="ru.novosoft.smsc.topmon.applet.TopMon.class" width=100% height=100% archive="topmon.jar">
		<param name="locale.country" value=<%=prefs.getLocale().getLanguage()%>>
		<param name="locale.language" value=<%=prefs.getLocale().getLanguage()%>>
		<param name="graph.scale" value=<%=topmonPrefs.get("topmon.graph.scale")%>>
		<param name="graph.grid" value=<%=topmonPrefs.get("topmon.graph.grid")%>>
		<param name="graph.higrid" value=<%=topmonPrefs.get("topmon.graph.higrid")%>>
		<param name="graph.head" value=<%=topmonPrefs.get("topmon.graph.head")%>>
		<param name="max.speed" value=<%=topmonPrefs.get("topmon.max.speed")%>>
		<param name="port" value="<%=port%>">
		<param name="host" value="<%=request.getServerName()%>">
	</applet>
</body>
</html>

