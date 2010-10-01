<%@ taglib prefix="c" uri="http://java.sun.com/jsp/jstl/core" %>
<%@ taglib prefix="fmt" uri="http://java.sun.com/jsp/jstl/fmt" %>
<%@ page import="ru.sibinco.scag.backend.SCAGAppContext,
                 ru.sibinco.scag.backend.users.UserPreferences,
                 ru.sibinco.scag.backend.users.ScagUser,
                 java.util.Map" %>
<%@ page import="ru.sibinco.scag.Constants"%>
<html>
<head>
    <link rel="STYLESHEET" type="text/css" href="content/styles/common.css">
</head>

<body style="margin:0px 0px 0px 0px;" onLoad="if (opener!=null) opener.document.opForm.submit()">
<%
    SCAGAppContext appContext = (SCAGAppContext) request.getAttribute(Constants.APP_CONTEXT);
    long port = 6695;
    UserPreferences prefs = new UserPreferences();
    if (appContext != null) {
        try {
            if (request.getUserPrincipal()!=null)
              prefs = ((ScagUser)appContext.getUserManager().getUsers().get(request.getUserPrincipal().getName())).getPrefs();
            port = appContext.getConfig().getInt("scmon.appletport");
        } catch (Throwable e) {
            e.printStackTrace();
        }
    }
    Map monPrefs = prefs.getMonPrefs();
%>

<applet code="ru.sibinco.scag.scmon.applet.ScMon.class" width="100%" height="100%" archive="scmon.jar">
    <param name="resource_servlet_uri" value="<%=request.getContextPath()+"/resource/?"%>">
    <param name="graph.scale" value="<%=monPrefs.get("scmon.graph.scale")%>">
    <param name="locales.messages" value="RUSSIA">
    <param name="locale" value="${locale}">
    <param name="graph.grid" value="<%=monPrefs.get("scmon.graph.grid")%>">
    <param name="graph.higrid" value="<%=monPrefs.get("scmon.graph.higrid")%>">
    <param name="graph.head" value="<%=monPrefs.get("scmon.graph.head")%>">
    <param name="max.speed" value="<%=monPrefs.get("scmon.max.speed")%>">
    <param name="port" value="<%=port%>">
    <param name="host" value="<%=request.getServerName()%>">
</applet>

</body>

</html>