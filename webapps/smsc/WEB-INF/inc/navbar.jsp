<%{
int itemsTotal = bean.getTotalSize(); // 31
int pageCurrent = bean.getStartPosition() / bean.getPageSize(); // 1
int pageFirst = (pageCurrent -4) < 0 ? 0 : (pageCurrent -4); // 0
int pageTotal = (itemsTotal / bean.getPageSize()) + ((itemsTotal % bean.getPageSize()) > 0 ? 1 : 0); // 2
int pageLast = (pageCurrent +4) > (pageTotal-1) ? (pageTotal-1) : (pageCurrent +4);
String jscript = "navigatePage";
%>
<script>
function navigatePage(pageNum)
{
	document.all.startPosition.value = pageNum*<%=bean.getPageSize()%>;
	opForm.submit();
	return false;
}
function clickFilter()
{
	document.all.jbutton.name = "mbFilter";
	opForm.submit();
	return false;
}
</script>
<table class=secNav cellspacing=1 cellpadding=0 width="100%">
<tr>
<td class=navb><a href="#" onclick="return <%=jscript%>(0)">first</a></td>
<%if(pageCurrent > 0){%>
<td class=navb><a href="#" onclick="return <%=jscript%>(<%=pageCurrent-1%>)"><img src="<%=CPATH%>/img/ico12_shift_ld.gif" class=ico12 alt="Previous page"></a></td>
<%}
for(int pageNum=pageFirst;pageNum<=pageLast;pageNum++)
{%>
<td class=<%=pageNum==pageCurrent ? "navs":"navb"%>><a href="#" onclick="return <%=jscript%>(<%=pageNum%>)"><%=pageNum+1%></a></td>
<%}%>
<%if(pageCurrent+1 < pageTotal){%>
<td class=navb><a href="#" onclick="return <%=jscript%>(<%=pageCurrent+1%>)"><img src="<%=CPATH%>/img/ico12_shift_rd.gif" class=ico12 alt="Next page"></a></td>
<%}%>
<td class=navb><a href="#" onclick="return <%=jscript%>(<%=pageTotal-1%>)">last</a></td>
<td class=navi>total:<%=itemsTotal%> items</td>
<td class=navb><a href="#" onclick="return clickFilter()">filter</a></td>
</tr>
</table>
<%}%>