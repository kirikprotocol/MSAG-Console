<%@taglib uri="http://java.sun.com/jsp/jstl/core" prefix="c"%><%@
 taglib uri="http://java.sun.com/jsp/jstl/functions" prefix="fn"%><%@
 taglib prefix="smf" uri="/smppgw/func"%><%@
 taglib prefix="sm-ep" tagdir="/WEB-INF/tags/edit/properties" %><%@
 tag body-content="empty" %><%@
 attribute name="title"       required="true"%><%@
 attribute name="name"        required="true"%>
<sm-ep:property title="" >
  <input class=check type=checkbox name="${name}" id="id_${fn:escapeXml(name)}" value="true" <c:if test="${bean[name]}">checked</c:if>>&nbsp;<label for="id_${fn:escapeXml(name)}">${title}</label>
</sm-ep:property>