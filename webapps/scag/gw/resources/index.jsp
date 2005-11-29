<%@include file="/WEB-INF/inc/header.jspf"%>
<sm:page title="Locale resources">
  <jsp:attribute name="menu">
    <sm-pm:menu>
      <sm-pm:item name="mbAdd" value="Add" title="Add new locale"/>
      <sm-pm:item name="mbDelete" value="Delete" title="Delete service center" onclick="return confirm('Are you sure to delete this object?')" isCheckSensible="true"/>
      <sm-pm:space/>
    </sm-pm:menu>
  </jsp:attribute>

  <jsp:body>
    <sm:table columns="checkbox,id" names="c,locale" widths="1,99" edit="id"/>
  </jsp:body>
</sm:page>