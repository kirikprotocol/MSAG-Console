	<%@ include file="/sketches/header.jsp" %>
  <%@ include file="menu.jsp"%>
  <% boolean isDest = request.getParameter("recipient") != null && request.getParameter("recipient").equals("dest"); %>
   <H4>Add <%= isDest ? "destination" : "source" %></H4>
   <FORM action="add_<%= isDest ? "destination_sme" : "route" %>.jsp" method=post>
   	<TABLE class="list" cellSpacing="0">
   		<TBODY>
   			<TR class="list">
   				<TH class="list">	Subjects
   				</TH>
   				<TD class="list">
   				   <INPUT type="checkbox" CHECKED value="Subject 1" name="src_subj">Subject 1<BR>
   				   <INPUT type="checkbox" CHECKED value="Subject 2" name="src_subj">Subject 2<BR>
   				   <INPUT type="checkbox" CHECKED value="Subject 3" name="src_subj">Subject 3<BR>
   				   <INPUT type="checkbox" CHECKED value="Subject 4" name="src_subj">Subject 4<BR>
   				   <INPUT type="checkbox" CHECKED value="Subject 5" name="src_subj">Subject 5<BR>
   				</TD>
   			</TR>
   			<TR class="list">
   				<TH class="list" noWrap width="10%">	Additional masks
   				</TH>
   				<TD class="list">
   					<TEXTAREA style="WIDTH: 100%" name="masks" rows="5" wrap="off">+7901 +7801
  911</TEXTAREA>
   				</TD>
   			</TR>
   		</TBODY>
   	</TABLE>
      <INPUT type="submit" value="Submit Query">
   </FORM>
	<%@ include file="/sketches/footer.jsp" %>