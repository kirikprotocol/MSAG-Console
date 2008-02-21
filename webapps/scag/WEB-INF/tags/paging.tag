<%@taglib uri="http://java.sun.com/jsp/jstl/core" prefix="c"%>
<%@taglib uri="http://java.sun.com/jsp/jstl/functions" prefix="fn"%>
<%@taglib uri="http://java.sun.com/jsp/jstl/fmt" prefix="fmt"%>
<%@taglib prefix="smf" uri="/scag/func"%>

<%@attribute name="filter" required="false"%>

<c:set var="pageCurrent" value="${(bean.startPosition-(bean.startPosition%bean.pageSize)) / bean.pageSize}"/>
<c:set var="pageFirst" value="${(pageCurrent -4) < 0 ? 0 : (pageCurrent -4)}"/>
<c:set var="pageTotal" value="${((bean.totalSize-(bean.totalSize%bean.pageSize)) / bean.pageSize) + ((bean.totalSize % bean.pageSize) > 0 ? 1 : 0)}"/>
<c:set var="pageLast" value="${(pageCurrent +4) > (pageTotal-1) ? (pageTotal-1) : (pageCurrent +4)}"/>
<script>
   function tableTag_navigatePage(pageNum) {
//       opForm.startPosition.value = pageNum * ${bean.pageSize};
        elem = document.getElementById("startPosition");
        elem.value = pageNum * ${bean.pageSize};
//       alert("pageNum='" + pageNum + "' startPosition='" + elem.value + "'");
       getElementByIdUni("opForm").submit();
       return false;
   }
</script>
<table class=navbar cellspacing=1 cellpadding=0>
    <tr>
        <td class=first onclick="return tableTag_navigatePage(0)" title="First page"><a href="#">&nbsp;</a></td>
        <c:if test="${pageCurrent > 0}">
        <td class=prev onclick="return tableTag_navigatePage(${pageCurrent-1})" title="Previous page"><a href="#">&nbsp;</a></td>
        </c:if>
        <c:forEach var="i"  begin="${pageFirst}" end="${pageLast < 0 ? 0 : pageLast}">
          <td class="<c:choose>
                        <c:when test="${i == pageCurrent}">current</c:when>
                        <c:otherwise>page</c:otherwise>
                     </c:choose>"
             onclick="return tableTag_navigatePage(${i})" title="Page ${i+1}">
             ${i+1}
          </td>
        </c:forEach>
        <c:if test="${pageCurrent+1 < pageTotal}">
        <td class=next title="Next page" onclick="return tableTag_navigatePage(${pageCurrent+1})"><a href="#">&nbsp;</a></td>
        </c:if>
        <td class=last onclick="return tableTag_navigatePage(${pageTotal > 0 ? pageTotal-1 : 0})" title="Last page"><a href="#">&nbsp;</a></td>
        <td class=total><fmt:message>common.table.total</fmt:message>:${bean.totalSize} <fmt:message>common.table.items</fmt:message></td>
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
        <%--td class=filter><%button(out, "but_filter.gif", "mbFilter", "Filter", "Filter", "return clickFilter()");%></td--%>
    </tr>
</table>
<!--<script>tableTag_checkChecks();</script>-->
