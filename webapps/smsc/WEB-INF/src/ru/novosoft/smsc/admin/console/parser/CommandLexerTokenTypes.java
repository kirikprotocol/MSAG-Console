// $ANTLR 2.7.1: "lexer.g" -> "CommandLexer.java"$

package ru.novosoft.smsc.admin.console.parser;

/* 
 * Lexer code generated by ANTLR 
 */

public interface CommandLexerTokenTypes {
	int EOF = 1;
	int NULL_TREE_LOOKAHEAD = 3;
	int ACT_ADD = 4;
	int ACT_ALTER = 5;
	int ACT_DELETE = 6;
	int ACT_LIST = 7;
	int ACT_VIEW = 8;
	int ACT_APPLY = 9;
	int ACT_SHOW = 10;
	int ACT_GRANT = 11;
	int ACT_REVOKE = 12;
	int ACT_CHECK = 13;
	int ACT_EXPORT = 14;
	int ACT_INFOSME = 15;
	int ACT_CONNECT = 16;
	int ACT_DISCONNECT = 17;
	int ADD_ACCESS = 18;
	int ADD_FOR = 19;
	int ADD_TO = 20;
	int TGT_ALL = 21;
	int TGT_ALIAS = 22;
	int TGT_ALIASES = 23;
	int TGT_ROUTE = 24;
	int TGT_ROUTES = 25;
	int TGT_PROFILE = 26;
	int TGT_SUBJECT = 27;
	int TGT_PRINCIPAL = 28;
	int TGT_DL = 29;
	int TGT_DLSUB = 30;
	int TGT_DLMEM = 31;
	int TGT_ACL = 32;
	int TGT_SME = 33;
	int TGT_PROVIDER = 34;
	int TGT_PROVIDERS = 35;
	int TGT_CATEGORY = 36;
	int TGT_CATEGORIES = 37;
	int TGT_STATS = 38;
	int TGT_STATS_E = 39;
	int TGT_SMS = 40;
	int TGT_SMS_E = 41;
	int TGT_ARCHIVE = 42;
	int TGT_GROUP = 43;
	int TGT_EMAILSME = 44;
	int TGT_IMPORT = 45;
	int OPT_ID = 46;
	int OPT_NAME = 47;
	int OPT_HIDE = 48;
	int OPT_NOHIDE = 49;
	int OPT_SUBSTIT = 50;
	int OPT_BILL = 51;
	int OPT_NOBILL = 52;
	int OPT_MTBILL = 53;
	int OPT_FRBILL = 54;
	int OPT_ARCH = 55;
	int OPT_NOARCH = 56;
	int OPT_ALLOW = 57;
	int OPT_DENY = 58;
	int OPT_RCPT = 59;
	int OPT_NORCPT = 60;
	int OPT_SRC = 61;
	int OPT_DST = 62;
	int OPT_MASK = 63;
	int OPT_SUBJ = 64;
	int OPT_SVCID = 65;
	int OPT_PRI = 66;
	int OPT_DEFSME = 67;
	int OPT_REPORT = 68;
	int OPT_ENCODE = 69;
	int OPT_NLIST = 70;
	int OPT_NELEM = 71;
	int OPT_OWNER = 72;
	int OPT_LOCALE = 73;
	int OPT_ACTIVE = 74;
	int OPT_INACTIVE = 75;
	int OPT_ABSENT = 76;
	int OPT_BARRED = 77;
	int OPT_BLOCKED = 78;
	int OPT_CAPACITY = 79;
	int OPT_UNCONDIT = 80;
	int OPT_MODIF = 81;
	int OPT_NOTMODIF = 82;
	int OPT_DIVERT = 83;
	int OPT_DM = 84;
	int OPT_FWD = 85;
	int OPT_SRCSME = 86;
	int OPT_SET = 87;
	int OPT_CLEAR = 88;
	int OPT_USSD7BIT = 89;
	int OPT_UDHCONCAT = 90;
	int OPT_TRANSLIT = 91;
	int OPT_ON = 92;
	int OPT_OFF = 93;
	int OPT_NOTES = 94;
	int OPT_RP = 95;
	int OPT_FD = 96;
	int OPT_CACHE = 97;
	int OPT_MODE = 98;
	int OPT_TYPE = 99;
	int OPT_SME_N = 100;
	int OPT_A_RANGE = 101;
	int OPT_TON = 102;
	int OPT_NPI = 103;
	int OPT_INT_V = 104;
	int OPT_SYS_TYPE = 105;
	int OPT_PASSWORD = 106;
	int OPT_TIMEOUT = 107;
	int OPT_R_SCHEME = 108;
	int OPT_P_LIMIT = 109;
	int OPT_S_LIMIT = 110;
	int OPT_WANT_ALIAS = 111;
	int OPT_FORCE_DC = 112;
	int OPT_DISABLED = 113;
	int OPT_TRANSIT = 114;
	int OPT_RENAME = 115;
	int OPT_DESCR = 116;
	int OPT_INPUTACCESSMASK = 117;
	int OPT_INPUTACCESSBIT = 118;
	int OPT_OUTPUTACCESSMASK = 119;
	int OPT_OUTPUTACCESSBIT = 120;
	int OPT_ACCESSMASK = 121;
	int OPT_ACCESSBIT = 122;
	int OPT_USERNAME = 123;
	int OPT_FORWARDEMAIL = 124;
	int OPT_REALNAME = 125;
	int OPT_ADDRESS = 126;
	int OPT_LIMITTYPE = 127;
	int OPT_LIMITVALUE = 128;
	int OPT_SERVICESMASK = 129;
	int OPT_SERVICESBIT = 130;
	int OPT_SPONSORED = 131;
	int OPT_NICK = 132;
	int OPT_TASK = 133;
	int VAL_FORCE = 134;
	int VAL_SUPPRESS = 135;
	int VAL_PASS = 136;
	int VAL_FULL = 137;
	int VAL_FINAL = 138;
	int VAL_NONE = 139;
	int VAL_DEF = 140;
	int VAL_UCS2 = 141;
	int VAL_LATIN1 = 142;
	int VAL_UCS2LATIN1 = 143;
	int VAL_STORE = 144;
	int VAL_FORWARD = 145;
	int VAL_DATAGRAM = 146;
	int VAL_TX = 147;
	int VAL_RX = 148;
	int VAL_TRX = 149;
	int VAL_SMPP = 150;
	int VAL_SS7 = 151;
	int WS = 152;
	int STR = 153;
	int QSTR = 154;
	int ESTR = 155;
	int STR_WS = 156;
	int STR_CHR = 157;
	int COMMA = 158;
	int ESC = 159;
	int DIGIT = 160;
}
