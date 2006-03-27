// $ANTLR 2.7.1: "parser.g" -> "CommandParser.java"$

package ru.novosoft.smsc.admin.console.parser;

/*
 * Parser code generated by ANTLR
 */

import ru.novosoft.smsc.admin.AdminException;
import ru.novosoft.smsc.admin.console.Command;
import ru.novosoft.smsc.admin.console.CommandContext;

import ru.novosoft.smsc.admin.console.commands.*;
import ru.novosoft.smsc.admin.console.commands.acl.*;
import ru.novosoft.smsc.admin.console.commands.alias.*;
import ru.novosoft.smsc.admin.console.commands.dl.*;
import ru.novosoft.smsc.admin.console.commands.profile.*;
import ru.novosoft.smsc.admin.console.commands.route.*;
import ru.novosoft.smsc.admin.console.commands.sme.*;
import ru.novosoft.smsc.admin.console.commands.misc.*;
import ru.novosoft.smsc.admin.console.commands.apply.*;
import ru.novosoft.smsc.admin.console.commands.closedgroup.*;


public interface CommandParserTokenTypes {
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
	int TGT_GROUP = 41;
	int OPT_NAME = 42;
	int OPT_HIDE = 43;
	int OPT_NOHIDE = 44;
	int OPT_SUBSTIT = 45;
	int OPT_BILL = 46;
	int OPT_NOBILL = 47;
	int OPT_ARCH = 48;
	int OPT_NOARCH = 49;
	int OPT_ALLOW = 50;
	int OPT_DENY = 51;
	int OPT_RCPT = 52;
	int OPT_NORCPT = 53;
	int OPT_SRC = 54;
	int OPT_DST = 55;
	int OPT_MASK = 56;
	int OPT_SUBJ = 57;
	int OPT_SVCID = 58;
	int OPT_PRI = 59;
	int OPT_DEFSME = 60;
	int OPT_REPORT = 61;
	int OPT_ENCODE = 62;
	int OPT_NLIST = 63;
	int OPT_NELEM = 64;
	int OPT_OWNER = 65;
	int OPT_LOCALE = 66;
	int OPT_ACTIVE = 67;
	int OPT_INACTIVE = 68;
	int OPT_ABSENT = 69;
	int OPT_BARRED = 70;
	int OPT_BLOCKED = 71;
	int OPT_CAPACITY = 72;
	int OPT_UNCONDIT = 73;
	int OPT_MODIF = 74;
	int OPT_NOTMODIF = 75;
	int OPT_DIVERT = 76;
	int OPT_DM = 77;
	int OPT_FWD = 78;
	int OPT_SRCSME = 79;
	int OPT_SET = 80;
	int OPT_CLEAR = 81;
	int OPT_USSD7BIT = 82;
	int OPT_UDHCONCAT = 83;
	int OPT_TRANSLIT = 84;
	int OPT_ON = 85;
	int OPT_OFF = 86;
	int OPT_NOTES = 87;
	int OPT_RP = 88;
	int OPT_FD = 89;
	int OPT_CACHE = 90;
	int OPT_MODE = 91;
	int OPT_TYPE = 92;
	int OPT_SME_N = 93;
	int OPT_A_RANGE = 94;
	int OPT_TON = 95;
	int OPT_NPI = 96;
	int OPT_INT_V = 97;
	int OPT_SYS_TYPE = 98;
	int OPT_PASSWORD = 99;
	int OPT_TIMEOUT = 100;
	int OPT_R_SCHEME = 101;
	int OPT_P_LIMIT = 102;
	int OPT_S_LIMIT = 103;
	int OPT_WANT_ALIAS = 104;
	int OPT_FORCE_DC = 105;
	int OPT_DISABLED = 106;
	int OPT_TRANSIT = 107;
	int OPT_RENAME = 108;
	int OPT_DESCR = 109;
	int OPT_INPUTACCESSMASK = 110;
	int OPT_INPUTACCESSBIT = 111;
	int OPT_OUTPUTACCESSMASK = 112;
	int OPT_OUTPUTACCESSBIT = 113;
	int OPT_ACCESSMASK = 114;
	int OPT_ACCESSBIT = 115;
	int VAL_FORCE = 116;
	int VAL_SUPPRESS = 117;
	int VAL_PASS = 118;
	int VAL_FULL = 119;
	int VAL_FINAL = 120;
	int VAL_NONE = 121;
	int VAL_DEF = 122;
	int VAL_UCS2 = 123;
	int VAL_LATIN1 = 124;
	int VAL_UCS2LATIN1 = 125;
	int VAL_STORE = 126;
	int VAL_FORWARD = 127;
	int VAL_DATAGRAM = 128;
	int VAL_TX = 129;
	int VAL_RX = 130;
	int VAL_TRX = 131;
	int VAL_SMPP = 132;
	int VAL_SS7 = 133;
	int WS = 134;
	int STR = 135;
	int QSTR = 136;
	int STR_WS = 137;
	int STR_CHR = 138;
	int COMMA = 139;
	int ESC = 140;
	int DIGIT = 141;
	int TGT_PROVEDERS = 142;
}
