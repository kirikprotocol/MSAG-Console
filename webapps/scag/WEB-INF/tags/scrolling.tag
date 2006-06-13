<%@ taglib uri="http://java.sun.com/jsp/jstl/core" prefix="c" %>
<%@ taglib uri="http://java.sun.com/jsp/jstl/functions" prefix="fn" %>
<%@ taglib uri="http://java.sun.com/jsp/jstl/fmt" prefix="fmt" %>
<%@ taglib prefix="smf" uri="/scag/func" %>
<%@ tag body-content="empty" %>
<%@ attribute name="columns" required="true" %>
<%@ attribute name="names" required="true" %>
<%@ attribute name="widths" required="false" %>
<%@ attribute name="filter" required="false" %>
<c:set var="columns" value="${fn:split(columns, ',')}"/>
<c:set var="names" value="${fn:split(names, ',')}"/>
<c:set var="widths" value="${fn:split(widths, ',')}"/>
<input type="hidden" id="sort" name="sort" value="${bean.sort}">

<script>
    function tableTag_sort(sort) {

        if (document.all.sort.value == sort)
            opForm.sort.value = "-" + sort;
        else
            opForm.sort.value = sort;
        opForm.submit();
        return false;
    }

</script>
<table class=list cellspacing=0>
    <thead>
        <tr>
            <c:forEach var="_column" items="${columns}" varStatus="status">
                <c:set var="column" value="${fn:trim(_column)}"/>
                <th align="left" width="${widths[status.count-1]}%">
                    <a href="#" onclick="return tableTag_sort('${column}')"
                        <c:if test="${fn:endsWith(bean.sort, column)}">
                            class="${fn:startsWith(bean.sort, '-') ? 'up' : 'down'}"</c:if>><fmt:message>${names[status.count-1]}</fmt:message>
                    </a>
               </th>
            </c:forEach>
        </tr>
    </thead>
</table>
<div id="dd">
    <table class=list cellspacing=0>
        <c:forEach var="_column" items="${columns}" varStatus="status">
                <c:set var="column" value="${fn:trim(_column)}"/>
                <th width="${widths[status.count-1]}%"></th>
            </c:forEach>
        <c:forEach var="user" items="${bean.tabledItems}" varStatus="status">
            <tr class='row${(status.count+1)%2}'>
                <c:forEach var="_column" items="${columns}">
                    <c:set var="column" value="${fn:trim(_column)}"/>
                    <c:set var="itemValue" value="${empty user[column] ? '&nbsp;' : fn:escapeXml(user[column])}"/>
                    <td align="center" class=ico>${itemValue}</td>
                </c:forEach>
            </tr>
        </c:forEach>
    </table>
</div>
