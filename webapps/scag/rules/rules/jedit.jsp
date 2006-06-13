<%--
  Created by IntelliJ IDEA.
  User: dym
  Date: 18.05.2006
  Time: 17:58:30
  To change this template use File | Settings | File Templates.
--%>
<%@taglib uri="http://java.sun.com/jsp/jstl/fmt" prefix="fmt"%>
<html>
    <head><title>jEdit window</title></head>
    <script>
              function open()
              {
                opener.status = "<fmt:message>jEdit.starting</fmt:message>";
                action = "<%=request.getParameter("action")%>";
                id = "<%=request.getParameter("id")%>";
                transport = "<%=request.getParameter("transport")%>";
                if (action == "edit") document.jedit.openRule(id,transport);
                  else document.jedit.newRule(id,transport);
                setInterval("toClose()",1000);
              }
              function toClose()
              {
                if (document.jedit.isStopped()) {
                   window.close();
                   if (action == "add") opener.document.opForm.submit();
                   opener.status = "<fmt:message>jEdit.stopped</fmt:message>";
                } else {
                  opener.status = "<fmt:message>jEdit.started</fmt:message>";
                }
              }
    </script>

  <body onLoad="open()">
    <applet code="org.gjt.sp.jedit.jEdit.class" width="1" height="1" archive="jedit.jar" name=jedit ID=jedit>
       <param name="noplugins" value="-noplugins">
       <param name="homedir" value="applet">
       <param name="username" value="rules">
       <param name="servletUrl" value="${pageContext.request.contextPath}/applet/myServlet">
    </applet>
  </body>

</html>