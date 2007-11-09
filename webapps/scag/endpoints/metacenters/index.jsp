<%@include file="/WEB-INF/inc/header.jspf"%>
<sm:page title="metacenters.index.title">
  <jsp:attribute name="menu">
    <sm-pm:menu>
      <sm-pm:item name="mbAdd" value="metaeps.index.item.mbadd.value" title="metaeps.index.item.mbadd.title"/>
      <sm-pm:item name="mbDelete" value="metaeps.index.item.mbdelete.value" title="metaeps.index.item.mbdelete.title"
                  onclick="return deleteConfirm()" isCheckSensible="true"/>
      <sm-pm:space/>
    </sm-pm:menu>
  </jsp:attribute>
    <jsp:body>
        <sm:smsc_table columns="checkbox,id"
                       names="c,metaeps.index.table.names.id"
                       widths="1,38" edit="id"/>
    </jsp:body>
</sm:page>