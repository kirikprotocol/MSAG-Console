<%@include file="/WEB-INF/inc/header.jspf"%>
<sm:page title="Subjects">
  <jsp:attribute name="menu">
    <sm-pm:menu>
      <sm-pm:item name="mbAdd" value="Add" title="Add new subject"/>
      <sm-pm:item name="mbDelete" value="Delete" title="Delete subject" isCheckSensible="true"/>
      <sm-pm:space/>
    </sm-pm:menu>
  </jsp:attribute>

  <jsp:body>
    <sm:table columns="checkbox,name,notes" names="c,name,notes" widths="1,49,50" edit="name"/>
  </jsp:body>
</sm:page>