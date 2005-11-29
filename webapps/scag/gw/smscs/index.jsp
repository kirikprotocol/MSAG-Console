<%@include file="/WEB-INF/inc/header.jspf"%>
<sm:page title="Service centers">
  <jsp:attribute name="menu">
    <sm-pm:menu>
      <sm-pm:item name="mbAdd" value="Add" title="Add new service center"/>
      <sm-pm:item name="mbDelete" value="Delete" title="Delete service center" onclick="return confirm('Are you sure to delete this object?')" isCheckSensible="true"/>
      <sm-pm:space/>
    </sm-pm:menu>
  </jsp:attribute>

  <jsp:body>
    <sm:table columns="checkbox,id,host,port" names="c,name,host,port" widths="1,33,33,33" edit="id"/>
  </jsp:body>
</sm:page>