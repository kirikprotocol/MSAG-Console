<%@ include file="/common/header.jsp"%>
<%@ include file="/smsc_service/menu.jsp"%>
<%@ include file="/common/scripts.jsp"%>
<h4>Subjects</h4>
<%
String name = request.getParameter("name");
boolean isCreate = name == null;
Subject s = null;
if (!isCreate)
  s = smsc.getSubjects().get(name);
%>
<h5><%=isCreate ? "Create" : "Edit"%>&nbsp;subject</h5>
<form action="edit_2.jsp" method="post">
  <table class="list" cellspacing="0">
    <tr class="list">
      <th class="list" width="1%">Name</th>
      <td class="list" width="99%"><input type="Text" name="name" id="subject_name" style="width: 100%" value="<%=isCreate ? "" : StringEncoderDecoder.encode(s.getName())%>" <%=isCreate ? "" : "readonly"%>></td>
    </tr>
    <tr class="list">
      <th class="list" width="1%">Default&nbsp;SME</th>
      <td class="list" width="99%"><select name="sme" style="width: 100%"><%
        Set smes = serviceManager.getServiceIds();
        for (Iterator i = smes.iterator(); i.hasNext(); )
        {
          String smeName = (String) i.next();
          %><option value="<%=StringEncoderDecoder.encode(smeName)%>"<%=!isCreate && s.getDefaultSme().getId().equals(smeName) ? " selected" : ""%>><%=StringEncoderDecoder.encode(smeName)%></option><%
        }
      %></select></td>
    </tr>
    <tr class="list">
      <th class="list">Masks</th>
      <td class="list"><textarea cols="" rows="5" name="masks" id="masks" style="width: 100%"><%
        if (!isCreate) {
          MaskList masks = s.getMasks();
          for (Iterator i = masks.iterator(); i.hasNext();)
          {
            out.println(StringEncoderDecoder.encode(((Mask)i.next()).getMask()));
          }
        }
      %></textarea></td>
    </tr>
  </table>
  <input type="Submit" onclick="return checkMasks(masks.value) && checkName(subject_name.value)">
</form>
<%@ include file="/common/footer.jsp"%>