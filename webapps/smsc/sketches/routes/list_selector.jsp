
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
</script>

<table border=0 cellspacing=0 cellpading=0>
<tr><td>Available</td><td>&nbsp;</td><td>Selected</td></tr>
<tr>
  <td>
    <select name=list_available multiple size=10>
      <option>a1<option>a2<option>a3<option>a4<option>a5
    </select>
  </td>
  <td width=1%>
    <input type=button value="&gt;&gt;" onClick="moveItems( list_available, list_selected )">
    <input type=button value="&lt;&lt;" onClick="moveItems( list_selected, list_available )">
  </td>
  <td>
    <select name=list_selected multiple size=10>
      <option>a6<option>a7
    </select>
  </td>
</tr>


