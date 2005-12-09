<%@ page import="ru.novosoft.smsc.wsme.beans.WSmeFormBean"%><script>
function clickMenu(menuSelection)
{
    document.getElementById('jbutton').name  = "menuSelection";
    document.getElementById('jbutton').value = menuSelection;
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
<div class=content>
<table>
	<tr>
	<th>&nbsp;<a href="#" onclick='return clickMenu(<%= WSmeFormBean.RESULT_HOME%>)' <%=
      ((bean.getMenuId() == WSmeFormBean.RESULT_HOME) ? "disabled":"")%>>Configuration</a>&nbsp;</th>
	<th>&nbsp;<a href="#" onclick='return clickMenu(<%= WSmeFormBean.RESULT_VISITORS%>)' <%=
      ((bean.getMenuId() == WSmeFormBean.RESULT_VISITORS) ? "disabled":"")%>>Visitors</a>&nbsp;</th>
	<th>&nbsp;<a href="#" onclick='return clickMenu(<%= WSmeFormBean.RESULT_LANGS%>)' <%=
      ((bean.getMenuId() == WSmeFormBean.RESULT_LANGS) ? "disabled":"")%>>Languages</a>&nbsp;</th>
	<th>&nbsp;<a href="#" onclick='return clickMenu(<%= WSmeFormBean.RESULT_ADS%>)' <%=
      ((bean.getMenuId() == WSmeFormBean.RESULT_ADS) ? "disabled":"")%>>Messages</a>&nbsp;</th>
	</tr>
</table>
</div>