<%@taglib uri="http://java.sun.com/jsp/jstl/core" prefix="c"%>
<%@taglib uri="http://java.sun.com/jsp/jstl/functions" prefix="fn"%>
<%@taglib uri="http://java.sun.com/jsp/jstl/fmt" prefix="fmt"%>
<%@taglib prefix="smf" uri="/scag/func"%>
<%@taglib prefix="sm-ep" tagdir="/WEB-INF/tags/edit/properties" %><%@
 tag body-content="empty" %><%@
 attribute name="head"       required="false"%><%@
 attribute name="title"       required="true"%><%@
 attribute name="indent"      required="false"%><%@
 attribute name="name"        required="true"%><%@
 attribute name="disabled"    required="false"%>
<c:choose>
  <c:when test="${!indent}">
    <sm-ep:property title="${head}" >
      <input class=check type=checkbox name="${name}" id="id_${fn:escapeXml(name)}" value="true" <c:if test="${bean[name]}">checked</c:if> <c:if test="${disabled}">disabled</c:if>>&nbsp;<label for="id_${fn:escapeXml(name)}"><fmt:message>${title}</fmt:message></label>
    </sm-ep:property>
  </c:when>
  <c:otherwise>
    <input class=check type=checkbox name="${name}" id="id_${fn:escapeXml(name)}" value="true" <c:if test="${bean[name]}">checked</c:if> <c:if test="${disabled}">disabled</c:if>>&nbsp;<label for="id_${fn:escapeXml(name)}"><fmt:message>${title}</fmt:message></label>
  </c:otherwise>
</c:choose>
<c:if test="${disabled}">
  <input type=hidden name="${name}" id="id_${fn:escapeXml(name)}" value="<c:if test="${bean[name]}">true</c:if>" >
</c:if>
