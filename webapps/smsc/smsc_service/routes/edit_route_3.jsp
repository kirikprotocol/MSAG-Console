<%@ include file="/common/header.jsp"%>
<%@ include file="/smsc_service/menu.jsp" %>
<%!
String showSources(Route route)
{
  String result = "";
  for (Iterator i = route.getSources().iterator(); i.hasNext();)
  {
    Source s = (Source) i.next();
    result += (s.isSubject() ? "<font size=\"-2\">subj</font> " : "<font size=\"-2\">mask</font> ") + s.getName() + "<br>";
  }
  
  return result;
}

String showDestinations(Route route, Set smes)
{
  String result = "";
  for (Iterator i = route.getDestinations().iterator(); i.hasNext();)
  {
    Destination d = (Destination) i.next();
    result += (d.isSubject() ? "<font size=\"-2\">subj</font> " : "<font size=\"-2\">mask</font> ") + d.getName()
           +  "<select name=\""+StringEncoderDecoder.encode(d.getName())+"\">";
    for (Iterator j = smes.iterator(); j.hasNext(); )
    {
      result += "<option>" + StringEncoderDecoder.encode((String) j.next()) + "</option>";
    }
    result += "</select><br>";
  }

  return result;
}
%><%
Set serviceIDs = serviceManager.getServiceIds();
if (serviceIDs.isEmpty())
  throw new NullPointerException("SME list is empty");

//SME defaultSme = new SME((String)serviceIDs.iterator().next());

String route_name = request.getParameter("name");
Route route = smsc.getRoutes().get(route_name);
if (route == null)
  throw new Exception("Route not specified");

DestinationList dsts = route.getDestinations();
for (Iterator i = dsts.iterator(); i.hasNext(); )
{
  Destination dst = (Destination) i.next();
  String smeId = request.getParameter(dst.getName());
  if (smeId == null)
    throw new Exception("SME not defined for destination \"" + dst.getName() + "\"");
  dst.setSme(smsc.getSmes().get(smeId));
}
%>
  <h4>Edit Route:</h4>
  Route edited sucessfully
<%@ include file="/common/footer.jsp"%>