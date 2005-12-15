<%@ page import="java.io.IOException"%><%!
void button(JspWriter out, String srcImgUrl, String name, String value, String title) throws IOException
{
	button(out, srcImgUrl, name, value, title, null);
}

void button(JspWriter out, String srcImgUrl, String name, String value, String title, String onclick) throws IOException
{
	out.print("<img id=\""+name+"\" src=\"" + srcImgUrl + "\" class=button title=\"" + getLocString(title) + "\""
				+ (onclick != null && onclick.length() > 0 ? " onClick=\"" + onclick + "\"" : " onClick=\"clickSubmit('"+name+"','"+value+"')\"") + ">");
}
void addButton(JspWriter out, String name, String value, String title) throws IOException
{
	addButton(out, name, value, title, null);
}
void addButton(JspWriter out, String name, String value, String title, String onclick) throws IOException
{
	button(out, "/images/but_add.gif", name, value, title, onclick);
}
void delButton(JspWriter out, String name, String value, String title) throws IOException
{
  delButton(out, name, value, title, null);
}
void delButton(JspWriter out, String name, String value, String title, String onclick) throws IOException
{
	button(out, "/images/but_del.gif", name, value, title, onclick);
}
%>