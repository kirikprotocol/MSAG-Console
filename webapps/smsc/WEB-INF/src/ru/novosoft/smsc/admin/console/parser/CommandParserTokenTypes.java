// $ANTLR 2.7.1: "parser.g" -> "CommandParser.java"$

package ru.novosoft.smsc.admin.console.parser;

/*
 * Parser code generated by ANTLR
 */

import ru.novosoft.smsc.admin.console.Command;
import ru.novosoft.smsc.admin.console.CommandContext;

import ru.novosoft.smsc.admin.console.commands.*;


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
	int TGT_ALIAS = 11;
	int TGT_ROUTE = 12;
	int TGT_PROFILE = 13;
	int TGT_SUBJECT = 14;
	int TGT_PRINCIPAL = 15;
	int TGT_DL = 16;
	int TGT_DLSUB = 17;
	int TGT_DLMEM = 18;
	int OPT_HIDE = 19;
	int OPT_NOHIDE = 20;
	int OPT_BILL = 21;
	int OPT_NOBILL = 22;
	int OPT_ARCH = 23;
	int OPT_NOARCH = 24;
	int OPT_ALLOW = 25;
	int OPT_DENY = 26;
	int OPT_RCPT = 27;
	int OPT_NORCPT = 28;
	int OPT_SRC = 29;
	int OPT_DST = 30;
	int OPT_MASK = 31;
	int OPT_SUBJ = 32;
	int OPT_SVCID = 33;
	int OPT_PRI = 34;
	int OPT_DEFSME = 35;
	int OPT_REPORT = 36;
	int OPT_ENCODE = 37;
	int OPT_NLIST = 38;
	int OPT_NELEM = 39;
	int OPT_OWNER = 40;
	int OPT_LOCALE = 41;
	int OPT_ACTIVE = 42;
	int OPT_INACTIVE = 43;
	int OPT_ABSENT = 44;
	int OPT_BARRED = 45;
	int OPT_BLOCKED = 46;
	int OPT_CAPACITY = 47;
	int OPT_UNCONDIT = 48;
	int OPT_MODIF = 49;
	int OPT_NOTMODIF = 50;
	int OPT_DIVERT = 51;
	int OPT_DM = 52;
	int OPT_FWD = 53;
	int OPT_SRCSME = 54;
	int OPT_SET = 55;
	int OPT_CLEAR = 56;
	int OPT_USSD7BIT = 57;
	int OPT_ON = 58;
	int OPT_OFF = 59;
	int VAL_FULL = 60;
	int VAL_NONE = 61;
	int VAL_DEF = 62;
	int VAL_UCS2 = 63;
	int VAL_LATIN1 = 64;
	int VAL_UCS2LATIN1 = 65;
	int VAL_STORE = 66;
	int VAL_FORWARD = 67;
	int VAL_DATAGRAM = 68;
	int WS = 69;
	int STR = 70;
	int QSTR = 71;
	int STR_WS = 72;
	int STR_CHR = 73;
	int COMMA = 74;
	int ESC = 75;
	int DIGIT = 76;
}
