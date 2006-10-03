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
                var checkResult = null;
                if (action == "edit") checkResult = document.jedit.openRule(id);
                  else checkResult = document.jedit.newRule(id);
                if (checkResult)
                {
                  alertError(checkResult);
                  opener.submit0();
                }
              }

              function alertError(checkResult) {
                if (checkResult == 'jEdit.rule_state.existError')
                  opener.alert("<fmt:message>jEdit.rule_state.existError</fmt:message>");
                else if (checkResult == 'jEdit.rule_state.notExistError')
                  opener.alert("<fmt:message>jEdit.rule_state.notExistError</fmt:message>");
                else if (checkResult == 'jEdit.rule_state.lockedError')
                  opener.alert("<fmt:message>jEdit.rule_state.lockedError</fmt:message>");                  
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
                   /*if (action == "addRule")*/ opener.submit0();
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