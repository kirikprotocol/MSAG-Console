<%@include file="/WEB-INF/inc/header.jspf"%>
<sm:page title="Operators">
  <jsp:attribute name="menu">
    <sm-pm:menu>
      <sm-pm:item name="mbAdd" value="Add" title="Add new Operator"/>
      <sm-pm:item name="mbDelete" value="Delete" title="Delete Operators" onclick="return confirm('Are you sure to delete this object?')" isCheckSensible="true"/>
      <sm-pm:space/>
    </sm-pm:menu>
  </jsp:attribute>
  <jsp:body>
    <sm:table columns="checkbox,name,description" names="c,name,description" widths="1,48,48" edit="name"/>
  </jsp:body>
</sm:page>