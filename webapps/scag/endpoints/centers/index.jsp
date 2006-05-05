<%@include file="/WEB-INF/inc/header.jspf"%>
<sm:page title="Center points">
  <jsp:attribute name="menu">
    <sm-pm:menu>
      <sm-pm:item name="mbAdd" value="Add" title="Add new Center point"/>
      <sm-pm:item name="mbDelete" value="Delete" title="Delete Center point(s)" onclick="return confirm('Are you sure to delete this object?')" isCheckSensible="true"/>
      <sm-pm:space/>
    </sm-pm:menu>
  </jsp:attribute>

  <jsp:body>
    <sm:table columns="checkbox,id,enabled,transport" names="c,id,enabled,transport" widths="1,48,48,48,48" edit="id"/>
  </jsp:body>
</sm:page>