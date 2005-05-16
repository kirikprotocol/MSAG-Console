<%@
 taglib uri="http://java.sun.com/jsp/jstl/core" prefix="c"%><%@
 taglib uri="http://java.sun.com/jsp/jstl/functions" prefix="fn"%><%@
 taglib prefix="sm" tagdir="/WEB-INF/tags"%><%@
 tag body-content="empty"              %><%@
 attribute name="deep" required="true" %>
<dl>
  <c:forEach begin="1" end="${deep}" var="i">
    <dt>dt ${i}</dt>
    <dd>
      dd ${i}/${deep}
      <c:if test="${ i != deep }"><sm:test deep="${i}"/></c:if>
    </dd>
  </c:forEach>
</dl>