<html>
<head>
	<link rel="STYLESHEET" type="text/css" href="/styles/common.css">
</head>
<body style="margin: 0px 0px 0px 0px;">
<%
	ru.novosoft.smsc.jsp.SMSCAppContext appContext = (ru.novosoft.smsc.jsp.SMSCAppContext)request.getAttribute("appContext");
	int port = 6695;
	if (appContext != null)
	{
		try {
			port = appContext.getConfig().getInt("topmon.appletport");
		} catch (Throwable e) {
			e.printStackTrace();
		}
	}
%>
	<applet code="ru.novosoft.smsc.topmon.applet.TopMon.class" width=100% height=100% archive="topmon.jar">
		<param name="locale.country" value="us">
		<param name="locale.language" value="en">
		<param name="graph.scale" value="3">
		<param name="graph.grid" value="2">
		<param name="graph.hgrid" value="10">
		<param name="graph.head" value="20">
		<param name="max.speed" value="50">
		<param name="port" value="<%=port%>">
		<param name="host" value="<%=request.getServerName()%>">
	</applet>
</body>
</html>

