<%@include file="/WEB-INF/inc/header.jspf"%>
<sm:page title="Users">
  <jsp:attribute name="menu">
    <sm-pm:menu>
      <sm-pm:item name="mbAdd" value="Add" title="Add new user"/>
      <sm-pm:item name="mbDelete" value="Delete" title="Delete user" isCheckSensible="true"/>
    <sm-pm:space/>
    </sm-pm:menu>
  </jsp:attribute>
  
  <jsp:body>
    <sm:table columns="checkbox,login,firstName,lastName,dept" names="c,login,first name,last name,department" widths="1,25,25,25,25" edit="login"/>
  </jsp:body>
</sm:page>