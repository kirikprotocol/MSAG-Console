<%@include file="/WEB-INF/inc/header.jspf"%>
<sm:page title="SME's">
  <jsp:attribute name="menu">
    <sm-pm:menu>
      <sm-pm:item name="mbAdd" value="Add" title="Register new SME"/>
      <sm-pm:item name="mbDelete" value="Delete" title="Unregister checked SME(s)" isCheckSensible="true"/>
      <sm-pm:space/>
    </sm-pm:menu>
  </jsp:attribute>

  <jsp:body>
    <sm:table columns="checkbox,id,providerName,smsc,wantAlias" names="c,id,provider,is SMSC,wantAlias" widths="1,49,48,1,1" edit="id"/>
  </jsp:body>
</sm:page>