<script language="JavaScript">
  function moveItems( src, dst ) {
    for( var i = 0; i < src.options.length; i++ ) {
      if( src.options[i].selected ) {
        dst.options[dst.options.length] = new Option( src.options[i].text, src.options[i].value );
	src.options[i] = null;
        i--;
      }
    }
  }
  
  function updateResult(src, resultField) {
    var result = "";
    for( var i = 0; i < src.options.length; i++ ) {
      result += src.options[i].text + '"';
    }
    resultField.value = result;
  }
  
  function moveAndUpdate(src, dst, controlled, resultField)
  {
    moveItems(src, dst);
    updateResult(controlled, resultField);
  }
</script>
<%! String createSelector(String availableName, Set available, String selectedName, Set selected, String resultFieldName) 
{
  String result = "<table border=0 cellspacing=0 cellpading=0>"
                  + "<thead>"
                  +   "<tr>"
                  +     "<th width=\"50%\">Available</th>"
                  +     "<th width=\"1%\">&nbsp;</th>"
                  +     "<th width=\"50%\">Selected</th>"
                  +   "</tr>"
                  + "</thead>"
                  + "<tbody>"
                  +   "<tr>"
                  +     "<td><select name=\"" + availableName + "\" id=\"" + availableName + "\" multiple size=10 style=\"width: 100%\">";
  
  for (Iterator i=available.iterator(); i.hasNext(); )
    result += "<option>" + ((String) i.next()) + "</option>";
  
  result += "</select></td><td width=1"+'%'+">"
          + "<input type=button value=\"&gt;&gt;\" onClick=\"moveAndUpdate(" + availableName + ", " + selectedName + ", " + selectedName + ", " + resultFieldName + ")\"><br>"
          + "<input type=button value=\"&lt;&lt;\" onClick=\"moveAndUpdate(" + selectedName + ", " + availableName + ", " + selectedName + ", " + resultFieldName + ")\">"
          + "</td><td><select name=\"" + selectedName + "\" multiple size=10 style=\"width: 100%\">";
  
  for (Iterator i=selected.iterator(); i.hasNext(); )
    result += "<option>" + ((String) i.next()) + "</option>";
  
  result += "</select></td></tr></tbody></table><input type=\"Hidden\" name=\"" + resultFieldName + "\" value=\"";
  for (Iterator i=selected.iterator(); i.hasNext(); )
    result += ((String) i.next()) + "&quot;";
  result += "\">";
  
  return result;
}
%>
