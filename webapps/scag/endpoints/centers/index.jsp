<%@include file="/WEB-INF/inc/header.jspf"%>
<sm:page title="centers.index.title" onLoad="refreshStatus()">
  <jsp:attribute name="menu">
    <sm-pm:menu>
      <sm-pm:item name="mbAdd" value="centers.index.item.mbadd.value" title="centers.index.item.mbadd.title"/>
      <sm-pm:item name="mbDelete" value="centers.index.item.mbdelete.value" title="centers.index.item.mbdelete.title"
                  onclick="return deleteConfirm()" isCheckSensible="true"/>
      <sm-pm:space/>
    </sm-pm:menu>
  </jsp:attribute>
    <jsp:body>        
        <sm:smsc_table columns="checkbox,id,enabled,connHostPort,connStatus" names="c,centers.index.table.names.id,centers.index.table.names.enabled,centers.index.table.names.host_port,centers.index.table.names.status" widths="1,38,20,38,40,20" edit="id"/>
    </jsp:body>
</sm:page>