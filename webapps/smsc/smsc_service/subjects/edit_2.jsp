<%@ include file="/common/header.jsp"%>
<%@ include file="/smsc_service/menu.jsp"%>
<h4>Subjects</h4>
<%
String name = request.getParameter("name");
String smeId = request.getParameter("sme");
String masks = request.getParameter("masks");
Subject s = routeManager.getSubjects().get(name);
if (s != null)
{
  s.setDefaultSme(new SME(smeId));
  s.setMasks(new MaskList(masks));
  %>Subject edited sucessfully.<%
}
else
{
  routeManager.getSubjects().add(new Subject(name, masks, new SME(smeId)));
  %>Subject added sucessfully.<%
}
%>
<%@ include file="/common/footer.jsp"%>