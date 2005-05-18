<%@ page import="ru.sibinco.scag.backend.SCAGAppContext"%><html>
<head>
	<link rel="STYLESHEET" type="text/css" href="/styles/common.css">
</head>
<body style="margin: 0px 0px 0px 0px;">
<%
	SCAGAppContext appContext = (SCAGAppContext) request.getAttribute("appContext");
	long port = 6695;
	if (appContext != null)
	{
		try {
			port = appContext.getConfig().getInt("perfmon.appletport");
		} catch (Throwable e) {
			e.printStackTrace();
		}
	}
%>
	<applet code="ru.sibinco.scag.perfmon.applet.PerfMon.class" width="100%" height="100%" archive="perfmon.jar">
		<param name="locale.country" value="us">
		<param name="locale.language" value="en">
		<param name="pixPerSecond" value="4">
		<param name="scale" value="80">
		<param name="block" value="8">
		<param name="vLightGrid" value="4">
		<param name="vMinuteGrid" value="6">
		<param name="port" value="<%=port%>">
		<param name="host" value="<%=request.getServerName()%>">
	</applet>
</body>
</html>
