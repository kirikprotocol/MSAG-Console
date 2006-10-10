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
                var checkResult = null;
                if (action == "edit") checkResult = document.jedit.openRule(id);
                  else checkResult = document.jedit.newRule(id);
                if (checkResult)
                {
                  alertError(checkResult);
                }
                if (opener.submit0) opener.submit0();
                document.focus();
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
                opener.jEditStarting();
                opener.assignjEditOpener(window,false);
                openjEditView(action,id);
                toClose();
              }

              function toClose()
              {
                var action = document.jedit.isWindowClosed();
                if (action) {
                   if (opener.submit0) opener.submit0();
                   if (document.jedit.isStopped()) {
                     if (opener.closejEditWindow) opener.closejEditWindow();
                     return;
                   }
                } else {
                  if (opener.assignjEditOpener) opener.assignjEditOpener(window,true);
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