<%!
String show_source(Source s)
{
  return "<font size=\"-2\">" + (s.isSubject() ? "subj" : "mask") + "</font>&nbsp;" + StringEncoderDecoder.encode(s.getName());
}

String show_sources(Route route)
{
  String result = "Sources: <b>";
  for (Iterator i = route.getSources().iterator(); i.hasNext(); ) {
    result += show_source((Source) i.next()) + " ";
  }
  result += "</b>";
  return result;
}

String show_destination(Destination d)
{
  return "<font size=\"-2\">" + (d.isSubject() ? "subj" : "mask") + "</font>&nbsp;" + StringEncoderDecoder.encode(d.getName()) 
         + "&nbsp;(" + StringEncoderDecoder.encode(d.getSme().getId()) + ')';
}

String show_destinations(Route route)
{
  String result = "Destinations: <b>";
  for (Iterator i = route.getDestinations().iterator(); i.hasNext(); ) {
    result += show_destination((Destination) i.next()) + " ";
  }
  result += "</b>";
  return result;
}

String show_set_combo(String name, Set values)
{
  String result = "<select name=\""+StringEncoderDecoder.encode(name)+"\">";
  for (Iterator j = values.iterator(); j.hasNext(); )
  {
    result += "<option>" + StringEncoderDecoder.encode((String) j.next()) + "</option>";
  }
  result += "</select>";
  return result;
}
%>