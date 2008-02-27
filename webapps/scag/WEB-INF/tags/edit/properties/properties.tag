<%@ taglib uri="http://java.sun.com/jsp/jstl/core" prefix="c" %>
<%@ taglib uri="http://java.sun.com/jsp/jstl/functions" prefix="fn" %>
<%@ taglib uri="http://java.sun.com/jsp/jstl/fmt" prefix="fmt" %>
<%@ tag body-content="scriptless" %>
<%@ attribute name="title" required="false" %>
<%@ attribute name="noColoredLines" required="false" %>
<%@ attribute name="noHeaders" required="false" %>
<%@ attribute name="noEdit" required="false" %>
<%@ attribute name="id" required="false" %>
<%@ attribute name="display" required="false" %>

<c:set var="SCAG_EDIT_PROPERTIES_noColoredLines" value="${noColoredLines}" scope="request"/>
<c:set var="SCAG_EDIT_PROPERTIES_noHeaders" value="${noHeaders}" scope="request"/>

<c:if test="${empty noEdit || !noEdit}">
    <input type="hidden" name="editId" value="${fn:escapeXml(bean.editId)}">
    <input type="hidden" name="add" value="${fn:escapeXml(bean.add)}">
</c:if>

<div class=page_subtitle><c:if test="${!empty title}"><fmt:message>${fn:escapeXml(title)}</fmt:message></c:if></div>

<table  <c:if test="${!empty id}">id="${id}"</c:if>
        <c:if test="${! empty display}">style="display:${display}"</c:if> class=properties_list cellspacing=0 cellpadding=0>
        <c:if test="${!SCAG_EDIT_PROPERTIES_noHeaders}">
            <col width="15%">
            <col width="100%">
        </c:if>
        <c:set var="prop_rowN" value="0" scope="request"/>
    <jsp:doBody/>
</table>