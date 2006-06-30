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
              function openjEditView(action,id)
              {
                document.focus();
                if (action == "edit") document.jedit.openRule(id);
                  else document.jedit.newRule(id);
              }

              function openjEditWindow(action,id)
              {
                opener.status = "<fmt:message>jEdit.starting</fmt:message>";
                openjEditView(action,id);
                toClose();
              }

              function toClose()
              {
                var action = document.jedit.isWindowClosed();
                if (document.jedit.isWindowClosed()) {
                   if (action == "addRule") opener.document.opForm.submit();
                   if (document.jedit.isStopped()) {
                     window.close();
                     opener.status = "<fmt:message>jEdit.stopped</fmt:message>";
                     opener.jEdit = null;
                     return;
                   }
                } else {
                  opener.jEdit = window;
                  opener.status = "<fmt:message>jEdit.started</fmt:message>";
                }
                setTimeout(toClose,1000);
              }
    </script>

  <body onLoad="openjEditWindow('<%=request.getParameter("action")%>','<%=request.getParameter("id")%>')">
    <applet code="org.gjt.sp.jedit.jEdit.class" width="1" height="1" archive="jedit.jar" name=jedit ID=jedit>
       <param name="noplugins" value="-noplugins">
       <param name="homedir" value="applet">
       <param name="username" value="rules">
       <param name="servletUrl" value="${pageContext.request.contextPath}/applet/myServlet">
    </applet>
  </body>

</html>