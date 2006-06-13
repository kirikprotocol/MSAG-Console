<%@include file="/WEB-INF/inc/header.jspf"%>
<sm:page title="users.index.title">
  <jsp:attribute name="menu">
    <sm-pm:menu>
      <sm-pm:item name="mbAdd" value="users.index.item.mbadd.value" title="users.index.item.mbadd.title"/>
      <sm-pm:item name="mbDelete" value="users.index.item.mbdelete.value" title="users.index.item.mbdelete.title" onclick="return confirm('<fmt:message>users.index.item.mbdelete.onclick<fmt:message>')" isCheckSensible="true"/>
    <sm-pm:space/>
    </sm-pm:menu>
  </jsp:attribute>
  
  <jsp:body>
    <sm:table columns="checkbox,login,firstName,lastName,dept" names="c,users.index.table.names.login,users.index.table.names.firstname,users.index.table.names.lastname,users.index.table.names.dept" widths="1,25,25,25,25" edit="login"/>
  </jsp:body>
</sm:page>