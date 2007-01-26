<%@ taglib uri="http://java.sun.com/jsp/jstl/core" prefix="c"%>
<%@ taglib uri="http://java.sun.com/jsp/jstl/functions" prefix="fn"%>
<%@ attribute name="name" required="true"%>
<%@ attribute name="fullName" required="true"%>
<%@ attribute name="priority" required="true"%>
<table class="properties_list" cellspacing="0">
    <col width="150px">
    <tr class=row${rowNum%2}>
        <th nowrap>${name}</th>
        <td>
            <select name="category_${fullName}" id="category_${fullName}" class="txt" style="font-size:80%;">
            <c:forEach items="${bean.priorities}" var="i">
                <c:choose>
                    <c:when test="${priority==i}">
                        <option value="${fn:escapeXml(i)}" selected="true">${fn:escapeXml(i)}</option>
                    </c:when>
                    <c:otherwise>
                        <option value="${fn:escapeXml(i)}">${fn:escapeXml(i)}</option>
                    </c:otherwise>
                </c:choose>
            </c:forEach>
            </select>
        </td>
    </tr>
</table>