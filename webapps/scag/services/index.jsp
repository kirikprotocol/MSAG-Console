<%@include file="/WEB-INF/inc/header.jspf"%>
<sm:page title="Service Providers">
  <jsp:attribute name="menu">
    <sm-pm:menu>
      <sm-pm:item name="mbAdd" value="Add Provider" title="Create new Service Providers"/>
      <sm-pm:item name="mbDelete" value="Delete" title="Delete Service Providers" onclick="return confirm('Are you sure to delete this object?')" isCheckSensible="true"/>
      <sm-pm:space/>
    </sm-pm:menu>
  </jsp:attribute>

  <jsp:body>
    <sm:table columns="checkbox,name,servicesSize" names="c,name,services num" widths="1,20,48,48" edit="name"/>
  </jsp:body>
</sm:page>