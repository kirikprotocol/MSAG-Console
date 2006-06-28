<%@include file="/WEB-INF/inc/header.jspf"%>
<sm:page title="Providers">
  <jsp:attribute name="menu">
    <sm-pm:menu>
      <sm-pm:item name="mbAdd" value="Add" title="Add new provider"/>
      <sm-pm:item name="mbDelete" value="Delete" title="Delete provider" onclick="return deleteConfirm()" isCheckSensible="true"/>
      <sm-pm:space/>
    </sm-pm:menu>
  </jsp:attribute>

  <jsp:body>
    <sm:table columns="checkbox,name" names="c,name" widths="1,15,84" edit="name"/>
  </jsp:body>
</sm:page>