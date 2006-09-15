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
	int ACT_CONNECT = 15;
	int ACT_DISCONNECT = 16;
	int ADD_ACCESS = 17;
	int ADD_FOR = 18;
	int ADD_TO = 19;
	int TGT_ALL = 20;
	int TGT_ALIAS = 21;
	int TGT_ALIASES = 22;
	int TGT_ROUTE = 23;
	int TGT_ROUTES = 24;
	int TGT_PROFILE = 25;
	int TGT_SUBJECT = 26;
	int TGT_PRINCIPAL = 27;
	int TGT_DL = 28;
	int TGT_DLSUB = 29;
	int TGT_DLMEM = 30;
	int TGT_ACL = 31;
	int TGT_SME = 32;
	int TGT_PROVIDER = 33;
	int TGT_PROVIDERS = 34;
	int TGT_CATEGORY = 35;
	int TGT_CATEGORIES = 36;
	int TGT_STATS = 37;
	int TGT_STATS_E = 38;
	int TGT_SMS = 39;
	int TGT_SMS_E = 40;
	int TGT_ARCHIVE = 41;
	int TGT_GROUP = 42;
	int TGT_EMAILSME = 43;
	int OPT_ID = 44;
	int OPT_NAME = 45;
	int OPT_HIDE = 46;
	int OPT_NOHIDE = 47;
	int OPT_SUBSTIT = 48;
	int OPT_BILL = 49;
	int OPT_NOBILL = 50;
	int OPT_ARCH = 51;
	int OPT_NOARCH = 52;
	int OPT_ALLOW = 53;
	int OPT_DENY = 54;
	int OPT_RCPT = 55;
	int OPT_NORCPT = 56;
	int OPT_SRC = 57;
	int OPT_DST = 58;
	int OPT_MASK = 59;
	int OPT_SUBJ = 60;
	int OPT_SVCID = 61;
	int OPT_PRI = 62;
	int OPT_DEFSME = 63;
	int OPT_REPORT = 64;
	int OPT_ENCODE = 65;
	int OPT_NLIST = 66;
	int OPT_NELEM = 67;
	int OPT_OWNER = 68;
	int OPT_LOCALE = 69;
	int OPT_ACTIVE = 70;
	int OPT_INACTIVE = 71;
	int OPT_ABSENT = 72;
	int OPT_BARRED = 73;
	int OPT_BLOCKED = 74;
	int OPT_CAPACITY = 75;
	int OPT_UNCONDIT = 76;
	int OPT_MODIF = 77;
	int OPT_NOTMODIF = 78;
	int OPT_DIVERT = 79;
	int OPT_DM = 80;
	int OPT_FWD = 81;
	int OPT_SRCSME = 82;
	int OPT_SET = 83;
	int OPT_CLEAR = 84;
	int OPT_USSD7BIT = 85;
	int OPT_UDHCONCAT = 86;
	int OPT_TRANSLIT = 87;
	int OPT_ON = 88;
	int OPT_OFF = 89;
	int OPT_NOTES = 90;
	int OPT_RP = 91;
	int OPT_FD = 92;
	int OPT_CACHE = 93;
	int OPT_MODE = 94;
	int OPT_TYPE = 95;
	int OPT_SME_N = 96;
	int OPT_A_RANGE = 97;
	int OPT_TON = 98;
	int OPT_NPI = 99;
	int OPT_INT_V = 100;
	int OPT_SYS_TYPE = 101;
	int OPT_PASSWORD = 102;
	int OPT_TIMEOUT = 103;
	int OPT_R_SCHEME = 104;
	int OPT_P_LIMIT = 105;
	int OPT_S_LIMIT = 106;
	int OPT_WANT_ALIAS = 107;
	int OPT_FORCE_DC = 108;
	int OPT_DISABLED = 109;
	int OPT_TRANSIT = 110;
	int OPT_RENAME = 111;
	int OPT_DESCR = 112;
	int OPT_INPUTACCESSMASK = 113;
	int OPT_INPUTACCESSBIT = 114;
	int OPT_OUTPUTACCESSMASK = 115;
	int OPT_OUTPUTACCESSBIT = 116;
	int OPT_ACCESSMASK = 117;
	int OPT_ACCESSBIT = 118;
	int OPT_USERNAME = 119;
	int OPT_FORWARDEMAIL = 120;
	int OPT_REALNAME = 121;
	int OPT_ADDRESS = 122;
	int OPT_LIMITTYPE = 123;
	int OPT_LIMITVALUE = 124;
	int OPT_SERVICESMASK = 125;
	int OPT_SERVICESBIT = 126;
	int VAL_FORCE = 127;
	int VAL_SUPPRESS = 128;
	int VAL_PASS = 129;
	int VAL_FULL = 130;
	int VAL_FINAL = 131;
	int VAL_NONE = 132;
	int VAL_DEF = 133;
	int VAL_UCS2 = 134;
	int VAL_LATIN1 = 135;
	int VAL_UCS2LATIN1 = 136;
	int VAL_STORE = 137;
	int VAL_FORWARD = 138;
	int VAL_DATAGRAM = 139;
	int VAL_TX = 140;
	int VAL_RX = 141;
	int VAL_TRX = 142;
	int VAL_SMPP = 143;
	int VAL_SS7 = 144;
	int WS = 145;
	int STR = 146;
	int QSTR = 147;
	int STR_WS = 148;
	int STR_CHR = 149;
	int COMMA = 150;
	int ESC = 151;
	int DIGIT = 152;
}
