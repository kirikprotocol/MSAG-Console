<%@ page import="ru.sibinco.scag.backend.SCAGAppContext"%>
 <%--
  Created by IntelliJ IDEA.
  User: dym
  Date: 18.05.2006
  Time: 17:58:30
  To change this template use File | Settings | File Templates.
--%>
<%@taglib uri="http://java.sun.com/jsp/jstl/fmt" prefix="fmt"%>
<html>
    <head>
        <link rel="STYLESHEET" type="text/css" href="content/styles/common.css">
        <title>jEdit window</title></head>
    <script>
              function openjEditView()
              {
                if (opener.submit0)
                {
                    opener.submit0();
                }
//o                document.focus();
                window.focus();
              }

              function openjEditWindow()
              {
//o                opener.jEditStarting();
                jEditStarting();
//o                opener.assignjEditOpener(window,false);
                assignjEditOpener(window, false);

                openjEditView();

                fireButtons();

                if (opener.assignjEditOpener) {
                    opener.assignjEditOpener(window,true);
                }

              }

              function fireButtons(){
                var stop = false;
                if (opener.submit0)
                {
                    stop = opener.submit0();
                }

                if (!stop){
                    setTimeout(fireButtons,1000);
                    //console.debug("set timer");
                }
              }
// added
                function jEditStarting()
                {
                    if (opener && !opener.closed) {
                      opener.status = "<fmt:message>jEdit.starting</fmt:message>";
                    } else {
                      window.status = "<fmt:message>jEdit.starting</fmt:message>";
                    }
                }

                function assignjEditOpener(jEditwindow, marker)
                {
                    if (opener && !opener.closed) {
                        opener.jEdit = jEditwindow;
                        if (marker) opener.status = "<fmt:message>jEdit.started</fmt:message>";
                    } else {
                        window.jEdit = jEditwindow;
                        if (marker) window.status = "<fmt:message>jEdit.started</fmt:message>";
                    }
                }
//-added

              function toClose()
              {
                   if (opener.submit0) {
                        opener.submit0();
                   }

                   if (opener.closejEditWindow) {
                       opener.closejEditWindow();
                   }
              }
    </script>

  <body style="margin:0px 0px 0px 0px;" onLoad="openjEditWindow()" onunload="toClose()">
    <applet code="org.gjt.sp.jedit.jEdit.class" width="100%" height="100%" archive="jedit.jar" name=jedit ID=jedit>
       <!-- param name="jnlp_href" value="/msag/rules/rules/jedit2applet.jnlp"/-->
       <param name="noplugins" value="-noplugins">
       <param name="homedir" value="applet">
       <param name="username" value="rules">
       <param name="servletUrl" value="${pageContext.request.contextPath}/applet/myServlet">
       <param name="ping_host" value="<%=((SCAGAppContext)request.getAttribute("appContext")).getLiveConnect().getPingHost()%>">
       <param name="ping_port" value="<%=((SCAGAppContext)request.getAttribute("appContext")).getLiveConnect().getPingPort()%>">
       <param name="ping_timeout" value="<%=((SCAGAppContext)request.getAttribute("appContext")).getLiveConnect().getTimeout()%>">
       <param name="action" value="<%=request.getParameter("action")%>">
       <param name="id" value="<%=request.getParameter("id")%>">
    </applet>
  </body>

</html>