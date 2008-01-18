<%@taglib uri="http://java.sun.com/jsp/jstl/core" prefix="c"%><%@
        taglib uri="http://java.sun.com/jsp/jstl/functions" prefix="fn"%><%@
        taglib uri="http://java.sun.com/jsp/jstl/fmt" prefix="fmt"%><%@
        taglib prefix="smf" uri="/scag/func"%><%@
        tag body-content="empty" %><%@
        attribute name="columns" required="true"%><%@
        attribute name="names" required="true"%><%@
        attribute name="widths" required="true"%><%@
        attribute name="edit" required="false"%><%@
        attribute name="child" required="false"%><%@
        attribute name="parentId" required="false"%><%@
        attribute name="subjType" required="false"%><%@
        attribute name="targetElemId" required="false"%>
<c:set var="columns" value="${fn:split(columns, ',')}"/>
<c:set var="names" value="${fn:split(names, ',')}"/>
<c:set var="widths" value="${fn:split(widths, ',')}"/>
<input type="hidden" id="startPosition" name="startPosition" value="${bean.startPosition}">
<input type="hidden" id="sort" name="sort" value="${bean.sort}">
<input type="hidden" id="mbEdit" name="mbEdit" value="">
<input type="hidden" id="editId" name="editId" value="">
<input type="hidden" id="childEditId" name="childEditId" value="">

<%--<OBJECT id="tdcSmscStatuses" CLASSID="clsid:333C7BC4-460F-11D0-BC04-0080C7055A83">--%>
<%--    <PARAM NAME="DataURL" VALUE="endpoints/centers/center_statuses.jsp">--%>
<%--    <PARAM NAME="UseHeader" VALUE="True">--%>
<%--    <PARAM NAME="TextQualifier" VALUE='"'>--%>
<%--</OBJECT>--%>
<script >
    var centerStatus =
    new StringTableDataSource({url: '<%=request.getContextPath()%>/endpoints/centers/center_statuses.jsp'});
</script>
<script>
    function edit(idToEdit, child, parentId) {
    var mbEdit = getElementByIdUni("mbEdit");
    edit.value = idToEdit;
    var editId = getElementByIdUni("editId");
    editId.value = idToEdit;
    var opForm = getElementByIdUni("opForm");
        opForm.action = "<%=request.getContextPath() + (request.getServletPath().endsWith(".jsp")
                          ? request.getServletPath().substring(0, request.getServletPath().lastIndexOf('/'))
                          : request.getServletPath())%>" + "/edit.jsp";
        opForm.submit();

        return false;
    }

    function refreshStatus() {
//        document.getElementById('tdcSmscStatuses').DataURL = document.getElementById('tdcSmscStatuses').DataURL;
//        document.getElementById('tdcSmscStatuses').reset();
        centerStatus.update();
        window.setTimeout(refreshStatus, 10000);
    }
</script>

<table class=list cellspacing=0>
<thead>
  <tr>
    <c:forEach var="_column" items="${columns}" varStatus="status">
      <c:set var="column" value="${fn:trim(_column)}"/>
      <c:choose>
        <c:when test="${column == 'checkbox'}">
          <th class=ico width="${widths[status.count-1]}%"><img src="content/images/ico16_checked_sa.gif" class=ico16 alt=""></th>
        </c:when>
        <c:otherwise>
          <th width="${widths[status.count-1]}%"><a href="#" onclick="return tableTag_sort('${column}')"
            <c:if test="${fn:endsWith(bean.sort, column)}">class="${fn:startsWith(bean.sort, '-') ? 'up' : 'down'}"</c:if>
          ><fmt:message>${names[status.count-1]}</fmt:message></a></th>
        </c:otherwise>
      </c:choose>
    </c:forEach>
  </tr>
</thead>

<tbody>
  <c:forEach var="user" items="${bean.tabledItems}" varStatus="status">
    <tr class='row${(status.count+1)%2}'>
      <c:forEach var="_column" items="${columns}">
        <c:set var="column" value="${fn:trim(_column)}"/>
        <c:choose>
          <c:when test="${column == 'checkbox'}">
            <td class=ico><input class=check <c:if test="${!empty targetElemId}">id="${targetElemId}Check"</c:if> type=checkbox name=checked value="${user['id']}" onClick="tableTag_checkChecks(<c:if test="${!empty targetElemId}">'${targetElemId}'</c:if>);" <c:if test="${smf:checked(bean, user['id'])}" >checked</c:if>></td>
          </c:when>
          <c:when test="${column == 'status'}">
            <td class=ico>  <c:set var="Id" value="${ user['id']}"/>
           <c:set var="itemValue" value="${empty user[column] ? '&nbsp;' :  smf:smeStatus(user[column],Id)}"/>
           ${itemValue}
           </td>
          </c:when>
          <c:when test="${fn:substringBefore('name',column) == fn:substringBefore('name',column)}">
            <c:set var="Id" value="${ user['id']}"/>
            <c:set var="itemValue" value="${empty user[column] ? '&nbsp;' : fn:escapeXml(user[column])}"/>
            <td  <c:if test="${column == 'connStatus'}">align="center"</c:if> nowrap>
                <c:choose>
                    <c:when test="${edit == column}">
                        <a href="#" onClick="return edit('${Id}','${child}','${parentId}');">${itemValue}</a>
                    </c:when>
                    <c:when test="${smf:isBoolean(itemValue)}">
                        <c:choose>
                           <c:when test="${itemValue}">
                               <span style="width:100%; text-align:center;"><img align="center" src="content/images/ic_checked.gif"></span>
                           </c:when>
                           <c:otherwise>
                               <span style="width:100%; text-align:center;"><img align="center" src="content/images/ic_not_checked.gif"></span>
                           </c:otherwise>
                        </c:choose>
                    </c:when>
                    <c:when test="${column == 'connHostPort'}">
                        <span id="CONNECTION_STATUSCENTER_${Id}" class=C080>&nbsp;</span>
      <script>
      centerStatus.addObserver(new ElementObserver({elementId: 'CONNECTION_STATUSCENTER_${Id}', field: '"${Id}"' }));
      </script>
                    </c:when>
                    <c:when test="${column == 'connStatus'}">
                        <span id="CONNECTION_STATUSCENTER_${Id}_S" class=C080>&nbsp;</span>
      <script>
      centerStatus.addObserver(new ElementObserver({elementId: 'CONNECTION_STATUSCENTER_${Id}_S', field: '"${Id}_s"' }));
      </script>
                    </c:when>
                    <c:otherwise>
                        ${itemValue}
                    </c:otherwise>
                </c:choose>
            </td>
          </c:when>
          <c:otherwise>
            <c:set var="itemValue" value="${empty user[column] ? '&nbsp;' : fn:escapeXml(user[column])}"/>
            <td><c:if test="${edit == column}"><a href="#" onClick="return edit('${itemValue}','${child}','${parentId}');"></c:if>
              <c:choose >
                <c:when test="${smf:isBoolean(user[column])}">
                  <c:choose>
                    <c:when test="${itemValue}"><span style="width:100%; text-align:center;"><img align="center" src="content/images/ic_checked.gif"></span></c:when>
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
<%@ include file="paging.tag" %>
