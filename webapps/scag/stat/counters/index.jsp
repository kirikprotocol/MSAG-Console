<%@include file="/WEB-INF/inc/header.jspf"%>
<sm:page title="statistics.counters.title">
  <jsp:attribute name="menu">
    <sm-pm:menu>
      <sm-pm:item name="mbAdd"    value="statistics.counters.mbadd.value"
                                  title="statistics.counters.mbadd.title"/>
      <sm-pm:item name="mbDelete" value="statistics.counters.mbdel.value"
                                  title="statistics.counters.mbdel.title"
                                  onclick="return deleteConfirm()" isCheckSensible="true"/>
      <sm-pm:space/>
    </sm-pm:menu>
  </jsp:attribute>

  <jsp:body>
    <sm:table columns="checkbox,id,type,CATableId" names="c,statistics.counters.edit.txt.id,statistics.counters.edit.list.type,statistics.counters.edit.list.ca" widths="1,15,20,20,44" edit="id"/>
  </jsp:body>
</sm:page>