<%!
String show_source(Source s)
{
  return "<font style=\"font-size: smaller;\">" + (s.isSubject() ? "subj" : "mask") + "</font>&nbsp;" + StringEncoderDecoder.encode(s.getName());
}

String show_sources(Route route)
{
  String result = "<b>Sources:</b> ";
  for (Iterator i = route.getSources().iterator(); i.hasNext(); ) {
    result += show_source((Source) i.next()) + " ";
  }
  return result;
}

String show_destination(Destination d)
{
  return "<font style=\"font-size: smaller;\">" + (d.isSubject() ? "subj" : "mask") + "</font>&nbsp;" + StringEncoderDecoder.encode(d.getName()) 
         + "&nbsp;(" + StringEncoderDecoder.encode(d.getSme().getId()) + ')';
}

String show_destinations(Route route)
{
  String result = "<b>Destinations:</b> ";
  for (Iterator i = route.getDestinations().iterator(); i.hasNext(); ) {
    result += show_destination((Destination) i.next()) + " ";
  }
  return result;
}
%>