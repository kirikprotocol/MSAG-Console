<%@ page import="ru.novosoft.smsc.wsme.beans.WSmeFormBean"%><script>
function clickMenu(menuSelection)
{
    document.all.jbutton.name  = "menuSelection";
    document.all.jbutton.value = menuSelection;
    opForm.submit();
    return false;
}
function setSort(sorting)
{
	if (sorting == "<%=bean.getSort()%>")
		opForm.sort.value = "-<%=bean.getSort()%>";
	else
		opForm.sort.value = sorting;

	opForm.submit();
	return false;
}
</script>

<table>
	<tr>
	<th>&nbsp;<a href="#" onclick='return clickMenu(<%= WSmeFormBean.RESULT_DONE%>)' <%=
      ((bean.getMenuId() == WSmeFormBean.RESULT_DONE) ? "disabled":"")%>>Configuration</a>&nbsp;</th>
	<th>&nbsp;<a href="#" onclick='return clickMenu(<%= WSmeFormBean.RESULT_VISITORS%>)' <%=
      ((bean.getMenuId() == WSmeFormBean.RESULT_VISITORS) ? "disabled":"")%>>Visitors</a>&nbsp;</th>
	<th>&nbsp;<a href="#" onclick='return clickMenu(<%= WSmeFormBean.RESULT_LANGS%>)' <%=
      ((bean.getMenuId() == WSmeFormBean.RESULT_LANGS) ? "disabled":"")%>>Languages</a>&nbsp;</th>
	<th>&nbsp;<a href="#" onclick='return clickMenu(<%= WSmeFormBean.RESULT_ADS%>)' <%=
      ((bean.getMenuId() == WSmeFormBean.RESULT_ADS) ? "disabled":"")%>>Messages</a>&nbsp;</th>
	</tr>
</table>
