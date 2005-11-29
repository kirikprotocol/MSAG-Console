<%@include file="/WEB-INF/inc/header.jspf"%>
<sm:page title="Billing rules">
  <jsp:attribute name="menu">
    <sm-pm:menu>
      <sm-pm:item name="mbAdd" value="Add" title="Add new rule"/>
      <sm-pm:item name="mbDelete" value="Delete" title="Delete rule" onclick="return confirm('Are you sure to delete this object?')" isCheckSensible="true"/>
      <sm-pm:space/>
    </sm-pm:menu>
  </jsp:attribute>

  <jsp:body>
    <sm:table columns="checkbox,id" names="c,name" widths="1,99" edit="id"/>
  </jsp:body>
</sm:page>