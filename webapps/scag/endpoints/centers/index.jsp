<%@include file="/WEB-INF/inc/header.jspf"%>
<sm:page title="Centers">
  <jsp:attribute name="menu">
    <sm-pm:menu>
      <sm-pm:item name="mbAdd" value="Add" title="Add new Center"/>
      <sm-pm:item name="mbDelete" value="Delete" title="Delete Centers" isCheckSensible="true"/>
      <sm-pm:space/>
    </sm-pm:menu>
  </jsp:attribute>

  <jsp:body>
    <sm:table columns="checkbox,id,providerName" names="c,id" widths="1,49,48" edit="id"/>
  </jsp:body>
</sm:page>