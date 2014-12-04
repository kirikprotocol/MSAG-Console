<%@include file="/WEB-INF/inc/header.jspf"%>
<c:if test="${param.access_denied}">
 <c:redirect url="/access_denied.jsp"/>
</c:if>
<sm:page form_uri="j_security_check" form_method="post" title="login.title">
  <jsp:attribute name="menu">
    <sm-pm:menu>
      <sm-pm:item name="jsubmit" value="login.item.value" title="login.item.title"/>
      <sm-pm:space/>
    </sm-pm:menu>
  </jsp:attribute>

  <jsp:body>
    <c:if test="${param.loginError}">
      <div class=error><div class="header"><fmt:message>login.label.error</fmt:message></div><fmt:message>login.label.error.comment</fmt:message></div>
    </c:if>
    <sm-ep:properties title="login.properties">
      <sm-ep:txt title="login.txt.login" name="username" validation="nonEmpty"/>
      <sm-ep:txt title="login.txt.password" name="password" validation="nonEmpty" type="password"/>
    </sm-ep:properties>
  </jsp:body>
</sm:page>
