<%@ include file="/common/header.jsp"%>
<%@ include file="/smsc_service/menu.jsp"%>
<h4>Subjects</h4>
<%
String name = request.getParameter("name");
String smeId = request.getParameter("sme");
String masks = request.getParameter("masks");
Subject s = smsc.getSubjects().get(name);
if (s != null)
{
  s.setDefaultSme(smsc.getSmes().get(smeId));
  s.setMasks(new MaskList(masks));
  %>Subject edited sucessfully.<%
}
else
{
  smsc.getSubjects().add(new Subject(name, masks, smsc.getSmes().get(smeId)));
  %>Subject added sucessfully.<%
}
%>
<%@ include file="/common/footer.jsp"%>