<%@ page import = "ru.novosoft.smsc.admin.route.*"%>
<%@ page import = "ru.novosoft.smsc.util.StringEncoderDecoder"%>
<%!
String show_source(Source s)
{
  return "<font style=\"font-size: smaller;\">" + (s.isSubject() ? "subj" : "mask") + "</font>&nbsp;" + StringEncoderDecoder.encode(s.getName());
}

String show_sources(SourceList sources)
{
  String result = "<b>Sources:</b> ";
  for (Iterator i = sources.iterator(); i.hasNext(); ) {
    result += show_source((Source) i.next()) + " ";
  }
  return result;
}

String show_destination(Destination d)
{
  return "<font style=\"font-size: smaller;\">" + (d.isSubject() ? "subj" : "mask") + "</font>&nbsp;" + StringEncoderDecoder.encode(d.getName()) 
         + "&nbsp;(" + StringEncoderDecoder.encode(d.getSme().getId()) + ')';
}

String show_destinations(DestinationList destinations)
{
  String result = "<b>Destinations:</b> ";
  for (Iterator i = destinations.iterator(); i.hasNext(); ) {
    result += show_destination((Destination) i.next()) + " ";
  }
  return result;
}
%>