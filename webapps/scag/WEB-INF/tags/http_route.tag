<%@taglib uri="http://java.sun.com/jsp/jstl/core" prefix="c"%>
<%@taglib uri="http://java.sun.com/jsp/jstl/functions" prefix="fn"%>
<%@taglib prefix="smf" uri="/scag/func"%>
<%@taglib uri="http://java.sun.com/jsp/jstl/fmt" prefix="fmt"%>
<%@tag body-content="empty" %>
<%@attribute name="columns" required="true"%>
<%@attribute name="names" required="true"%>
<%@attribute name="widths" required="true"%>
<%@attribute name="edit" required="false"%>
<%@attribute name="child" required="false"%>
<%@attribute name="parentId" required="false"%>
<%@attribute name="subjType" required="false"%>
<%@attribute name="targetElemId" required="true"%>
<%@attribute name="ali" required="false"%>
<%@attribute name="exc" required="false"%>
<%@attribute name="defaultItemId" required="true"%>

<c:set var="columns" value="${fn:split(columns, ',')}"/>
<c:set var="names" value="${fn:split(names, ',')}"/>
<c:set var="widths" value="${fn:split(widths, ',')}"/>
<input type="hidden" id="startPosition" name="startPosition" value="${bean.startPosition}">
<input type="hidden" id="sort" name="sort" value="${bean.sort}">
<input type="hidden" id="mbEdit" name="mbEdit" value="">
<input type="hidden" id="editId" name="editId" value="">
<input type="hidden" id="childEditId" name="childEditId" value="">

<script>

function clearForm() {
    var opForm = getElementByIdUni("opForm");
    opForm.target = "_self";
    opForm.action = "<%=request.getContextPath() + (request.getServletPath().endsWith(".jsp")
                      ? request.getServletPath().substring(0, request.getServletPath().lastIndexOf('/'))
                      : request.getServletPath())%>/add.jsp";
    opForm.submit();
    return false;
}


function edit(idToEdit, child, parentId) {
    var mbEdit = document.getElementById("mbEdit");
    edit.value = idToEdit;
    var editId = document.getElementById("editId");
    editId.value = idToEdit;

    var addPath = (child == null || child == '') ? "/edit.jsp" : (child + "/edit.jsp?parentId=" + idToEdit + "&editChild=true");
    var actions = '';
    if ((parentId == undefined || parentId == '')) {
        actions = "<%=request.getContextPath() + (request.getServletPath().endsWith(".jsp")
                          ? request.getServletPath().substring(0, request.getServletPath().lastIndexOf('/'))
                          : request.getServletPath())%>" + addPath;
    } else {
        actions = "<%=request.getContextPath()%>" + child + "/edit.jsp?editId=" + idToEdit + "&parentId=" + parentId;
    }
    var opForm = getElementByIdUni("opForm");
    opForm.action = actions;
    opForm.submit();

    return false;
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
  <c:forEach var="user" items="${bean.httpRuteItems}" varStatus="status">
    <tr class='row${(status.count+1)%2}'>
      <c:forEach var="_column" items="${columns}">
        <c:set var="column" value="${fn:trim(_column)}"/>
        <c:choose>
          <c:when test="${column == 'checkbox'}">
            <td class=ico><input class=check id="${targetElemId}Check" type=checkbox name=checked value="${user['id']}" onClick="tableTag_checkChecksHttpRoute('${targetElemId}','${defaultItemId}');" <c:if test="${smf:checked(bean, user['id'])}" >checked</c:if>></td>
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
            <td <c:if test="${column!=exc}">align="${ali}"</c:if>
                <c:if test="${column=='defaultRoute'}"> id="defaultRoute_${Id}"</c:if>>
                <c:choose>
                    <c:when test="${edit == column}">
                        <a href="#" onClick="return edit('${Id}','${child}','${parentId}');">${itemValue}</a>
                    </c:when>
                    <c:when test="${smf:isBoolean(itemValue)}">
                        <c:choose>
                           <c:when test="${itemValue}">
                               <span id="${itemValue}" style="width:100%; text-align:center;"><img align="center" src="content/images/ic_checked.gif"></span>
                           </c:when>
                           <c:otherwise>
                               <span id="${itemValue}" style="width:100%; text-align:center;"><img align="center" src="content/images/ic_not_checked.gif"></span>
                           </c:otherwise>
                        </c:choose>
                    </c:when>
                    <c:otherwise>
                       ${itemValue}
                    </c:otherwise>
                </c:choose>
            </td>
          </c:when>
          <c:otherwise>
            <c:set var="itemValue" value="${empty user[column] ? '&nbsp;' : fn:escapeXml(user[column])}"/>
            <td <c:if test="${column!=exc}">align="${ali}"</c:if> >
                <c:if test="${edit == column}"><a href="#" onClick="return edit('${itemValue}','${child}','${parentId}');"></c:if>
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

<c:set var="pageCurrent" value="${(bean.startPosition-(bean.startPosition%bean.pageSize)) / bean.pageSize}"/>
<c:set var="pageFirst" value="${(pageCurrent -4) < 0 ? 0 : (pageCurrent -4)}"/>
<c:set var="pageTotal" value="${((bean.totalHttpSize-(bean.totalHttpSize%bean.pageSize)) / bean.pageSize) + ((bean.totalHttpSize % bean.pageSize) > 0 ? 1 : 0)}"/>
<c:set var="pageLast" value="${(pageCurrent +4) > (pageTotal-1) ? (pageTotal-1) : (pageCurrent +4)}"/>
<table class=navbar cellspacing=1 cellpadding=0>
<tr>
<td class=first onclick="return tableTag_navigatePage(0)" title="First page"><a href="#">&nbsp;&nbsp;&nbsp;</a></td>
<c:if test="${pageCurrent > 0}">
<td class=prev onclick="return tableTag_navigatePage(${pageCurrent-1})" title="Previous page"><a href="#">&nbsp;&nbsp;&nbsp;</a></td>
</c:if>
<c:forEach var="i"  begin="${pageFirst}" end="${pageLast < 0 ? 0 : pageLast}">
  <td class="<c:choose><c:when test="${i == pageCurrent}"
    >current</c:when><c:otherwise>page</c:otherwise></c:choose>" onclick="return tableTag_navigatePage(${i})" title="Page ${i+1}">${i+1}</td>
</c:forEach>
<c:if test="${pageCurrent+1 < pageTotal}">
<td class=next title="Next page" onclick="return tableTag_navigatePage(${pageCurrent+1})"><a href="#">&nbsp;&nbsp;&nbsp;</a></td>
</c:if>
<td class=last onclick="return tableTag_navigatePage(${pageTotal > 0 ? pageTotal-1 : 0})" title="Last page"><a href="#">&nbsp;&nbsp;&nbsp;</a></td>
<td class=total><fmt:message>common.table.total</fmt:message>:${bean.totalHttpSize} <fmt:message>common.table.items</fmt:message></td>
<td class=pageSize><select id="pageSize" name="pageSize" onchange="document.all.startPosition.value=0; opForm.submit();" >
  <option value=  "3" <c:if test="${bean.pageSize ==   3}">selected</c:if>  >3</option>
  <option value=  "5" <c:if test="${bean.pageSize ==   5}">selected</c:if>  >5</option>
  <option value= "10" <c:if test="${bean.pageSize ==  10}">selected</c:if> >10</option>
  <option value= "25" <c:if test="${bean.pageSize ==  25}">selected</c:if> >25</option>
  <option value= "50" <c:if test="${bean.pageSize ==  50}">selected</c:if> >50</option>
  <option value="100" <c:if test="${bean.pageSize == 100}">selected</c:if>>100</option>
  <option value="250" <c:if test="${bean.pageSize == 250}">selected</c:if>>250</option>
  <option value="500" <c:if test="${bean.pageSize == 500}">selected</c:if>>500</option>
</select></td>
<c:if test="${filter==true}">
    <td class=filter><img src="content/images/but_filter.gif" class=button jbuttonName="mbFilter" jbuttonValue="Filter" title="Filter" jbuttonOnclick="return filter()"></td>
</c:if>
</tr>
</table>
