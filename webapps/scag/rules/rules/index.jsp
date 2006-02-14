<%@ include file="/WEB-INF/inc/header.jspf" %>
<sm:page title="SCAG Rules">
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
        function changeTransportId() {
              var transport = opForm.all.transportId.options[opForm.all.transportId.selectedIndex].value;
              opForm.submit();
              return true;
         }
        function changeProviderId() {
              var provider = opForm.all.providerId.options[opForm.all.providerId.selectedIndex].value;
              opForm.submit();
              return true;
         }
      </script>
      <applet code="org.gjt.sp.jedit.jEdit.class" codebase="rules/rules" width="1" height="1" archive="jedit.jar" name=jedit ID=jedit>
            <param name="noplugins" value="-noplugins">
            <param name="homedir" value="applet">
            <param name="username" value="rules">
            <param name="servletUrl" value="/scag/applet/myServlet">
      </applet>

      <sm-ep:properties title="Filter(s)" noEdit="true">

         <sm-ep:list title="Transport" onChange="changeTransportId();" name="transportId"
                          values="${fn:join(bean.transportIds, ',')}"
                          valueTitles="${fn:join(bean.transportTitles, ',')}"/>

         <c:choose>
           <c:when test="${bean.administrator}">
             <sm-ep:list title="Provider" onChange="changeProviderId();" name="providerId"
                             values="${fn:join(bean.providerIds, ',')}"
                             valueTitles="${fn:join(bean.providerNames, ',')}"/>
           </c:when>
         </c:choose>
      </sm-ep:properties>
      <br>
      <sm:table columns="checkbox,id,name,providerName,transport" names="c,id,name,provider,transport"
                widths="1,15,28,28,28" edit="id" goal="jedit"/>
      <c:choose>
            <c:when test="${bean.newRule}">
             <script>newRule();</script>
            </c:when>
      </c:choose>
    </jsp:body>
</sm:page>