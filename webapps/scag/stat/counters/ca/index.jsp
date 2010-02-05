<%@include file="/WEB-INF/inc/header.jspf"%>
<sm:page title="statistics.counters.ca.title">
  <jsp:attribute name="menu">
    <sm-pm:menu>
      <sm-pm:item name="mbAdd"    value="statistics.counters.ca.mbadd.value"
                                  title="statistics.counters.ca.mbadd.title"/>
      <sm-pm:item name="mbDelete" value="statistics.counters.ca.mbdel.value"
                                  title="statistics.counters.ca.mbdel.title"
                                  onclick="return deleteConfirm()" isCheckSensible="true"/>
      <sm-pm:space/>
    </sm-pm:menu>
  </jsp:attribute>

  <jsp:body>
    <sm:table columns="checkbox,id,system" names="c,statistics.counters.ca.edit.txt.id,statistics.counters.ca.edit.txt.system" widths="1,15,20,64" edit="id"/>
  </jsp:body>
</sm:page>