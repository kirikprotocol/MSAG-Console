<%@include file="/WEB-INF/inc/header.jspf"%>
<sm:page form_uri="j_security_check" form_method="post" title="Login to SMPP GW">
  <jsp:attribute name="menu">
    <sm-pm:menu>
      <sm-pm:item name="jsubmit" value="Login" title="Login with provided username and password"/>
      <sm-pm:space/>
    </sm-pm:menu>
  </jsp:attribute>

  <jsp:body>
    <c:if test="${request.loginError}">
      <div class=error><div class="header">Login error</div>Login name or password are invalid</div>
    </c:if>
    <sm-ep:properties title="Login">
      <sm-ep:txt title="login" name="j_username" validation="nonEmpty"/>
      <sm-ep:txt title="password" name="j_password" validation="nonEmpty" type="password"/>
    </sm-ep:properties>
  </jsp:body>
</sm:page>
