<%!
void button(JspWriter out, String srcImgUrl, String name, String value, String title) throws IOException
{
	button(out, srcImgUrl, name, value, title, null);
}

void button(JspWriter out, String srcImgUrl, String name, String value, String title, String onclick) throws IOException
{
	out.print("<img src=\"" + CPATH + "/img/" + srcImgUrl + "\" class=button jbuttonName=\"" + name + "\" jbuttonValue=\"" + value + "\" title=\"" + title + "\"" 
				+ (onclick != null && onclick.length() > 0 ? " jbuttonOnclick=\"" + onclick + "\"" : "") + ">");
}
void addButton(JspWriter out, String name, String value, String title) throws IOException
{
	button(out, "but_add.gif", name, value, title);
}
void delButton(JspWriter out, String name, String value, String title) throws IOException
{
	button(out, "but_del.gif", name, value, title);
}
%>