<%{
int itemsTotal = bean.getTotalSizeInt(); // 31
int pageCurrent = bean.getStartPositionInt() / bean.getPageSizeInt(); // 1
int pageFirst = (pageCurrent -4) < 0 ? 0 : (pageCurrent -4); // 0
int pageTotal = (itemsTotal / bean.getPageSizeInt()) + ((itemsTotal % bean.getPageSizeInt()) > 0 ? 1 : 0); // 2
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
<table class=navbar cellspacing=1 cellpadding=0>
<tr>
<td class=first onclick="return <%=jscript%>(0)" title="First page">&nbsp;</td>
<%if(pageCurrent > 0){%>
<td class=prev onclick="return <%=jscript%>(<%=pageCurrent-1%>)" title="Previous page">&nbsp;</td>
<%}
for(int pageNum=pageFirst;pageNum<=pageLast;pageNum++)
{%>
<td class=<%=pageNum==pageCurrent ? "current":"page"%> onclick="return <%=jscript%>(<%=pageNum%>)" title="Page <%=pageNum+1%>"><%=pageNum+1%></td>
<%}%>
<%if(pageCurrent+1 < pageTotal){%>
<td class=next title="Next page" onclick="return <%=jscript%>(<%=pageCurrent+1%>)">&nbsp;</td>
<%}%>
<td class=last onclick="return <%=jscript%>(<%=pageTotal > 0 ? pageTotal-1 : 0%>)" title="Last page">&nbsp;</td>
<td class=total>total:<%=itemsTotal%> items</td>
