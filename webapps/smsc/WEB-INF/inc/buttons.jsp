<%@ page import="java.io.IOException"%><%!
void button(JspWriter out, String srcImgUrl, String name, String value, String title) throws IOException
{
	button(out, srcImgUrl, name, value, title, null);
}

void button(JspWriter out, String srcImgUrl, String name, String value, String title, String onclick) throws IOException
{
	out.print("<img src=\"/images/" + srcImgUrl + "\" class=button jbuttonName=\"" + name + "\" jbuttonValue=\"" + value + "\" title=\"" + title + "\""
				+ (onclick != null && onclick.length() > 0 ? " jbuttonOnclick=\"" + onclick + "\"" : "") + ">");
}
void addButton(JspWriter out, String name, String value, String title) throws IOException
{
	addButton(out, name, value, title, null);
}
void addButton(JspWriter out, String name, String value, String title, String onclick) throws IOException
{
	button(out, "but_add.gif", name, value, title, onclick);
}
void delButton(JspWriter out, String name, String value, String title) throws IOException
{
  delButton(out, name, value, title, null);
}
void delButton(JspWriter out, String name, String value, String title, String onclick) throws IOException
{
	button(out, "but_del.gif", name, value, title, onclick);
}
%>