<script>
function clickMenu(menuSelection)
{
	document.all.jbutton.name  = "menuSelection";
  document.all.jbutton.value = menuSelection;
	opForm.submit();
	return false;
}
</script>

<table>
	<tr>
		<th>&nbsp;<a href="#" onclick='clickMenu(<%= WSmeFormBean.RESULT_VISITORS%>)'>Visitors</a>&nbsp;</th>
		<th>&nbsp;<a href="#" onclick='clickMenu(<%= WSmeFormBean.RESULT_LANGS%>)'>Languages</a>&nbsp;</th>
		<th>&nbsp;<a href="#" onclick='clickMenu(<%= WSmeFormBean.RESULT_ADS%>)'>Messages</a>&nbsp;</th>
    <th>&nbsp;<a href="#" onclick='clickMenu(<%= WSmeFormBean.RESULT_OK%>)'>History</a>&nbsp;</th>
    <th>&nbsp;<a href="#" onclick='clickMenu(<%= WSmeFormBean.RESULT_DONE%>)'>General settings</a>&nbsp;</th>
	</tr>
</table>
