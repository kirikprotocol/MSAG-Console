<%@taglib uri="http://java.sun.com/jsp/jstl/core" prefix="c"%><%@
 taglib uri="http://java.sun.com/jsp/jstl/functions" prefix="fn"%><%@
 taglib prefix="sm-ep" tagdir="/WEB-INF/tags/edit/properties" %>
<%@tag body-content="empty" %>
<%@attribute name="title"      required="true"%>
<%@attribute name="name"       required="true"%>
<%@attribute name="maxlength"  required="false"%>
<%@attribute name="styleWidth" required="false"%>
<%@attribute name="readonly"   required="false"%>
<%@attribute name="type"       required="false"%>
<%@attribute name="validation" required="false"%>
<sm-ep:property title="${title}">
    <input class=txt type="${empty type ? 'text' : type}" maxlength="${maxlength}"
            <c:if test="${!empty styleWidth}">style="width:${styleWidth}"</c:if>
            name="${fn:escapeXml(name)}"
            value=
            <c:choose>
                <c:when test="${fn:escapeXml(bean[name])==0 || fn:escapeXml(bean[name])==-1}">""</c:when>
                <c:otherwise>"${fn:escapeXml(bean[name])}"</c:otherwise>
            </c:choose>
            <c:if test="${readonly}"> readonly</c:if>
            <c:if test="${!empty validation}"> validation="${validation}" onkeyup="resetValidation(this)"</c:if>
            onchange="this.value=trim(this.value);">
</sm-ep:property>