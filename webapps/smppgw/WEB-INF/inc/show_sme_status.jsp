<%@ page import="ru.sibinco.lib.backend.sme.SmeStatus,ru.sibinco.smppgw.backend.SmppGWAppContext,
                 ru.sibinco.lib.backend.sme.Sme,
                 ru.sibinco.lib.backend.util.StringEncoderDecoder"%><%!
String showSmeStatus(SmeStatus status)
{
	String result = "";
	if (status != null)
	{
		if (status.isInternal())
		{
			result += "><img src=\"/images/ic_internal.gif\" width=10 height=10 title='internal'></span>";
		}
		else
		{
			if (!status.isConnected())
			{
				result += "><img src=\"/images/ic_disconnct.gif\" width=10 height=10 title='disconnected'></span>";
			}
			else
			{
				switch (status.getBindMode())
				{
					case Sme.MODE_TX:
						result += " class=C080>TX&nbsp;" + status.getInAddress() + "</span>";
						break;
					case Sme.MODE_RX:
						result += " class=C080>RX&nbsp;" + status.getInAddress() + "</span>";
						break;
					case Sme.MODE_TRX:
						result += " class=C080>TRX&nbsp;" + status.getInAddress() + "</span>";
						break;
					default:
						result += " class=C080>unknown&nbsp;" + status.getInAddress() + '/' + status.getOutAddress() + "</span>";
				}
			}
		}
	}
	else
	{
		result += " class=C000>unknown</span>";
	}
	return result;
}
%>