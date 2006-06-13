<%@include file="/WEB-INF/inc/header.jspf"%>
<sm:page form_uri="j_security_check" form_method="post" title="login.title">
  <jsp:attribute name="menu">
    <sm-pm:menu>
      <sm-pm:item name="jsubmit" value="login.item.value" title="login.item.title"/>
      <sm-pm:space/>
    </sm-pm:menu>
  </jsp:attribute>

  <jsp:body>
    <c:if test="${request.loginError}">
      <div class=error><div class="header"><fmt:message>login.label.error</fmt:message></div><fmt:message>login.label.error.comment</fmt:message></div>
    </c:if>
    <sm-ep:properties title="login.properties">
      <sm-ep:txt title="login.txt.login" name="j_username" validation="nonEmpty"/>
      <sm-ep:txt title="login.txt.password" name="j_password" validation="nonEmpty" type="password"/>
    </sm-ep:properties>
  </jsp:body>
</sm:page>
