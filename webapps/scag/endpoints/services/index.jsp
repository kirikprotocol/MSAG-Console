<%@include file="/WEB-INF/inc/header.jspf"%>
<sm:page title="Services">
  <jsp:attribute name="menu">
    <sm-pm:menu>
      <sm-pm:item name="mbAdd" value="Add" title="Add new Service"/>
      <sm-pm:item name="mbDelete" value="Delete" title="Delete Services" isCheckSensible="true"/>
      <sm-pm:space/>
    </sm-pm:menu>
  </jsp:attribute>

  <jsp:body>
    <sm:table columns="checkbox,id,providerName, transportName" names="c,id,provider, transport" widths="1,49,48,48" edit="id"/>
  </jsp:body>
</sm:page>