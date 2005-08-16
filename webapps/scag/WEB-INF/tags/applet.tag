<%@
 tag body-content="empty" %><%@
 attribute name="className" required="false"%><%
	ru.sibinco.scag.backend.SCAGAppContext appContext =
          (ru.sibinco.scag.backend.SCAGAppContext) request.getAttribute("appContext");
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
	 <applet code="org.gjt.sp.jedit.jEdit.class" width=100% height=100% archive="jedit.jar" >
		<param name="noplugins" value="-noplugins">
		<param name="username" value="a">
		<param name="servletUrl" value="/scag/applet/myServlet">
  </applet>
