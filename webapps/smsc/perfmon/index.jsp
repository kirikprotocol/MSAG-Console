<html>
<body>
<%
	ru.novosoft.smsc.jsp.SMSCAppContext appContext = (ru.novosoft.smsc.jsp.SMSCAppContext)request.getAttribute("appContext");
	int port = 6695;
	if (appContext != null)
	{
		try {
			port = appContext.getConfig().getInt("perfmon.appletport");
		} catch (Throwable e) {
			e.printStackTrace();
		}
	}
%>
	<applet code="ru.novosoft.smsc.perfmon.applet.PerfMon.class" width=460 height=400 archive="perfmon.jar">
		<param name="locale.country" value="ru">
		<param name="locale.language" value="ru">
		<param name="pixPerSecond" value="4">
		<param name="scale" value="140">
		<param name="block" value="7">
		<param name="vLightGrid" value="4">
		<param name="vMinuteGrid" value="6">
		<param name="port" value="<%=port%>">
		<param name="host" value="<%=request.getServerName()%>">
	</applet>
</body>
</html>
