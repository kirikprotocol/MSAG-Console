<%@ page import = "ru.novosoft.smsc.jsp.util.tables.impl.alias.AliasFilter"%>
<%@ include file="/common/header.jsp"%>
<%@ include file="/smsc_service/menu.jsp" %>
<%@ include file="/common/list_selector.jsp"%>
<form method="post" action="filter_2.jsp">
<%
AliasFilter filter = (AliasFilter) session.getAttribute("alias_filter");
if (filter == null)
  filter = new AliasFilter();

String aliases = "";
for (Iterator i = filter.getAliasStrings().iterator(); i.hasNext(); )
{
  aliases += (String) i.next();
  if (i.hasNext())
    aliases += '\n';
}

String addresses = "";
for (Iterator i = filter.getAddressStrings().iterator(); i.hasNext(); )
{
  addresses += (String) i.next();
  if (i.hasNext())
    addresses += '\n';
}

int hide_option = filter.getHideOption();

Integer pagesizeI = (Integer) session.getAttribute("alias_page_size");
int pagesize = pagesizeI == null ? 20 : pagesizeI.intValue();
%>
	<table class="list" cellspacing="0">
		<tbody>
			<tr class="list">
				<th class="list" colspan="2">Alias list options</th>
			</tr>
			<tr class="list">
				<th class="list" nowrap width="10%">Aliases<br>
				<font size="-2">Enter aliases <br>
				delimited by space<br>
				or new line</font></th>
				<td class="list">
  				<textarea style="WIDTH: 100%" name="aliases" rows="5"><%=aliases%></textarea>
				</td>
			</tr>
			<tr class="list">
				<th class="list" nowrap width="10%">Addresses<br>
				<font size="-2">Enter addresses<br>
				delimited by space<br>
				or new line</font></th>
				<td class="list">
  				<textarea style="WIDTH: 100%" name="addresses" rows="5"><%=addresses%></textarea>
				</td>
			</tr>
			<tr class="list">
				<th class="list" nowrap width="10%">Hide</th>
				<td class="list">
				<select name="hide">
					<option value="no_filter"<%=hide_option == filter.HIDE_NOFILTER    ? " SELECTED" : ""%>>Show all</option>
					<option value="true"     <%=hide_option == filter.HIDE_SHOW_HIDE   ? " SELECTED" : ""%>>Show hided only</option>
					<option value="false"    <%=hide_option == filter.HIDE_SHOW_NOHIDE ? " SELECTED" : ""%>>Show not hided only</option>
				</select>
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