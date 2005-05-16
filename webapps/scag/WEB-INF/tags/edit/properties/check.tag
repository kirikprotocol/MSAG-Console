<%@taglib uri="http://java.sun.com/jsp/jstl/core" prefix="c"%><%@
 taglib uri="http://java.sun.com/jsp/jstl/functions" prefix="fn"%><%@
 taglib prefix="smf" uri="/smppgw/func"%><%@
 taglib prefix="sm-ep" tagdir="/WEB-INF/tags/edit/properties" %><%@
 tag body-content="empty" %><%@
 attribute name="head"       required="false"%><%@
 attribute name="title"       required="true"%><%@
 attribute name="indent"      required="false"%><%@
 attribute name="name"        required="true"%>
<c:choose>
 <c:when test="${!indent}">
 <sm-ep:property title="${head}" >
  <input class=check type=checkbox name="${name}" id="id_${fn:escapeXml(name)}" value="true" <c:if test="${bean[name]}">checked</c:if>>&nbsp;<label for="id_${fn:escapeXml(name)}">${title}</label>
 </sm-ep:property>
</c:when>
<c:otherwise>
  <input class=check type=checkbox name="${name}" id="id_${fn:escapeXml(name)}" value="true" <c:if test="${bean[name]}">checked</c:if>>&nbsp;<label for="id_${fn:escapeXml(name)}">${title}</label>
</c:otherwise>
</c:choose>

