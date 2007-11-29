package ru.sibinco.smpp.ub_sme.util;

import ru.aurorisoft.smpp.Message;

import java.io.*;
import java.util.Properties;
import java.util.StringTokenizer;
import java.util.regex.Pattern;


public class Utils {

    private final static org.apache.log4j.Category Logger =
            org.apache.log4j.Category.getInstance(Utils.class);

    public static boolean getBooleanProperty(Properties properties, String key, boolean defaultValue) {
        boolean result = defaultValue;
        String value = properties.getProperty(key);
        if (value != null) {
            result = value.trim().equals("1") || value.trim().equalsIgnoreCase("yes") || value.trim().equalsIgnoreCase("true");
        }
        return result;
    }

    public static int getIntProperty(Properties properties, String key, int defaultValue) {
        int result = defaultValue;
        String value = properties.getProperty(key);
        if (value != null) {
            try {
                result = Integer.parseInt(value.trim());
            } catch (NumberFormatException e) {
            }
        }
        return result;
    }

    public static long getLongProperty(Properties properties, String key, long defaultValue) {
        long result = defaultValue;
        String value = properties.getProperty(key);
        if (value != null) {
            try {
                result = Long.parseLong(value.trim());
            } catch (NumberFormatException e) {
            }
        }
        return result;
    }

    /**
     * Unescape \x sequence
     *
     * @param s - incoming String
     * @return unescaped String
     */
    public static String unescape(String s) {
        StringBuffer sb = new StringBuffer(s.length());
        int l = s.length();
        boolean escape = false;
        for (int i = 0; i < l; i++) {
            char c = s.charAt(i);
            if (escape) {
                switch (c) {
                    case 'r':
                        sb.append('\r');
                        break;
                    case 'n':
                        sb.append('\n');
                        break;
                    case 't':
                        sb.append('\t');
                        break;
                    case '\\':
                        sb.append('\\');
                        break;
                    default:
                        sb.append(c);
                        break;
                }
                escape = false;
                continue;
            }
            if (c == '\\') {
                escape = true;
            } else {
                sb.append(c);
            }
        }
        return sb.toString();
    }

    public static String trim(String str) {
        if (str == null) return "";
        str = str.trim();
        if (str.length() == 0) return "";
        StringBuffer sb = new StringBuffer(str.length());
        BufferedReader rd = null;
        try {
            rd = new BufferedReader(new StringReader(str));
            String line;
            while ((line = rd.readLine()) != null) {
                sb.append(line.trim()).append('\n');
            }
        } catch (IOException e) {
        } finally {
            if (rd != null)
                try {
                    rd.close();
                } catch (IOException e) {
                }
        }
        return sb.toString().trim();
    }

    public static String aggregateRegexp(String pattern) throws IllegalArgumentException {
        StringTokenizer st = new StringTokenizer(pattern, ",");
        StringBuffer sb = new StringBuffer();
        if (st.hasMoreTokens()) {
            String token = st.nextToken();
            sb.append("(.*?");
            sb.append(token);
            sb.append(".*?)");
        }
        while (st.hasMoreTokens()) {
            String token = st.nextToken();
            sb.append("|(.*?");
            sb.append(token);
            sb.append(".*?)");
        }
        if (Logger.isDebugEnabled()) Logger.debug("Pattern " + pattern + " aggregated to regexp " + sb.toString());

        return sb.toString();
    }

    public static String getRegexpFromWildcard(String str) throws IllegalArgumentException {
        StringBuffer sb = new StringBuffer(str.length());
        for (int i = 0; i < str.length(); i++) {
            char ch = str.charAt(i);
            if (Character.isDigit(ch) || Character.isLetter(ch)) {
                sb.append(ch);
            } else if (ch == '*') {
                sb.append(".*");
            } else if (ch == '?') {
                sb.append(".");
            } else if (ch == '.') {
                sb.append("\\.");
            } else if (ch == '_') {
                sb.append("\\_");
            } else if (ch == ' ') {
                sb.append(" ");
            } else if (ch == ':') {
                sb.append(":");
            } else {
                throw new IllegalArgumentException("Invalid request processor name: " + str + ". Character " + i + " is invalid.");
            }
        }
        Pattern.compile(sb.toString());
        if (Logger.isDebugEnabled())
            Logger.debug("Wildcard " + str + " transformed to " + sb.toString() + " reqular expression");
        return sb.toString();
    }

    public static Message cloneMessage(Message message) {
        if (message == null)
            return null;
        Message result = new Message();
        result.setType(message.getType());
        result.setDestinationAddress(message.getDestinationAddress());
        result.setEsmClass(message.getEsmClass());
        result.setMessage(message.getMessage(), message.getEncoding());
        result.setSourceAddress(message.getSourceAddress());
        result.setConnectionName(message.getConnectionName());
        result.setOperatorName(message.getOperatorName());
        result.setServiceType(message.getServiceType());
        if (message.hasScheduleDeliveryTime())
            result.setScheduleDeliveryTime(message.getScheduleDeliveryTime());
        if (message.hasValidityPeriod())
            result.setValidityPeriod(message.getValidityPeriod());
        result.setReceiptRequested(message.getReceiptRequested());
        result.setReplaceIfPresent(message.isReplaceIfPresent());
        if (message.hasMsValidity())
            result.setMsValidity(message.getMsValidity());
        if (message.hasMsMsgWaitFacilities())
            result.setMsMsgWaitFacilities(message.getMsMsgWaitFacilities());
        if (message.hasItsSessionInfo())
            result.setItsSessionInfo(message.getItsSessionInfo());
        if (message.hasTransactionId())
            result.setTransactionId(message.getTransactionId());
        if (message.hasAddressPrefix())
            result.setAddressPrefix(message.getAddressPrefix());
        result.setForceMessagePayload(message.isForceMessagePayload());
        if (message.hasDestinationPort())
            result.setDestinationPort(message.getDestinationPort());
        if (message.hasSourcePort())
            result.setSourcePort(message.getSourcePort());
        if (message.hasUssdServiceOp())
            result.setUssdServiceOp(message.getUssdServiceOp());
        if (message.hasUserMessageReference())
            result.setUserMessageReference(message.getUserMessageReference());
        if (message.hasSarMsgRefNum())
            result.setSarMsgRefNum(message.getSarMsgRefNum());
        if (message.hasSarSegmentSeqnum())
            result.setSarSegmentSeqnum(message.getSarSegmentSeqnum());
        if (message.hasSarTotalSegments())
            result.setSarTotalSegments(message.getSarTotalSegments());
        if (message.hasMessageTransportType())
            result.setMessageTransportType(message.getMessageTransportType());
        if (message.hasMessageContentType())
            result.setMessageContentType(message.getMessageContentType());
        if (message.hasExpectedMessageTransportType())
            result.setExpectedMessageTransportType(message.getExpectedMessageTransportType());
        if (message.hasExpectedMessageContentType())
            result.setExpectedMessageContentType(message.getExpectedMessageContentType());
        if (message.hasChargingType())
            result.setChargingType(message.getChargingType());
        if (message.hasSdpResponseType())
            result.setSdpResponseType(message.getSdpResponseType());
        if (message.hasDeliverRoute())
            result.setDeliverRoute(message.getDeliverRoute());
        if (message.hasDialogDirective())
            result.setDialogDirective(message.getDialogDirective());
        return result;
    }

    public static String trimAbonent(String abonent) {
        if (abonent == null || abonent.length() == 0) {
            return null;
        }
        int i = abonent.length();
        while (--i >= 0 && Character.isDigit(abonent.charAt(i))) ;
        return abonent.substring(++i);
    }
}
