<%@taglib uri="http://java.sun.com/jsp/jstl/core" prefix="c"%><%@
 taglib uri="http://java.sun.com/jsp/jstl/functions" prefix="fn"%><%@
 tag body-content="scriptless"%><%@attribute name="title" required="true"%>
<tr <c:if test="${!SCAG_EDIT_TREE_noColoredLines}">class="row${prop_rowN%2}"</c:if>><c:set var="prop_rowN" value="${prop_rowN+1}" scope="request"/>
	<c:if test="${!SCAG_EDIT_TREE_noHeaders}"><th>${fn:escapeXml(title)}</th></c:if>
	<td><jsp:doBody/></td>
</tr>