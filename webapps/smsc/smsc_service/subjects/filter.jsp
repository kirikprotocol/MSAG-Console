<%@ page import = "ru.novosoft.smsc.jsp.util.tables.impl.SubjectFilter"%>
<%@ include file="/common/header.jsp"%>
<%@ include file="/smsc_service/menu.jsp" %>
<%@ include file="/common/list_selector.jsp"%>
<form method="post" action="filter_2.jsp">
<%
SubjectFilter filter = (SubjectFilter) session.getAttribute("subject_filter");
if (filter == null)
  filter = new SubjectFilter();

Set selected_names = filter.getNames();
Set available_names = new HashSet();
available_names.addAll(smsc.getSubjects().getNames());
available_names.removeAll(selected_names);

String masks = maskStrings2String(filter.getMaskStrings());
List   smes  = filter.getSmeIds();

Integer pagesizeI = (Integer) session.getAttribute("subject_page_size");
int pagesize = pagesizeI == null ? 20 : pagesizeI.intValue();
%>
	<table class="list" cellspacing="0">
		<tbody>
			<tr class="list">
				<th class="list" colspan="2">Alias list options</th>
			</tr>
			<tr class="list">
				<th class="list" nowrap width="10%">Subjects</th>
				<td class="list">
					<%=createSelector("available_names", available_names, "selected_names", selected_names, "names_selected")%></td>
				</td>
			</tr>
			<tr class="list">
				<th class="list" nowrap width="10%">Default SME's</th>
				<td class="list">
          <% for (Iterator i=serviceManager.getServiceIds().iterator(); i.hasNext(); ) { 
            String sme_name = (String) i.next();
            %><input type="checkbox" name="smes" <%=smes.contains(sme_name) ? "checked" : ""%> value="<%=StringEncoderDecoder.encode(sme_name)%>"><%=StringEncoderDecoder.encode(sme_name)%><br>
          <%}%>
				</td>
			</tr>
			<tr class="list">
				<th class="list" nowrap width="10%">Masks</th>
				<td class="list">
  				<textarea style="WIDTH: 100%" name="masks" rows="5"><%=masks%></textarea>
				</td>
			</tr>
			<tr class="list">
				<th class="list" nowrap width="10%">Items per page</th>
				<td class="list"><input value="<%=pagesize%>" name="pagesize"></td>
			</tr>
			<tr class="list">
				<td class="list" colspan="2">
					<input type="submit" name="action" value="Clear filter">
					<input type="submit" name="action" value="Refresh">
					<input type="submit" name="action" value="Apply filter">
				</td>
			</tr>
		</tbody>
	</table>
</form>
<%@ include file="/common/footer.jsp"%>