<%@taglib uri="http://java.sun.com/jsp/jstl/core" prefix="c"%><%@
 taglib uri="http://java.sun.com/jsp/jstl/functions" prefix="fn"%><%@
 taglib uri="http://java.sun.com/jsp/jstl/fmt" prefix="fmt"%><%@
 taglib prefix="smf" uri="/scag/func"%><%@
 taglib prefix="sm-ep" tagdir="/WEB-INF/tags/edit/properties" %><%@
 tag body-content="empty" %><%@
 attribute name="title"       required="true"%><%@
 attribute name="name"        required="true"%><%@
 attribute name="values"      required="true"%><%@
 attribute name="valueTitles" required="true"%><%@
 attribute name="onClick"     required="false"%>
<sm-ep:property title="${title}"
  ><c:set var="pValues" value="${fn:split(values, ',')}"
 /><c:set var="valueTitles" value="${fn:split(valueTitles, ',')}"
 /><c:set var="checks" value="${bean[name]}"
 /><c:forEach var="i" items="${values}" varStatus="st"
  ><input class=check type=checkbox name="${name}" id="id_${fn:escapeXml(i)}" value="${i}" <c:if test="${smf:contains(checks, i)}">checked</c:if> <c:if test="${!empty onClick}">onClick="${onClick}"</c:if>>&nbsp;<label for="id_${fn:escapeXml(i)}"><fmt:message>${valueTitles[st.count-1]}</fmt:message></label><c:if test="${st.count < fn:length(values)}"><br>
</c:if></c:forEach
></sm-ep:property>