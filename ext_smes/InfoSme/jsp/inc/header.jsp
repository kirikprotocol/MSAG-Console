<%@ include file="/WEB-INF/inc/sme_menu.jsp"%>
<%--script>
function setSort(sorting)
{
	if (sorting == "<%=bean.getSort()%>")
		opForm.sort.value = "-<%=bean.getSort()%>";
	else
		opForm.sort.value = sorting;

	opForm.submit();
	return false;
}
</script--%>
<%
	sme_menu_begin(out);
	sme_menu_button(out, "mbMenu",  "apply",     "Apply",     "");
	sme_menu_button(out, "mbMenu",  "options",   "Options",   "Global InfoSme options");
	sme_menu_button(out, "mbMenu",  "drivers",   "Drivers",   "DB drivers");
	sme_menu_button(out, "mbMenu",  "providers", "Providers", "Providers");
	sme_menu_button(out, "mbMenu",  "tasks",     "Tasks",     "Tasks list");
	sme_menu_button(out, "mbMenu",  "shedules",  "Shedules",  "Shedules");
	sme_menu_space(out);
	sme_menu_end(out);
%>
