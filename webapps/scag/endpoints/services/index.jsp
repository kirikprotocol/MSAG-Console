<%@include file="/WEB-INF/inc/header.jspf"%>
<sm:page title="sp.index.title">
  <jsp:attribute name="menu">
    <sm-pm:menu>
      <sm-pm:item name="mbAdd" value="sp.index.item.mbadd.value" title="sp.index.item.mbadd.title"/>
      <sm-pm:item name="mbDelete" value="sp.index.item.mbdelete.value" title="sp.index.item.mbdelete.title" onclick="deleteConfirm()" isCheckSensible="true"/>
      <sm-pm:space/>
    </sm-pm:menu>
  </jsp:attribute>
  <jsp:body>
    <sm:table columns="checkbox,id,enabled,transport" names="c,sp.index.table.names.id,sp.index.table.names.enabled,sp.index.table.names.transport" widths="1,48,48,48,48" edit="id"/>
  </jsp:body>
</sm:page>