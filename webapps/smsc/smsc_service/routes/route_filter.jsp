<%@ page import = "ru.novosoft.smsc.jsp.util.tables.impl.RouteFilter"%>
<%@ include file="/common/header.jsp"%>
<%@ include file="/smsc_service/menu.jsp" %>
<%@ include file="/common/list_selector.jsp"%>
<%@ include file="utils.jsp" %>
<form method="post" action="route_filter_2.jsp">
<%
RouteFilter route_filter = (RouteFilter) session.getAttribute("route_filter");
if (route_filter == null)
  route_filter = new RouteFilter();

Set     selected_src  = route_filter.getSourceSubjectNames();
Set     available_src = new HashSet();
available_src.addAll(smsc.getSubjects().getNames());
available_src.removeAll(selected_src);
String  src_masks     = "";
for (Iterator i = route_filter.getSourceMaskStrings().iterator(); i.hasNext(); )
{
  src_masks += (String) i.next();
  if (i.hasNext())
    src_masks += '\n';
}

Set     selected_dst  = route_filter.getDestinationSubjectNames();
Set     available_dst = new HashSet();
available_dst.addAll(smsc.getSubjects().getNames());
available_dst.removeAll(selected_dst);
String  dst_masks     = "";
for (Iterator i = route_filter.getDestinationMaskStrings().iterator(); i.hasNext(); )
{
  dst_masks += (String) i.next();
  if (i.hasNext())
    dst_masks += '\n';
}
List    smes          = route_filter.getSmeIds();
boolean intersection  = route_filter.isIntersection();

Integer pagesizeI = (Integer) session.getAttribute("route_page_size");
int pagesize = pagesizeI == null ? 20 : pagesizeI.intValue();
%>
	<table class="list" cellspacing="0">
		<tbody>
			<tr class="list">
				<th class="list" colspan="2">Route list options</th>
			</tr>
			<tr class="list">
				<th class="list" nowrap width="10%">Sources</th>
				<td class="list">
					<table cellspacing="0" width="100%" border="1">
						<tbody>
							<tr>
								<td><%=createSelector("available_src", available_src, "selected_src", selected_src, "source_selected")%></td>
							</tr>
							<tr>
								<td colspan>
									<textarea style="WIDTH: 100%" name="src_masks" rows="5"><%=src_masks%></textarea>
								</td>
							</tr>
						</tbody>
					</table>
				</td>
			</tr>
			<tr class="list">
				<th class="list" nowrap width="10%">Destinations</th>
				<td class="list">
					<table cellspacing="0" width="100%" border="1">
						<tbody>
							<tr>
								<td><%=createSelector("available_dst", available_dst, "selected_dst", selected_dst, "destination_selected")%></td>
							</tr>
							<tr>
								<td colspan>
									<textarea style="WIDTH: 100%" name="dst_masks" rows="5"><%=dst_masks%></textarea>
								</td>
							</tr>
						</tbody>
					</table>
				</td>
			</tr>
			<tr class="list">
				<th class="list" nowrap width="10%">SME's</th>
				<td class="list">
          <% for (Iterator i=serviceManager.getServiceIds().iterator(); i.hasNext(); ) { 
            String sme_name = (String) i.next();
            %><input type="checkbox" <%=smes.contains(sme_name) ? "checked" : ""%> value="<%=StringEncoderDecoder.encode(sme_name)%>" name="smes"><%=StringEncoderDecoder.encode(sme_name)%><br>
          <%}%>
				</td>
			</tr>
			<tr class="list">
				<th class="list" nowrap width="10%">Items per page</th>
				<td class="list"><input value="<%=pagesize%>" name="pagesize"></td>
			</tr>
			<tr class="list">
				<th class="list" nowrap width="10%">Show info</th>
				<td class="list">
					<input type="checkbox" name="intersection" value="true" <%=intersection ? "checked" : ""%>>Strict&nbsp;Filter<br>
          <input type="checkbox" name="<%=getTriggerParamName("route_show_source_adresses")%>"      value="true" <%=process_trigger("route_show_source_adresses"     , session, request) ? "checked" : ""%>>Show&nbsp;sources&nbsp;list<br>
					<input type="checkbox" name="<%=getTriggerParamName("route_show_destination_adresses")%>" value="true" <%=process_trigger("route_show_destination_adresses", session, request) ? "checked" : ""%>>Show&nbsp;destinations&nbsp;list<br>
				</td>
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