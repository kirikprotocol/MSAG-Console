<%@ page import="ru.sibinco.scag.backend.SCAGAppContext" %>
<html>
<head>
    <link rel="STYLESHEET" type="text/css" href="/styles/common.css">
</head>

<body style="margin:0px 0px 0px 0px;">
<%
    SCAGAppContext appContext = (SCAGAppContext) request.getAttribute("appContext");
    long port = 6695;
    if (appContext != null) {
        try {
            port = appContext.getConfig().getInt("svcmon.appletport");
        } catch (Throwable e) {
            e.printStackTrace();
        }
    }
%>

<applet code="ru.sibinco.scag.svcmon.applet.SvcMon.class" width="100%" height="100%" archive="svcmon.jar">
    <param name="locale.country" value="us">
    <param name="locale.language" value="en">
    <param name="graph.scale" value="2">
    <param name="locales.messages" value="RUSSIA">
    <param name="graph.grid" value="2">
    <param name="graph.higrid" value="10">
    <param name="graph.head" value="20">
    <param name="max.speed" value="50">
    <param name="port" value="<%=port%>">
    <param name="host" value="<%=request.getServerName()%>">
</applet>

</body>

</html>