header {
package ru.novosoft.smsc.admin.console.parser;

/* 
 * Lexer code generated by ANTLR 
 */
}

class CommandLexer extends Lexer;
options {
	k=2;
	charVocabulary = '\u0000'..'\u00FF';
	exportVocab=CommandLexer; // use vocab generated by lexer
	defaultErrorHandler=false;
}
tokens {
  ACT_ADD       = "add";
  ACT_ALTER     = "alter";
  ACT_DELETE    = "delete";
  ACT_LIST      = "list";
  ACT_VIEW      = "view";
  ACT_APPLY     = "apply";
  ACT_SHOW      = "show";
  ACT_GRANT     = "grant";
  ACT_REVOKE    = "revoke";
  ACT_CHECK     = "check";

  ADD_ACCESS    = "access";
  ADD_FOR       = "for";
  ADD_TO        = "to";

  TGT_ALIAS     = "alias";
  TGT_ROUTE     = "route";
  TGT_PROFILE   = "profile";
  TGT_SUBJECT   = "subject";
  TGT_PRINCIPAL = "principal";
  TGT_DL      	= "dl";
  TGT_DLSUB     = "dlsubmitter";
  TGT_DLMEM     = "dlmember";
  TGT_ACL       = "acl";

  OPT_NAME      = "name";
  OPT_HIDE      = "hide";
  OPT_NOHIDE    = "nohide";
  OPT_SUBSTIT   = "substitute";
  OPT_BILL      = "bill";
  OPT_NOBILL    = "nobill";
  OPT_ARCH      = "arc";
  OPT_NOARCH    = "noarc";
  OPT_ALLOW     = "allow";
  OPT_DENY      = "deny";
  OPT_RCPT      = "receipt";
  OPT_NORCPT    = "noreceipt";
  OPT_SRC       = "src";
  OPT_DST       = "dst";
  OPT_MASK      = "mask";
  OPT_SUBJ      = "subj";
  OPT_SVCID     = "serviceid";
  OPT_PRI       = "priority";
  OPT_DEFSME    = "defaultsme";
  OPT_REPORT    = "report";
  OPT_ENCODE    = "encoding";
  OPT_NLIST     = "numlist";
  OPT_NELEM     = "numelem";
  OPT_OWNER     = "owner";
  OPT_LOCALE    = "locale";
  OPT_ACTIVE    = "active";
  OPT_INACTIVE  = "inactive";
  OPT_ABSENT    = "absent";
  OPT_BARRED    = "barred";
  OPT_BLOCKED   = "blocked";
  OPT_CAPACITY  = "capacity";
  OPT_UNCONDIT  = "unconditional";
  OPT_MODIF     = "modifiable";
  OPT_NOTMODIF  = "notmodifiable";
  OPT_DIVERT    = "divert";
  OPT_DM        = "dm";
  OPT_FWD       = "fwd";
  OPT_SRCSME    = "srcsme";
  OPT_SET       = "set";
  OPT_CLEAR     = "clear";
  OPT_USSD7BIT  = "ussd7bit";
  OPT_UDHCONCAT = "udhconcat";
  OPT_ON        = "on";
  OPT_OFF       = "off";
  OPT_NOTES     = "notes";
  OPT_RP        = "replayPath";
  OPT_FD        = "forceDelivery";
  OPT_CACHE     = "cache";

  VAL_FORCE     = "force";
  VAL_SUPPRESS  = "suppress";
  VAL_PASS      = "pass";
  VAL_FULL      = "full";
  VAL_NONE      = "none";
  VAL_DEF       = "default";
  VAL_UCS2      = "ucs2";
  VAL_LATIN1    = "latin1";
  VAL_UCS2LATIN1= "ucs2-latin1";
  VAL_STORE     = "store";
  VAL_FORWARD   = "forward";
  VAL_DATAGRAM  = "datagram";
}

WS    	: 	( STR_WS
		// handle newlines
		| (	"\r\n"  // Evil DOS
		    |	'\r'    // Macintosh
		    |	'\n'    // Unix (the right way)
		  )
		  { newline(); }
		)
		{ _ttype = Token.SKIP; }
	;

STR
options {
  paraphrase = "more input";
}
	:	(STR_CHR)+
	;
	
QSTR
options {
  paraphrase = "quoted string";
}
	:	'"' (ESC | ~'"')+ '"'
	;

protected
STR_WS	:	' '|'\t'|'\f'
	;

protected
STR_CHR	:	'a'..'z'|'A'..'Z'|'0'..'9'|'_'|'$'
		|'.'|'?'|'!'|'#'|'+'|'-'|'/'|'*'|'%'
		|'\u00C0'..'\u00FF'|'\u00B8'|'\u00A8'
	;

COMMA
options {
  paraphrase = "comma character ','";
}
	:	','
	;

protected
ESC	:	'\\'
		(	'n'
		|	'r'
		|	't'
		|	'b'
		|	'f'
		|	'"'
		|	'\''
		|	'\\'
		|	'0'..'3'
			(
				options {
					warnWhenFollowAmbig = false;
				}
			:	DIGIT
				(
					options {
						warnWhenFollowAmbig = false;
					}
				:	DIGIT
				)?
			)?
		|	'4'..'7'
			(
				options {
					warnWhenFollowAmbig = false;
				}
			:	DIGIT
			)?
		)
	;

protected
DIGIT
	:	'0'..'9'
	;

