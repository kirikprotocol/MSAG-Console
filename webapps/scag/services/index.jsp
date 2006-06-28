<%@include file="/WEB-INF/inc/header.jspf"%>
<sm:page title="services.index.title">
  <jsp:attribute name="menu">
    <sm-pm:menu>
      <sm-pm:item name="mbAdd" value="services.index.item.mbadd.value" title="services.index.item.mbadd.title"/>
      <sm-pm:item name="mbDelete" value="services.index.item.mbdelete.value" title="services.index.item.mbdelete.title"
                  onclick="return deleteConfirm()" isCheckSensible="true"/>
      <sm-pm:space/>
    </sm-pm:menu>
  </jsp:attribute>

  <jsp:body>
    <sm:table columns="checkbox,name,servicesSize" names="c,services.index.table.names.name,services.index.table.names.servicessize" widths="1,20,48,48" edit="name"/>
  </jsp:body>
</sm:page>