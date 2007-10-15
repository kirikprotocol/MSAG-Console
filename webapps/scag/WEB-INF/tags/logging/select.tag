<%@ taglib uri="http://java.sun.com/jsp/jstl/core" prefix="c"%>
<%@ taglib uri="http://java.sun.com/jsp/jstl/functions" prefix="fn"%>
<%@ attribute name="name" required="true"%>
<%@ attribute name="fullName" required="true"%>
<%@ attribute name="priority" required="true"%>
<table class="properties_list" cellspacing="0" border="0">
    <col width="150px">
    <tr class=row${rowNum%2}>
        <c:if test="${name!='_empty_name_'}">
            <th nowrap>${name}</th>
        </c:if>
        <td>
            <select name="category_${fullName}" id="category_${fullName}" class="txt" style="font-size:80%;">
            <c:set var="selected" value="0"></c:set>
            <c:forEach items="${bean.priorities}" var="i">
                <c:choose>
                    <c:when test="${priority==i}">
                        <option value="${fn:escapeXml(i)}" selected="true">${fn:escapeXml(i)}</option>
                        <c:set var="selected" value="1"></c:set>
                    </c:when>
                    <c:otherwise>
                        <option value="${fn:escapeXml(i)}" <c:if test="${selected!=1}">selected="true"</c:if> >${fn:escapeXml(i)}
                        </option>
                    </c:otherwise>
                </c:choose>
            </c:forEach>
            </select>
        </td>
    </tr>
</table>