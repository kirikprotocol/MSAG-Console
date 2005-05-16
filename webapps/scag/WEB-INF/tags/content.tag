<%@taglib uri="http://java.sun.com/jsp/jstl/core" prefix="c"%><%@ tag body-content="scriptless"%><%@attribute name="menu" required="false"%><%@attribute name="menu2" required="false"%>
${menu}
<div class=content>
<jsp:doBody/>
</div>
<c:choose>
  <c:when test="${!empty menu2}">${menu2}</c:when>
  <c:otherwise>${menu}</c:otherwise>
</c:choose>
