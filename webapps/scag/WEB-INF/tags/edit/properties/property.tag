<%@ taglib uri="http://java.sun.com/jsp/jstl/core" prefix="c" %>
<%@ taglib uri="http://java.sun.com/jsp/jstl/functions" prefix="fn" %>
<%@ taglib uri="http://java.sun.com/jsp/jstl/fmt" prefix="fmt" %>
<%@ tag body-content="scriptless" %>
<%@ attribute name="title" required="true" %>
<%@ attribute name="rowId" required="false" %>
<%@ attribute name="style" required="false" %>

<tr style="${style}"
    <c:if test="${!empty rowId}">id="${rowId}"</c:if>
    <c:if test="${!SCAG_EDIT_PROPERTIES_noColoredLines}">class="row${prop_rowN%2}"</c:if>>
    <c:set var="prop_rowN" value="${prop_rowN+1}" scope="request"/>
    <c:if test="${!SCAG_EDIT_PROPERTIES_noHeaders}"><th><c:if test="${!empty title}"><fmt:message>${fn:escapeXml(title)}</fmt:message></c:if></th></c:if>
    <td>
        <jsp:doBody/>
    </td>
</tr>