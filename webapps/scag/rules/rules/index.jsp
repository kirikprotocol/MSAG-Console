<%@ include file="/WEB-INF/inc/header.jspf" %>
<sm:page title="Rules">
    <jsp:attribute name="menu">
        <sm-pm:menu>
            <sm-pm:item name="mbAdd" value="Add" title="Add new rule" onclick="clearForm()"/>
            <sm-pm:item name="mbDelete" value="Delete" title="Delete rule" onclick="return confirm('Are you sure to delete this object?')" isCheckSensible="true"/>
            <sm-pm:space/>
        </sm-pm:menu>
    </jsp:attribute>

    <jsp:body>
      <script>
        function newRule()
        {
          document.jedit.newRule();
          return false;
        }
      </script>
      <applet code="org.gjt.sp.jedit.jEdit.class" codebase="rules/rules" width="1" height="1" archive="jedit.jar" name=jedit ID=jedit>
            <param name="noplugins" value="-noplugins">
            <param name="homedir" value="applet">
            <param name="username" value="rules">
            <param name="servletUrl" value="/scag/applet/myServlet">
       </applet>
        <sm:table columns="checkbox,id,name,providerName,transport" names="c,id,name,provider,transport"
                  widths="1,15,28,28,28" edit="id" goal="jedit"/>
      <c:choose>
            <c:when test="${bean.newRule}">
             <script>newRule();</script>
            </c:when>
      </c:choose>
    </jsp:body>
</sm:page>