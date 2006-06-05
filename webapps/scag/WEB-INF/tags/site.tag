<%@taglib prefix="c" uri="http://java.sun.com/jsp/jstl/core" %>
<%@taglib prefix="fn" uri="http://java.sun.com/jsp/jstl/functions"%>
<%@taglib prefix="smf" uri="/scag/func"%>
<%@tag body-content="empty"%>
<%@attribute name="columns" required="true"%>
<%@attribute name="names" required="true"%>
<%@attribute name="widths" required="true"%>
<%@attribute name="edit" required="false"%>
<c:set var="columns" value="${fn:split(columns,',')}"/>
<c:set var="names" value="${fn:split(names,',')}"/>
<c:set var="widths" value="${fn:split(widths,',')}"/>
<c:set var="tblItems" value="${bean.tabledItems}"/>
<c:set var="siteItems" value="${bean.siteItems}"/>

<input type="hidden" id="startPosition" name="startPosition" value="${bean.startPosition}">
<input type="hidden" id="sort" name="sort" value="${bean.sort}">
<input type="hidden" id="mbHttpSiteEdit" name="mbHttpSiteEdit" value="">
<input type="hidden" id="httpSiteId" name="httpSiteId" value="">
<input type="hidden" id="subjectType" name="subjectType" value="">


<script type="text/javascript">
    function editSite(idToEdit) {
        opForm.mbHttpSiteEdit.value = idToEdit;
        opForm.httpSiteId.value = idToEdit;
        opForm.subjectType.value = '1';
        opForm.action = "<%=request.getContextPath() + (request.getServletPath().endsWith(".jsp")
                         ? request.getServletPath().substring(0, request.getServletPath().lastIndexOf('/'))
                         : request.getServletPath())%>/edit.jsp?subjectType=1";
        opForm.submit();
        return false;
    }
</script>

<table class="list" cellspacing="0">
    <thead>
        <tr>
            <c:forEach var="_column" items="${columns}" varStatus="status">
                <c:set var="column" value="${fn:trim(_column)}"/>
                <c:choose>
                    <c:when test="${column == 'checkbox'}">
                        <th class="ico" width="${widths[count.count-1]}%">
                           <img src="content/images/ico16_checked_sa.gif" alt="" class="ico16">
                        </th>
                    </c:when>
                    <c:otherwise>
                        <th width="${widths[status.count-1]}%">
                            <a href="#" onclick="return tableTag_sort('${column}')"
                                <c:if test="${fn:endsWith(bean.sort,column)}">
                                    class="${fn:startsWith(bean.sort,'-') ? 'up' : 'down'}"
                                </c:if>>${names[status.count-1]}</a>
                        </th>
                    </c:otherwise>
                </c:choose>
            </c:forEach>
        </tr>
    </thead>

    <tbody>
       <c:forEach var="user" items="${siteItems}" varStatus="status">
           <tr class="row${(status.count+1)%2}">
               <c:forEach var="_column" items="${columns}">
                   <c:set var="column"  value="${fn:trim(_column)}"/>
                   <c:choose>
                       <c:when test="${column == 'checkbox'}">
                           <td class="ico">
                               <input class="check" type="checkbox" name="checked" value="${user['id']}"
                                   onclick="tableTag_checkChecks();" <c:if test="${smf:checked(bean,user['id'])}">checked</c:if>
                           </td>
                       </c:when>
                       <c:when test="${column == 'status'}">
                           <td class="ico">
                               <c:set var="Id" value="${user['id']}"/>
                               <c:set var="itemValue" value="${empty user[column] ? '&nbsp;' : smf:smeStatus(user[column],Id)}"/>
                               ${itemValue}
                           </td>
                       </c:when>
                       <c:when test="${fn:substringBefore('name',column) == fn:substringBefore('name',column)}">
                           <c:set var="Id" value="${user['id']}"/>
                           <c:set var="itemValue" value="${empty user[column] ? '&nbsp;' : fn:escapeXml(user[column])}"/>
                           <td>
                               <c:choose>
                                   <c:when test="${edit == column}">
                                       <a href="#" onclick="return editSite('${Id}');">${itemValue}</a>
                                   </c:when>
                                   <c:otherwise>
                                       ${itemValue}
                                   </c:otherwise>
                               </c:choose>
                           </td>
                       </c:when>
                       <c:otherwise>
                           <c:set var="itemValue"
                                  value="${empty user[column] ? '&nbsp;' : fn:escapeXml(user[column])}"/>
                           <td>
                               <c:if test="${edit == column}">
                                   <a href="#" onClick="return editSite('${itemValue}');"></c:if>
                               <c:choose>
                                   <c:when test="${smf:isBoolean(user[column])}">
                                       <c:choose>
                                           <c:when test="${itemValue}">
                                               <span style="width:100%; text-align:center;">
                                                   <img align="center" src="content/images/ic_checked.gif"></span>

                                                   </c:when>
                                           <c:otherwise>&nbsp;</c:otherwise>
                                       </c:choose>
                                   </c:when>
                                   <c:otherwise>${itemValue}</c:otherwise>
                               </c:choose><c:if test="${edit == column}"></a></c:if></td>
                       </c:otherwise>
                   </c:choose>
               </c:forEach>
           </tr>
       </c:forEach>
    </tbody>
</table>
<%@include file="paging.tag"%>

