// $ANTLR 2.7.1: "lexer.g" -> "CommandLexer.java"$

package ru.novosoft.smsc.admin.console.parser;

/* 
 * Lexer code generated by ANTLR 
 */

import java.io.InputStream;
import antlr.TokenStreamException;
import antlr.TokenStreamIOException;
import antlr.TokenStreamRecognitionException;
import antlr.CharStreamException;
import antlr.CharStreamIOException;
import antlr.ANTLRException;
import java.io.Reader;
import java.util.Hashtable;
import antlr.CharScanner;
import antlr.InputBuffer;
import antlr.ByteBuffer;
import antlr.CharBuffer;
import antlr.Token;
import antlr.CommonToken;
import antlr.RecognitionException;
import antlr.NoViableAltForCharException;
import antlr.MismatchedCharException;
import antlr.TokenStream;
import antlr.ANTLRHashString;
import antlr.LexerSharedInputState;
import antlr.collections.impl.BitSet;
import antlr.SemanticException;

public class CommandLexer extends antlr.CharScanner implements CommandLexerTokenTypes, TokenStream
 {
public CommandLexer(InputStream in) {
	this(new ByteBuffer(in));
}
public CommandLexer(Reader in) {
	this(new CharBuffer(in));
}
public CommandLexer(InputBuffer ib) {
	this(new LexerSharedInputState(ib));
}
public CommandLexer(LexerSharedInputState state) {
	super(state);
	literals = new Hashtable();
	literals.put(new ANTLRHashString("full", this), new Integer(141));
	literals.put(new ANTLRHashString("barred", this), new Integer(77));
	literals.put(new ANTLRHashString("translit", this), new Integer(91));
	literals.put(new ANTLRHashString("route", this), new Integer(24));
	literals.put(new ANTLRHashString("udhconcat", this), new Integer(90));
	literals.put(new ANTLRHashString("ucs2-latin1", this), new Integer(147));
	literals.put(new ANTLRHashString("owner", this), new Integer(72));
	literals.put(new ANTLRHashString("dlsubmitter", this), new Integer(30));
	literals.put(new ANTLRHashString("src", this), new Integer(61));
	literals.put(new ANTLRHashString("RX", this), new Integer(152));
	literals.put(new ANTLRHashString("pass", this), new Integer(140));
	literals.put(new ANTLRHashString("store", this), new Integer(148));
	literals.put(new ANTLRHashString("divert", this), new Integer(83));
	literals.put(new ANTLRHashString("aliases", this), new Integer(23));
	literals.put(new ANTLRHashString("outputaccessbit", this), new Integer(120));
	literals.put(new ANTLRHashString("clear", this), new Integer(88));
	literals.put(new ANTLRHashString("import", this), new Integer(45));
	literals.put(new ANTLRHashString("forceDelivery", this), new Integer(96));
	literals.put(new ANTLRHashString("grant", this), new Integer(11));
	literals.put(new ANTLRHashString("timeout", this), new Integer(107));
	literals.put(new ANTLRHashString("task", this), new Integer(133));
	literals.put(new ANTLRHashString("transit", this), new Integer(114));
	literals.put(new ANTLRHashString("nohide", this), new Integer(49));
	literals.put(new ANTLRHashString("for", this), new Integer(19));
	literals.put(new ANTLRHashString("check", this), new Integer(13));
	literals.put(new ANTLRHashString("routes", this), new Integer(25));
	literals.put(new ANTLRHashString("frbill", this), new Integer(54));
	literals.put(new ANTLRHashString("descr", this), new Integer(116));
	literals.put(new ANTLRHashString("SMPP", this), new Integer(154));
	literals.put(new ANTLRHashString("emailsme", this), new Integer(44));
	literals.put(new ANTLRHashString("servicesmask", this), new Integer(129));
	literals.put(new ANTLRHashString("arc", this), new Integer(55));
	literals.put(new ANTLRHashString("disabled", this), new Integer(113));
	literals.put(new ANTLRHashString("absent", this), new Integer(76));
	literals.put(new ANTLRHashString("replayPath", this), new Integer(95));
	literals.put(new ANTLRHashString("active", this), new Integer(74));
	literals.put(new ANTLRHashString("unconditional", this), new Integer(80));
	literals.put(new ANTLRHashString("providers", this), new Integer(35));
	literals.put(new ANTLRHashString("sme", this), new Integer(33));
	literals.put(new ANTLRHashString("nobill", this), new Integer(52));
	literals.put(new ANTLRHashString("proclimit", this), new Integer(109));
	literals.put(new ANTLRHashString("hide", this), new Integer(48));
	literals.put(new ANTLRHashString("inputaccessbit", this), new Integer(118));
	literals.put(new ANTLRHashString("none", this), new Integer(143));
	literals.put(new ANTLRHashString("outputaccessmask", this), new Integer(119));
	literals.put(new ANTLRHashString("noreceipt", this), new Integer(60));
	literals.put(new ANTLRHashString("inputaccessmask", this), new Integer(117));
	literals.put(new ANTLRHashString("cache", this), new Integer(97));
	literals.put(new ANTLRHashString("suppress", this), new Integer(139));
	literals.put(new ANTLRHashString("name", this), new Integer(47));
	literals.put(new ANTLRHashString("archive", this), new Integer(42));
	literals.put(new ANTLRHashString("view", this), new Integer(8));
	literals.put(new ANTLRHashString("forceDC", this), new Integer(112));
	literals.put(new ANTLRHashString("off", this), new Integer(93));
	literals.put(new ANTLRHashString("dlmember", this), new Integer(31));
	literals.put(new ANTLRHashString("forward", this), new Integer(149));
	literals.put(new ANTLRHashString("all", this), new Integer(21));
	literals.put(new ANTLRHashString("wantAlias", this), new Integer(111));
	literals.put(new ANTLRHashString("bill", this), new Integer(51));
	literals.put(new ANTLRHashString("status", this), new Integer(135));
	literals.put(new ANTLRHashString("limitvalue", this), new Integer(128));
	literals.put(new ANTLRHashString("latin1", this), new Integer(146));
	literals.put(new ANTLRHashString("addressRange", this), new Integer(101));
	literals.put(new ANTLRHashString("defaultsme", this), new Integer(67));
	literals.put(new ANTLRHashString("username", this), new Integer(123));
	literals.put(new ANTLRHashString("receipt", this), new Integer(59));
	literals.put(new ANTLRHashString("delete", this), new Integer(6));
	literals.put(new ANTLRHashString("modifiable", this), new Integer(81));
	literals.put(new ANTLRHashString("priority", this), new Integer(66));
	literals.put(new ANTLRHashString("limittype", this), new Integer(127));
	literals.put(new ANTLRHashString("accessmask", this), new Integer(121));
	literals.put(new ANTLRHashString("category", this), new Integer(36));
	literals.put(new ANTLRHashString("npi", this), new Integer(103));
	literals.put(new ANTLRHashString("connect", this), new Integer(16));
	literals.put(new ANTLRHashString("ton", this), new Integer(102));
	literals.put(new ANTLRHashString("to", this), new Integer(20));
	literals.put(new ANTLRHashString("realname", this), new Integer(125));
	literals.put(new ANTLRHashString("provider", this), new Integer(34));
	literals.put(new ANTLRHashString("principal", this), new Integer(28));
	literals.put(new ANTLRHashString("force", this), new Integer(138));
	literals.put(new ANTLRHashString("final", this), new Integer(142));
	literals.put(new ANTLRHashString("export", this), new Integer(14));
	literals.put(new ANTLRHashString("TX", this), new Integer(151));
	literals.put(new ANTLRHashString("deny", this), new Integer(58));
	literals.put(new ANTLRHashString("schedlimit", this), new Integer(110));
	literals.put(new ANTLRHashString("messages", this), new Integer(41));
	literals.put(new ANTLRHashString("rename", this), new Integer(115));
	literals.put(new ANTLRHashString("subject", this), new Integer(27));
	literals.put(new ANTLRHashString("list", this), new Integer(7));
	literals.put(new ANTLRHashString("SS7", this), new Integer(155));
	literals.put(new ANTLRHashString("ussd7bit", this), new Integer(89));
	literals.put(new ANTLRHashString("serviceid", this), new Integer(65));
	literals.put(new ANTLRHashString("remove", this), new Integer(137));
	literals.put(new ANTLRHashString("mask", this), new Integer(63));
	literals.put(new ANTLRHashString("statistics", this), new Integer(39));
	literals.put(new ANTLRHashString("acl", this), new Integer(32));
	literals.put(new ANTLRHashString("resend", this), new Integer(136));
	literals.put(new ANTLRHashString("notes", this), new Integer(94));
	literals.put(new ANTLRHashString("set", this), new Integer(87));
	literals.put(new ANTLRHashString("dm", this), new Integer(84));
	literals.put(new ANTLRHashString("TRX", this), new Integer(153));
	literals.put(new ANTLRHashString("ucs2", this), new Integer(145));
	literals.put(new ANTLRHashString("sms", this), new Integer(40));
	literals.put(new ANTLRHashString("interfaceVersion", this), new Integer(104));
	literals.put(new ANTLRHashString("fwd", this), new Integer(85));
	literals.put(new ANTLRHashString("locale", this), new Integer(73));
	literals.put(new ANTLRHashString("add", this), new Integer(4));
	literals.put(new ANTLRHashString("dl", this), new Integer(29));
	literals.put(new ANTLRHashString("subj", this), new Integer(64));
	literals.put(new ANTLRHashString("group", this), new Integer(43));
	literals.put(new ANTLRHashString("dst", this), new Integer(62));
	literals.put(new ANTLRHashString("default", this), new Integer(144));
	literals.put(new ANTLRHashString("numelem", this), new Integer(71));
	literals.put(new ANTLRHashString("accessbit", this), new Integer(122));
	literals.put(new ANTLRHashString("allow", this), new Integer(57));
	literals.put(new ANTLRHashString("alter", this), new Integer(5));
	literals.put(new ANTLRHashString("password", this), new Integer(106));
	literals.put(new ANTLRHashString("revoke", this), new Integer(12));
	literals.put(new ANTLRHashString("id", this), new Integer(46));
	literals.put(new ANTLRHashString("datagram", this), new Integer(150));
	literals.put(new ANTLRHashString("servicesbit", this), new Integer(130));
	literals.put(new ANTLRHashString("stats", this), new Integer(38));
	literals.put(new ANTLRHashString("apply", this), new Integer(9));
	literals.put(new ANTLRHashString("report", this), new Integer(68));
	literals.put(new ANTLRHashString("blocked", this), new Integer(78));
	literals.put(new ANTLRHashString("profile", this), new Integer(26));
	literals.put(new ANTLRHashString("encoding", this), new Integer(69));
	literals.put(new ANTLRHashString("smeN", this), new Integer(100));
	literals.put(new ANTLRHashString("disconnect", this), new Integer(17));
	literals.put(new ANTLRHashString("show", this), new Integer(10));
	literals.put(new ANTLRHashString("on", this), new Integer(92));
	literals.put(new ANTLRHashString("create", this), new Integer(134));
	literals.put(new ANTLRHashString("categories", this), new Integer(37));
	literals.put(new ANTLRHashString("systemType", this), new Integer(105));
	literals.put(new ANTLRHashString("receiptScheme", this), new Integer(108));
	literals.put(new ANTLRHashString("nick", this), new Integer(132));
	literals.put(new ANTLRHashString("forwardemail", this), new Integer(124));
	literals.put(new ANTLRHashString("mode", this), new Integer(98));
	literals.put(new ANTLRHashString("infosme", this), new Integer(15));
	literals.put(new ANTLRHashString("address", this), new Integer(126));
	literals.put(new ANTLRHashString("substitute", this), new Integer(50));
	literals.put(new ANTLRHashString("sponsored", this), new Integer(131));
	literals.put(new ANTLRHashString("capacity", this), new Integer(79));
	literals.put(new ANTLRHashString("mtbill", this), new Integer(53));
	literals.put(new ANTLRHashString("type", this), new Integer(99));
	literals.put(new ANTLRHashString("srcsme", this), new Integer(86));
	literals.put(new ANTLRHashString("access", this), new Integer(18));
	literals.put(new ANTLRHashString("numlist", this), new Integer(70));
	literals.put(new ANTLRHashString("inactive", this), new Integer(75));
	literals.put(new ANTLRHashString("alias", this), new Integer(22));
	literals.put(new ANTLRHashString("notmodifiable", this), new Integer(82));
	literals.put(new ANTLRHashString("noarc", this), new Integer(56));
caseSensitiveLiterals = true;
setCaseSensitive(true);
}

public Token nextToken() throws TokenStreamException {
	Token theRetToken=null;
tryAgain:
	for (;;) {
		Token _token = null;
		int _ttype = Token.INVALID_TYPE;
		resetText();
		try {   // for char stream error handling
			try {   // for lexical error handling
				switch ( LA(1)) {
				case '\t':  case '\n':  case '\u000c':  case '\r':
				case ' ':
				{
					mWS(true);
					theRetToken=_returnToken;
					break;
				}
				case ',':
				{
					mCOMMA(true);
					theRetToken=_returnToken;
					break;
				}
				default:
					if ((LA(1)=='"') && (_tokenSet_0.member(LA(2)))) {
						mQSTR(true);
						theRetToken=_returnToken;
					}
					else if ((LA(1)=='"') && (LA(2)=='"')) {
						mESTR(true);
						theRetToken=_returnToken;
					}
					else if ((_tokenSet_1.member(LA(1)))) {
						mSTR(true);
						theRetToken=_returnToken;
					}
				else {
					if (LA(1)==EOF_CHAR) {uponEOF(); _returnToken = makeToken(Token.EOF_TYPE);}
				else {throw new NoViableAltForCharException((char)LA(1), getFilename(), getLine());}
				}
				}
				if ( _returnToken==null ) continue tryAgain; // found SKIP token
				_ttype = _returnToken.getType();
				_ttype = testLiteralsTable(_ttype);
				_returnToken.setType(_ttype);
				return _returnToken;
			}
			catch (RecognitionException e) {
				throw new TokenStreamRecognitionException(e);
			}
		}
		catch (CharStreamException cse) {
			if ( cse instanceof CharStreamIOException ) {
				throw new TokenStreamIOException(((CharStreamIOException)cse).io);
			}
			else {
				throw new TokenStreamException(cse.getMessage());
			}
		}
	}
}

	public final void mWS(boolean _createToken) throws RecognitionException, CharStreamException, TokenStreamException {
		int _ttype; Token _token=null; int _begin=text.length();
		_ttype = WS;
		int _saveIndex;
		
		{
		switch ( LA(1)) {
		case '\t':  case '\u000c':  case ' ':
		{
			mSTR_WS(false);
			break;
		}
		case '\n':  case '\r':
		{
			{
			if ((LA(1)=='\r') && (LA(2)=='\n')) {
				match("\r\n");
			}
			else if ((LA(1)=='\r') && (true)) {
				match('\r');
			}
			else if ((LA(1)=='\n')) {
				match('\n');
			}
			else {
				throw new NoViableAltForCharException((char)LA(1), getFilename(), getLine());
			}
			
			}
			newline();
			break;
		}
		default:
		{
			throw new NoViableAltForCharException((char)LA(1), getFilename(), getLine());
		}
		}
		}
		_ttype = Token.SKIP;
		if ( _createToken && _token==null && _ttype!=Token.SKIP ) {
			_token = makeToken(_ttype);
			_token.setText(new String(text.getBuffer(), _begin, text.length()-_begin));
		}
		_returnToken = _token;
	}
	
	protected final void mSTR_WS(boolean _createToken) throws RecognitionException, CharStreamException, TokenStreamException {
		int _ttype; Token _token=null; int _begin=text.length();
		_ttype = STR_WS;
		int _saveIndex;
		
		switch ( LA(1)) {
		case ' ':
		{
			match(' ');
			break;
		}
		case '\t':
		{
			match('\t');
			break;
		}
		case '\u000c':
		{
			match('\f');
			break;
		}
		default:
		{
			throw new NoViableAltForCharException((char)LA(1), getFilename(), getLine());
		}
		}
		if ( _createToken && _token==null && _ttype!=Token.SKIP ) {
			_token = makeToken(_ttype);
			_token.setText(new String(text.getBuffer(), _begin, text.length()-_begin));
		}
		_returnToken = _token;
	}
	
	public final void mSTR(boolean _createToken) throws RecognitionException, CharStreamException, TokenStreamException {
		int _ttype; Token _token=null; int _begin=text.length();
		_ttype = STR;
		int _saveIndex;
		
		{
		int _cnt6=0;
		_loop6:
		do {
			if ((_tokenSet_1.member(LA(1)))) {
				mSTR_CHR(false);
			}
			else {
				if ( _cnt6>=1 ) { break _loop6; } else {throw new NoViableAltForCharException((char)LA(1), getFilename(), getLine());}
			}
			
			_cnt6++;
		} while (true);
		}
		if ( _createToken && _token==null && _ttype!=Token.SKIP ) {
			_token = makeToken(_ttype);
			_token.setText(new String(text.getBuffer(), _begin, text.length()-_begin));
		}
		_returnToken = _token;
	}
	
	protected final void mSTR_CHR(boolean _createToken) throws RecognitionException, CharStreamException, TokenStreamException {
		int _ttype; Token _token=null; int _begin=text.length();
		_ttype = STR_CHR;
		int _saveIndex;
		
		switch ( LA(1)) {
		case 'a':  case 'b':  case 'c':  case 'd':
		case 'e':  case 'f':  case 'g':  case 'h':
		case 'i':  case 'j':  case 'k':  case 'l':
		case 'm':  case 'n':  case 'o':  case 'p':
		case 'q':  case 'r':  case 's':  case 't':
		case 'u':  case 'v':  case 'w':  case 'x':
		case 'y':  case 'z':
		{
			matchRange('a','z');
			break;
		}
		case 'A':  case 'B':  case 'C':  case 'D':
		case 'E':  case 'F':  case 'G':  case 'H':
		case 'I':  case 'J':  case 'K':  case 'L':
		case 'M':  case 'N':  case 'O':  case 'P':
		case 'Q':  case 'R':  case 'S':  case 'T':
		case 'U':  case 'V':  case 'W':  case 'X':
		case 'Y':  case 'Z':
		{
			matchRange('A','Z');
			break;
		}
		case '0':  case '1':  case '2':  case '3':
		case '4':  case '5':  case '6':  case '7':
		case '8':  case '9':
		{
			matchRange('0','9');
			break;
		}
		case '_':
		{
			match('_');
			break;
		}
		case '$':
		{
			match('$');
			break;
		}
		case '.':
		{
			match('.');
			break;
		}
		case '?':
		{
			match('?');
			break;
		}
		case '!':
		{
			match('!');
			break;
		}
		case '#':
		{
			match('#');
			break;
		}
		case '+':
		{
			match('+');
			break;
		}
		case '-':
		{
			match('-');
			break;
		}
		case '/':
		{
			match('/');
			break;
		}
		case '\\':
		{
			match('\\');
			break;
		}
		case '*':
		{
			match('*');
			break;
		}
		case '%':
		{
			match('%');
			break;
		}
		case '@':
		{
			match('@');
			break;
		}
		case ':':
		{
			match(':');
			break;
		}
		case '\u00c0':  case '\u00c1':  case '\u00c2':  case '\u00c3':
		case '\u00c4':  case '\u00c5':  case '\u00c6':  case '\u00c7':
		case '\u00c8':  case '\u00c9':  case '\u00ca':  case '\u00cb':
		case '\u00cc':  case '\u00cd':  case '\u00ce':  case '\u00cf':
		case '\u00d0':  case '\u00d1':  case '\u00d2':  case '\u00d3':
		case '\u00d4':  case '\u00d5':  case '\u00d6':  case '\u00d7':
		case '\u00d8':  case '\u00d9':  case '\u00da':  case '\u00db':
		case '\u00dc':  case '\u00dd':  case '\u00de':  case '\u00df':
		case '\u00e0':  case '\u00e1':  case '\u00e2':  case '\u00e3':
		case '\u00e4':  case '\u00e5':  case '\u00e6':  case '\u00e7':
		case '\u00e8':  case '\u00e9':  case '\u00ea':  case '\u00eb':
		case '\u00ec':  case '\u00ed':  case '\u00ee':  case '\u00ef':
		case '\u00f0':  case '\u00f1':  case '\u00f2':  case '\u00f3':
		case '\u00f4':  case '\u00f5':  case '\u00f6':  case '\u00f7':
		case '\u00f8':  case '\u00f9':  case '\u00fa':  case '\u00fb':
		case '\u00fc':  case '\u00fd':  case '\u00fe':  case '\u00ff':
		{
			matchRange('\u00C0','\u00FF');
			break;
		}
		case '\u00b8':
		{
			match('\u00B8');
			break;
		}
		case '\u00a8':
		{
			match('\u00A8');
			break;
		}
		default:
			if (((LA(1) >= '\u0400' && LA(1) <= '\u04ff'))) {
				matchRange('\u0400','\u04FF');
			}
		else {
			throw new NoViableAltForCharException((char)LA(1), getFilename(), getLine());
		}
		}
		if ( _createToken && _token==null && _ttype!=Token.SKIP ) {
			_token = makeToken(_ttype);
			_token.setText(new String(text.getBuffer(), _begin, text.length()-_begin));
		}
		_returnToken = _token;
	}
	
	public final void mQSTR(boolean _createToken) throws RecognitionException, CharStreamException, TokenStreamException {
		int _ttype; Token _token=null; int _begin=text.length();
		_ttype = QSTR;
		int _saveIndex;
		
		match('"');
		{
		int _cnt9=0;
		_loop9:
		do {
			if ((LA(1)=='\\')) {
				mESC(false);
			}
			else if ((_tokenSet_2.member(LA(1)))) {
				matchNot('"');
			}
			else {
				if ( _cnt9>=1 ) { break _loop9; } else {throw new NoViableAltForCharException((char)LA(1), getFilename(), getLine());}
			}
			
			_cnt9++;
		} while (true);
		}
		match('"');
		if ( _createToken && _token==null && _ttype!=Token.SKIP ) {
			_token = makeToken(_ttype);
			_token.setText(new String(text.getBuffer(), _begin, text.length()-_begin));
		}
		_returnToken = _token;
	}
	
	protected final void mESC(boolean _createToken) throws RecognitionException, CharStreamException, TokenStreamException {
		int _ttype; Token _token=null; int _begin=text.length();
		_ttype = ESC;
		int _saveIndex;
		
		match('\\');
		{
		switch ( LA(1)) {
		case 'n':
		{
			match('n');
			break;
		}
		case 'r':
		{
			match('r');
			break;
		}
		case 't':
		{
			match('t');
			break;
		}
		case 'b':
		{
			match('b');
			break;
		}
		case 'f':
		{
			match('f');
			break;
		}
		case '"':
		{
			match('"');
			break;
		}
		case '\'':
		{
			match('\'');
			break;
		}
		case '\\':
		{
			match('\\');
			break;
		}
		case '0':  case '1':  case '2':  case '3':
		{
			matchRange('0','3');
			{
			if (((LA(1) >= '0' && LA(1) <= '9')) && (_tokenSet_3.member(LA(2)))) {
				mDIGIT(false);
				{
				if (((LA(1) >= '0' && LA(1) <= '9')) && (_tokenSet_3.member(LA(2)))) {
					mDIGIT(false);
				}
				else if ((_tokenSet_3.member(LA(1))) && (true)) {
				}
				else {
					throw new NoViableAltForCharException((char)LA(1), getFilename(), getLine());
				}
				
				}
			}
			else if ((_tokenSet_3.member(LA(1))) && (true)) {
			}
			else {
				throw new NoViableAltForCharException((char)LA(1), getFilename(), getLine());
			}
			
			}
			break;
		}
		case '4':  case '5':  case '6':  case '7':
		{
			matchRange('4','7');
			{
			if (((LA(1) >= '0' && LA(1) <= '9')) && (_tokenSet_3.member(LA(2)))) {
				mDIGIT(false);
			}
			else if ((_tokenSet_3.member(LA(1))) && (true)) {
			}
			else {
				throw new NoViableAltForCharException((char)LA(1), getFilename(), getLine());
			}
			
			}
			break;
		}
		default:
		{
			throw new NoViableAltForCharException((char)LA(1), getFilename(), getLine());
		}
		}
		}
		if ( _createToken && _token==null && _ttype!=Token.SKIP ) {
			_token = makeToken(_ttype);
			_token.setText(new String(text.getBuffer(), _begin, text.length()-_begin));
		}
		_returnToken = _token;
	}
	
	public final void mESTR(boolean _createToken) throws RecognitionException, CharStreamException, TokenStreamException {
		int _ttype; Token _token=null; int _begin=text.length();
		_ttype = ESTR;
		int _saveIndex;
		
		match('"');
		match('"');
		if ( _createToken && _token==null && _ttype!=Token.SKIP ) {
			_token = makeToken(_ttype);
			_token.setText(new String(text.getBuffer(), _begin, text.length()-_begin));
		}
		_returnToken = _token;
	}
	
	public final void mCOMMA(boolean _createToken) throws RecognitionException, CharStreamException, TokenStreamException {
		int _ttype; Token _token=null; int _begin=text.length();
		_ttype = COMMA;
		int _saveIndex;
		
		match(',');
		if ( _createToken && _token==null && _ttype!=Token.SKIP ) {
			_token = makeToken(_ttype);
			_token.setText(new String(text.getBuffer(), _begin, text.length()-_begin));
		}
		_returnToken = _token;
	}
	
	protected final void mDIGIT(boolean _createToken) throws RecognitionException, CharStreamException, TokenStreamException {
		int _ttype; Token _token=null; int _begin=text.length();
		_ttype = DIGIT;
		int _saveIndex;
		
		matchRange('0','9');
		if ( _createToken && _token==null && _ttype!=Token.SKIP ) {
			_token = makeToken(_ttype);
			_token.setText(new String(text.getBuffer(), _begin, text.length()-_begin));
		}
		_returnToken = _token;
	}
	
	
	private static final long _tokenSet_0_data_[] = { -17179869185L, -1L, -1L, -1L, 0L, 0L, 0L, 0L, 0L, 0L, 0L, 0L, 0L, 0L, 0L, 0L, -1L, -1L, -1L, -1L, 0L, 0L, 0L, 0L, 0L, 0L, 0L, 0L, 0L, 0L, 0L, 0L, 0L, 0L, 0L, 0L, 0L, 0L, 0L, 0L, 0L, 0L, 0L, 0L, 0L, 0L, 0L, 0L, 0L, 0L, 0L, 0L, 0L, 0L, 0L, 0L, 0L, 0L, 0L, 0L, 0L, 0L, 0L, 0L, 0L, 0L, 0L, 0L };
	public static final BitSet _tokenSet_0 = new BitSet(_tokenSet_0_data_);
	private static final long _tokenSet_1_data_[] = { -8646933025675804672L, 576460746263625727L, 72058693549555712L, -1L, 0L, 0L, 0L, 0L, 0L, 0L, 0L, 0L, 0L, 0L, 0L, 0L, -1L, -1L, -1L, -1L, 0L, 0L, 0L, 0L, 0L, 0L, 0L, 0L, 0L, 0L, 0L, 0L, 0L, 0L, 0L, 0L, 0L, 0L, 0L, 0L, 0L, 0L, 0L, 0L, 0L, 0L, 0L, 0L, 0L, 0L, 0L, 0L, 0L, 0L, 0L, 0L, 0L, 0L, 0L, 0L, 0L, 0L, 0L, 0L, 0L, 0L, 0L, 0L };
	public static final BitSet _tokenSet_1 = new BitSet(_tokenSet_1_data_);
	private static final long _tokenSet_2_data_[] = { -17179869185L, -268435457L, -1L, -1L, 0L, 0L, 0L, 0L, 0L, 0L, 0L, 0L, 0L, 0L, 0L, 0L, -1L, -1L, -1L, -1L, 0L, 0L, 0L, 0L, 0L, 0L, 0L, 0L, 0L, 0L, 0L, 0L, 0L, 0L, 0L, 0L, 0L, 0L, 0L, 0L, 0L, 0L, 0L, 0L, 0L, 0L, 0L, 0L, 0L, 0L, 0L, 0L, 0L, 0L, 0L, 0L, 0L, 0L, 0L, 0L, 0L, 0L, 0L, 0L, 0L, 0L, 0L, 0L };
	public static final BitSet _tokenSet_2 = new BitSet(_tokenSet_2_data_);
	private static final long _tokenSet_3_data_[] = { -1L, -1L, -1L, -1L, 0L, 0L, 0L, 0L, 0L, 0L, 0L, 0L, 0L, 0L, 0L, 0L, -1L, -1L, -1L, -1L, 0L, 0L, 0L, 0L, 0L, 0L, 0L, 0L, 0L, 0L, 0L, 0L, 0L, 0L, 0L, 0L, 0L, 0L, 0L, 0L, 0L, 0L, 0L, 0L, 0L, 0L, 0L, 0L, 0L, 0L, 0L, 0L, 0L, 0L, 0L, 0L, 0L, 0L, 0L, 0L, 0L, 0L, 0L, 0L, 0L, 0L, 0L, 0L };
	public static final BitSet _tokenSet_3 = new BitSet(_tokenSet_3_data_);
	
	}
