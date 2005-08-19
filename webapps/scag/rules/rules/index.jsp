<%@include file="/WEB-INF/inc/header.jspf"%>
<sm:page title="Rules">
  <jsp:attribute name="menu">
    <sm-pm:menu>
      <sm-pm:item name="mbAdd" value="Add" title="Add new rule"/>
      <sm-pm:item name="mbDelete" value="Delete" title="Delete rule" isCheckSensible="true"/>
      <sm-pm:space/>
    </sm-pm:menu>
  </jsp:attribute>

  <jsp:body>
    <sm:table columns="checkbox,name,providerName,notes" names="c,name,provider,notes" widths="1,39,39,21" edit="name" target="jedit"/>
  </jsp:body>
</sm:page>