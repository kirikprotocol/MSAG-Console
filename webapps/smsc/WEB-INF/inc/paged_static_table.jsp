<%if (tableHelper instanceof ru.novosoft.smsc.jsp.util.helper.statictable.PagedStaticTableHelper) {
    int itemsTotal = tableHelper.getTotalSize(); // 31
    int pageCurrent = tableHelper.getStartPosition() / tableHelper.getPageSize(); // 1
    int pageFirst = (pageCurrent - 4) < 0 ? 0 : (pageCurrent - 4); // 0
    int pageTotal = (itemsTotal / tableHelper.getPageSize()) + ((itemsTotal % tableHelper.getPageSize()) > 0 ? 1 : 0); // 2
    int pageLast = (pageCurrent + 4) > (pageTotal - 1) ? (pageTotal - 1) : (pageCurrent + 4);
    String startPositionPrefix = tableHelper.getStartPositionPrefix();
    int pageSize = tableHelper.getPageSize();
    if (request.getAttribute("paged_table_js_included") == null) {%>
<script src="/scripts/paged_static_table.js" type="text/javascript"></script>
<%    request.setAttribute("paged_table_js_included", new Object()); }%>
<input type=hidden name="<%=startPositionPrefix%>" id="<%=startPositionPrefix%>" value="<%=tableHelper.getStartPosition()%>">
<%@ include file="/WEB-INF/inc/static_table.jsp"%>
<table class=navbar cellspacing=1 cellpadding=0>
  <tr>
    <td class=first><a <%=makeHref("navigatePage('" + startPositionPrefix + "'," + pageSize +  ", 0)")%>><img src="/images/nav_first.gif" width="12" height="11" alt="<%=getLocString("navbar.firstPage")%>"></a></td>
    <%if(pageCurrent > 0){%>
    <td class=prev><a <%=makeHref("navigatePage('" + startPositionPrefix + "'," + pageSize + "," + (pageCurrent - 1) + ")")%>><img src="/images/nav_prev.gif" width="12" height="11" alt="<%=getLocString("navbar.prevPage")%>"></a></td>
    <%}
  for(int pageNum=pageFirst;pageNum<=pageLast;pageNum++) {%>
    <td class="<%=pageNum==pageCurrent ? "current":"page"%>"><a <%=makeHref("navigatePage('" + startPositionPrefix + "'," + pageSize + "," + pageNum + ")")%> title="<%=getLocString("navbar.pagePre") + (pageNum + 1)%>"><%=pageNum + 1%></a></td>
  <%}%>
  <%if(pageCurrent+1 < pageTotal){%>
    <td class=next><a <%=makeHref("navigatePage('" + startPositionPrefix + "'," + pageSize + "," + (pageCurrent + 1) + ")")%>><img src="/images/nav_next.gif" width="12" height="11" alt="<%=getLocString("navbar.nextPage")%>"></a></td>
  <%}%>
    <td class=last><a <%=makeHref("navigatePage('" + startPositionPrefix + "'," + pageSize + "," + (pageTotal > 0 ? pageTotal - 1 : 0) + ")")%>><img src="/images/nav_last.gif" width="12" height="11" alt="<%=getLocString("navbar.lastPage")%>"></a></td>
    <td class=total><%= getLocString("common.sortmodes.total") + ": " + itemsTotal + " " + getLocString("common.util.items")%></td>
  <% if (tableHelper.isFilterEnabled()) { %>
    <td class=filter><%button(out, getLocImageFileName("but_filter"), tableHelper.getMbFilterPrefix(), "Filter", "common.hints.filter", "return clickFilter()");%></td>
  <%}%>
  </tr>
</table>
<%} else {%>
Invalid tableHelper type!!! tableHelper must extends PagedTableHelper
<%} %>