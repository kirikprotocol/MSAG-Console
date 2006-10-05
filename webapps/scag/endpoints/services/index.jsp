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
      <OBJECT id="tdcSvcStatuses" CLASSID="clsid:333C7BC4-460F-11D0-BC04-0080C7055A83">
            <PARAM NAME="DataURL" VALUE="/scag/endpoints/services/index.jsp">
            <PARAM NAME="UseHeader" VALUE="True">
            <PARAM NAME="TextQualifier" VALUE='"'>
        </OBJECT>
        <script>
            function refreshStatus() {
                document.getElementById('tdcSvcStatuses').DataURL = document.getElementById('tdcSvcStatuses').DataURL;
                document.getElementById('tdcSvcStatuses').reset();
                window.setTimeout(refreshStatus, 10000);
            }
            refreshStatus();
        </script>
    <sm:table columns="checkbox,id,enabled,connHost,connStatus" names="c,sp.index.table.names.id,sp.index.table.names.enabled,sp.index.table.names.host,sp.index.table.names.status" widths="1,30,10,40,40" edit="id"/>
  </jsp:body>
</sm:page>