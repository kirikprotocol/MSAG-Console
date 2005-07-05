<%@taglib uri="http://java.sun.com/jsp/jstl/core" prefix="c"%><%@
 tag body-content="empty"%><%@
 attribute name="name" required="true"%><%@
 attribute name="value" required="true" %><%@
 attribute name="title" required="true"%><%@
 attribute name="enabled" required="false"%><%@
 attribute name="onclick" required="false"%><%@
 attribute name="isCheckSensible" required="false"%>

<%!
 boolean isNotNull = false;
%>

<%
    if(session.getAttribute("BROWSER_TYPE") != null){
        isNotNull = true;
    }
%>
<% if(isNotNull){
    if(session.getAttribute("BROWSER_TYPE").equals("IE")){
%>

<c:if test="${page_menu_delimiter_needed}"><td width=1px>|</td></c:if>
    <td width=1px>
        <a id="${name}" jbuttonName="${name}" jbuttonValue="${value}" title="${title}"
            <c:if test="${!empty enabled && !enabled}">disabled</c:if>
            <c:if test="${!empty onclick}">jbuttonOnclick="${onclick}"</c:if>
            <c:if test="${isCheckSensible}">isCheckSensible="${isCheckSensible}"</c:if>>
        ${value}
        </a>
    </td>
<c:set var="page_menu_delimiter_needed" value="true" scope="request"></c:set>
<%
}else{ %>

        <c:if test="${page_menu_delimiter_needed}"><td width=1px>|</td></c:if>
            <td width=1px>
                <input type=submit id="${name}" name="${name}" value="${value}" title="${title}"
                    <c:if test="${!empty enabled && !enabled}">disabled</c:if>
                    <c:if test="${!empty onclick}">jbuttonOnclick="${onclick}"</c:if>
                    <c:if test="${isCheckSensible}">isCheckSensible="${isCheckSensible}"</c:if>>
                </input>
            </td>
        <c:set var="page_menu_delimiter_needed" value="true" scope="request"></c:set>
<%
    }
}
else{
    %>
    <c:if test="${page_menu_delimiter_needed}"><td width=1px>|</td></c:if>
            <td width=1px>
                <input type="submit" id="${name}" name="${name}" value="${value}" title="${title}"
                    <c:if test="${!empty enabled && !enabled}">disabled</c:if>
                    <c:if test="${!empty onclick}">jbuttonOnclick="${onclick}"</c:if>
                    <c:if test="${isCheckSensible}">isCheckSensible="${isCheckSensible}"</c:if>>
                </input>
            </td>
        <c:set var="page_menu_delimiter_needed" value="true" scope="request"></c:set>
    <%
}
%>