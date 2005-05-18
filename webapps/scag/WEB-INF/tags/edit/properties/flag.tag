<%@taglib uri="http://java.sun.com/jsp/jstl/core" prefix="c"%><%@
 taglib uri="http://java.sun.com/jsp/jstl/functions" prefix="fn"%><%@
 taglib prefix="smf" uri="/scag/func"%><%@
 taglib prefix="sm-ep" tagdir="/WEB-INF/tags/edit/properties" %><%@
 tag body-content="empty" %><%@
 attribute name="title"       required="true"%><%@
 attribute name="name"        required="true"%>
<sm-ep:property title="${title}">
  <c:choose>
    <c:when test="${bean[name]}"><img src="/images/ic_checked.gif"></c:when>
    <c:otherwise>&nbsp;</c:otherwise>
  </c:choose>
  <input type=hidden name="${name}" value="<c:choose><c:when test='${bean[name]}'>true</c:when><c:otherwise>false</c:otherwise></c:choose>">
</sm-ep:property>