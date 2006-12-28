<%@include file="/WEB-INF/inc/header.jspf"%>
<sm:page title="sp.index.title" onLoad="refreshStatus()">
  <jsp:attribute name="menu">
    <sm-pm:menu>
      <sm-pm:item name="mbAdd" value="sp.index.item.mbadd.value" title="sp.index.item.mbadd.title"/>
      <sm-pm:item name="mbDelete" value="sp.index.item.mbdelete.value" title="sp.index.item.mbdelete.title"
                  onclick="return deleteConfirm()" isCheckSensible="true"/>
      <sm-pm:space/>
    </sm-pm:menu>
  </jsp:attribute>
  <jsp:body>
    <sm:svc_table columns="checkbox,id,enabled,connHost,connStatus" names="c,sp.index.table.names.id,sp.index.table.names.enabled,sp.index.table.names.host,sp.index.table.names.status" widths="1,38,20,38,40,20" edit="id"/>
  </jsp:body>
</sm:page>