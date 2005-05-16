<%@taglib uri="http://java.sun.com/jsp/jstl/core" prefix="c"%><%@
 taglib uri="http://java.sun.com/jsp/jstl/functions" prefix="fn"%><%@
 taglib prefix="smf" uri="/smppgw/func"%><%@
 tag body-content="empty" %><%@
 attribute name="columns" required="true"%><%@
 attribute name="names" required="true"%><%@
 attribute name="widths" required="true"%><%@
 attribute name="edit" required="false"%>
<c:set var="columns" value="${fn:split(columns, ',')}"/>
<c:set var="names" value="${fn:split(names, ',')}"/>
<c:set var="widths" value="${fn:split(widths, ',')}"/>
<input type="hidden" id="startPosition" name="startPosition" value="${bean.startPosition}">
<input type="hidden" id="sort" name="sort" value="${bean.sort}">
<input type="hidden" id="mbEdit" name="mbEdit" value="">
<input type="hidden" id="editId" name="editId" value="">

<script>
function tableTag_navigatePage(pageNum)
{
	opForm.startPosition.value = pageNum*${bean.pageSize};
	opForm.submit();
	return false;
}
function tableTag_sort(sort)
{
  if (document.all.sort.value == sort)
    opForm.sort.value = "-" + sort;
  else
    opForm.sort.value = sort;
	opForm.submit();
	return false;
}
function tableTag_checkChecks()
{
  var buttonsEnabled;
  buttonsEnabled = false;
  for (i=0; i<opForm.elements.length; i++)
  {
    var elem = opForm.elements[i];
    if (elem.tagName == "INPUT" && elem.className == "check" && elem.checked)
      buttonsEnabled = true;
  }

  for (i=0; i<document.all.length; i++)
  {
    var elem = document.all[i];
    if (elem.tagName == "A" && elem.isCheckSensible)
      elem.disabled = !buttonsEnabled;
  }
}
function edit(idToEdit)
{
  opForm.mbEdit.value = idToEdit;
  opForm.editId.value = idToEdit;
  opForm.action = "<%=request.getContextPath() + (request.getServletPath().endsWith(".jsp")
                                                  ? request.getServletPath().substring(0, request.getServletPath().lastIndexOf('/'))
                                                  : request.getServletPath())%>/edit.jsp";
  opForm.submit();
  return false;
}
</script>
<table class=list cellspacing=0>
<thead>
  <tr>
    <c:forEach var="column" items="${columns}" varStatus="status">
      <c:choose>
        <c:when test="${column == 'checkbox'}">
          <th class=ico width="${widths[status.count-1]}%"><img src="/images/ico16_checked_sa.gif" class=ico16 alt=""></th>
        </c:when>
        <c:otherwise>
          <th width="${widths[status.count-1]}%"><a href="#" onclick="return tableTag_sort('${column}')"
            <c:if test="${fn:endsWith(bean.sort, column)}">class="${fn:startsWith(bean.sort, '-') ? 'up' : 'down'}"</c:if>
          >${names[status.count-1]}</a></th>
        </c:otherwise>
      </c:choose>
    </c:forEach>
  </tr>
</thead>

<tbody>
  <c:forEach var="user" items="${bean.tabledItems}" varStatus="status">
    <tr class='row${(status.count+1)%2}'>
      <c:forEach var="column" items="${columns}">
        <c:choose>
          <c:when test="${column == 'checkbox'}">
            <td class=ico><input class=check type=checkbox name=checked value="${user['id']}" onClick="tableTag_checkChecks();" <c:if test="${smf:checked(bean, user['id'])}" >checked</c:if>></td>
          </c:when>
          <c:when test="${column == 'status'}">
            <td class=ico>  <c:set var="Id" value="${ user['id']}"/>
           <c:set var="itemValue" value="${empty user[column] ? '&nbsp;' :  smf:smeStatus(user[column],Id)}"/>
           ${itemValue}
           </td>
          </c:when>
          <c:otherwise>
            <c:set var="itemValue" value="${empty user[column] ? '&nbsp;' : fn:escapeXml(user[column])}"/>
            <td><c:if
                test="${edit == column}"><a href="#" onClick="return edit('${itemValue}');"></c:if
              ><c:choose
                ><c:when test="${smf:isBoolean(user[column])}"><c:choose><c:when test="${itemValue}"><span style="width:100%; text-align:center;"><img align="center" src="/images/ic_checked.gif"></span></c:when><c:otherwise>&nbsp;</c:otherwise></c:choose></c:when
                ><c:otherwise>${itemValue}</c:otherwise>
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
<c:set var="pageTotal" value="${((bean.totalSize-(bean.totalSize%bean.pageSize)) / bean.pageSize) + ((bean.totalSize % bean.pageSize) > 0 ? 1 : 0)}"/>
<c:set var="pageLast" value="${(pageCurrent +4) > (pageTotal-1) ? (pageTotal-1) : (pageCurrent +4)}"/>
<table class=navbar cellspacing=1 cellpadding=0>
<tr>
<td class=first onclick="return tableTag_navigatePage(0)" title="First page"><a href="#">&nbsp;</a></td>
<c:if test="${pageCurrent > 0}">
<td class=prev onclick="return tableTag_navigatePage(${pageCurrent-1})" title="Previous page"><a href="#">&nbsp;</a></td>
</c:if>
<c:forEach var="i"  begin="${pageFirst}" end="${pageLast < 0 ? 0 : pageLast}">
  <td class="<c:choose><c:when test="${i == pageCurrent}"
    >current</c:when><c:otherwise>page</c:otherwise></c:choose>" onclick="return tableTag_navigatePage(${i})" title="Page ${i+1}">${i+1}</td>
</c:forEach>
<c:if test="${pageCurrent+1 < pageTotal}">
<td class=next title="Next page" onclick="return tableTag_navigatePage(${pageCurrent+1})"><a href="#">&nbsp;</a></td>
</c:if>
<td class=last onclick="return tableTag_navigatePage(${pageTotal > 0 ? pageTotal-1 : 0})" title="Last page"><a href="#">&nbsp;</a></td>
<td class=total>total:${bean.totalSize} items</td>
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
<%--td class=filter><%button(out, "but_filter.gif", "mbFilter", "Filter", "Filter", "return clickFilter()");%></td--%>
</tr>
</table>
<script>tableTag_checkChecks();</script>