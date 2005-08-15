<%-- <%@include file="/WEB-INF/inc/header.jspf"%>
<sm:page title="Rules">

  <jsp:body>

<sm:applet />
  </jsp:body>
</sm:page>

--%>
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
	 <applet code="org.gjt.sp.jedit.jEdit.class" width="100%" height="100%" archive="jedit.jar" >
		<param name="noplugins" value="-noplugins">
		<param name="username" value="a">
		<param name="servletUrl" value="/scag/applet/myServlet">
  </applet>
</body>
</html>
--%>

