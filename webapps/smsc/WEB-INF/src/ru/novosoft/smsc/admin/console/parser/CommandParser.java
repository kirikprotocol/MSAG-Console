// $ANTLR 2.7.1: "parser.g" -> "CommandParser.java"$

package ru.novosoft.smsc.admin.console.parser;

/*
 * Parser code generated by ANTLR
 */

import ru.novosoft.smsc.admin.console.Command;
import ru.novosoft.smsc.admin.console.CommandContext;

import ru.novosoft.smsc.admin.console.commands.*;


import antlr.TokenBuffer;
import antlr.TokenStreamException;
import antlr.TokenStreamIOException;
import antlr.ANTLRException;
import antlr.LLkParser;
import antlr.Token;
import antlr.TokenStream;
import antlr.RecognitionException;
import antlr.NoViableAltException;
import antlr.MismatchedTokenException;
import antlr.SemanticException;
import antlr.ParserSharedInputState;
import antlr.collections.impl.BitSet;
import antlr.collections.AST;
import antlr.ASTPair;
import antlr.collections.impl.ASTArray;

public class CommandParser extends antlr.LLkParser
       implements CommandParserTokenTypes
 {

protected CommandParser(TokenBuffer tokenBuf, int k) {
  super(tokenBuf,k);
  tokenNames = _tokenNames;
}

public CommandParser(TokenBuffer tokenBuf) {
  this(tokenBuf,1);
}

protected CommandParser(TokenStream lexer, int k) {
  super(lexer,k);
  tokenNames = _tokenNames;
}

public CommandParser(TokenStream lexer) {
  this(lexer,1);
}

public CommandParser(ParserSharedInputState state) {
  super(state,1);
  tokenNames = _tokenNames;
}

	public final Command  parse() throws RecognitionException, TokenStreamException {
		Command cmd;
		
		
		cmd = null;
		
		
		switch ( LA(1)) {
		case ACT_ADD:
		{
			match(ACT_ADD);
			cmd=add();
			break;
		}
		case ACT_DELETE:
		{
			match(ACT_DELETE);
			cmd=del();
			break;
		}
		case ACT_ALTER:
		{
			match(ACT_ALTER);
			cmd=alt();
			break;
		}
		case ACT_LIST:
		{
			match(ACT_LIST);
			cmd=lst();
			break;
		}
		case ACT_VIEW:
		{
			match(ACT_VIEW);
			cmd=view();
			break;
		}
		case ACT_APPLY:
		{
			match(ACT_APPLY);
			cmd = new ApplyCommand();
			break;
		}
		default:
		{
			throw new NoViableAltException(LT(1), getFilename());
		}
		}
		return cmd;
	}
	
	public final Command  add() throws RecognitionException, TokenStreamException {
		Command cmd;
		
		
		cmd = null;
		
		
		switch ( LA(1)) {
		case TGT_ROUTE:
		{
			match(TGT_ROUTE);
			cmd=addroute();
			break;
		}
		case TGT_ALIAS:
		{
			match(TGT_ALIAS);
			cmd=addalias();
			break;
		}
		case TGT_SUBJECT:
		{
			match(TGT_SUBJECT);
			cmd=addsubject();
			break;
		}
		case TGT_PROFILE:
		{
			match(TGT_PROFILE);
			cmd=addprofile();
			break;
		}
		default:
		{
			throw new NoViableAltException(LT(1), getFilename());
		}
		}
		return cmd;
	}
	
	public final Command  del() throws RecognitionException, TokenStreamException {
		Command cmd;
		
		
		cmd = null;
		
		
		switch ( LA(1)) {
		case TGT_ROUTE:
		{
			match(TGT_ROUTE);
			cmd=delroute();
			break;
		}
		case TGT_ALIAS:
		{
			match(TGT_ALIAS);
			cmd=delalias();
			break;
		}
		case TGT_SUBJECT:
		{
			match(TGT_SUBJECT);
			cmd=delsubject();
			break;
		}
		default:
		{
			throw new NoViableAltException(LT(1), getFilename());
		}
		}
		return cmd;
	}
	
	public final Command  alt() throws RecognitionException, TokenStreamException {
		Command cmd;
		
		
		cmd = null;
		
		
		switch ( LA(1)) {
		case TGT_ROUTE:
		{
			match(TGT_ROUTE);
			cmd=altroute();
			break;
		}
		case TGT_ALIAS:
		{
			match(TGT_ALIAS);
			cmd=altalias();
			break;
		}
		case TGT_SUBJECT:
		{
			match(TGT_SUBJECT);
			cmd=altsubject();
			break;
		}
		case TGT_PROFILE:
		{
			match(TGT_PROFILE);
			cmd=altprofile();
			break;
		}
		default:
		{
			throw new NoViableAltException(LT(1), getFilename());
		}
		}
		return cmd;
	}
	
	public final Command  lst() throws RecognitionException, TokenStreamException {
		Command cmd;
		
		
		cmd = null;
		
		
		switch ( LA(1)) {
		case TGT_ROUTE:
		{
			match(TGT_ROUTE);
			cmd = new RouteListCommand();
			break;
		}
		case TGT_ALIAS:
		{
			match(TGT_ALIAS);
			cmd = new AliasListCommand();
			break;
		}
		case TGT_SUBJECT:
		{
			match(TGT_SUBJECT);
			cmd = new SubjectListCommand();
			break;
		}
		default:
		{
			throw new NoViableAltException(LT(1), getFilename());
		}
		}
		return cmd;
	}
	
	public final Command  view() throws RecognitionException, TokenStreamException {
		Command cmd;
		
		
		cmd = null;
		
		
		switch ( LA(1)) {
		case TGT_ROUTE:
		{
			match(TGT_ROUTE);
			cmd=viewroute();
			break;
		}
		case TGT_ALIAS:
		{
			match(TGT_ALIAS);
			cmd=viewalias();
			break;
		}
		case TGT_SUBJECT:
		{
			match(TGT_SUBJECT);
			cmd=viewsubject();
			break;
		}
		case TGT_PROFILE:
		{
			match(TGT_PROFILE);
			cmd=viewprofile();
			break;
		}
		default:
		{
			throw new NoViableAltException(LT(1), getFilename());
		}
		}
		return cmd;
	}
	
	public final RouteAddCommand  addroute() throws RecognitionException, TokenStreamException {
		RouteAddCommand cmd;
		
		Token  num = null;
		Token  pri = null;
		
		cmd = new RouteAddCommand();
		
		
		{
		
				    cmd.setRoute(getnameid("Route name"));
				
		}
		addroute_flags(cmd);
		{
		match(OPT_SVCID);
		num = LT(1);
		match(STR);
		
				    try {
					cmd.setServiceId(Integer.parseInt(num.getText()));
				    } catch (NumberFormatException ex) {
					throw new NumberFormatException("Expecting integer value for <serviceid>");
				    }
				
		}
		{
		match(OPT_PRI);
		pri = LT(1);
		match(STR);
		
				    try {
					cmd.setPriority(Integer.parseInt(pri.getText()));		    
				    } catch (NumberFormatException ex) {
					throw new NumberFormatException("Expecting integer value for <priority>");
				    }
				
		}
		route_src(cmd);
		route_dst(cmd);
		return cmd;
	}
	
	public final AliasAddCommand  addalias() throws RecognitionException, TokenStreamException {
		AliasAddCommand cmd;
		
		Token  mask = null;
		Token  addr = null;
		
		cmd = new AliasAddCommand();
		
		
		{
		try { // for error handling
			mask = LT(1);
			match(STR);
		}
		catch (RecognitionException ex) {
			
				    throw new RecognitionException("Alias mask expected");
				
		}
		cmd.setAlias(mask.getText());
		}
		{
		try { // for error handling
			addr = LT(1);
			match(STR);
		}
		catch (RecognitionException ex) {
			
				    throw new RecognitionException("Alias address expected");
				
		}
		cmd.setAddress(addr.getText());	
		}
		{
		switch ( LA(1)) {
		case OPT_HIDE:
		{
			match(OPT_HIDE);
			cmd.setHide(true);
			break;
		}
		case OPT_NOHIDE:
		{
			match(OPT_NOHIDE);
			cmd.setHide(false);
			break;
		}
		case EOF:
		{
			break;
		}
		default:
		{
			throw new NoViableAltException(LT(1), getFilename());
		}
		}
		}
		return cmd;
	}
	
	public final SubjectAddCommand  addsubject() throws RecognitionException, TokenStreamException {
		SubjectAddCommand cmd;
		
		
		cmd = new SubjectAddCommand();
		
		
		{
		
				    cmd.setSubject(getnameid("Subject name"));
				
		}
		{
		
				    cmd.setDefaultSmeId(getnameid("SME id"));
				
		}
		addsubj_masks(cmd);
		return cmd;
	}
	
	public final ProfileAddCommand  addprofile() throws RecognitionException, TokenStreamException {
		ProfileAddCommand cmd;
		
		Token  mask = null;
		
		cmd = new ProfileAddCommand();
		
		
		{
		try { // for error handling
			mask = LT(1);
			match(STR);
		}
		catch (RecognitionException ex) {
			
				    throw new RecognitionException("Profile mask expected");
				
		}
		cmd.setMask(mask.getText());
		}
		{
		match(OPT_REPORT);
		{
		switch ( LA(1)) {
		case VAL_FULL:
		{
			match(VAL_FULL);
			cmd.setFullReport();
			break;
		}
		case VAL_NONE:
		{
			match(VAL_NONE);
			cmd.setNoneReport();
			break;
		}
		default:
		{
			throw new NoViableAltException(LT(1), getFilename());
		}
		}
		}
		}
		{
		switch ( LA(1)) {
		case OPT_ENCODE:
		{
			match(OPT_ENCODE);
			{
			switch ( LA(1)) {
			case VAL_DEF:
			{
				match(VAL_DEF);
				cmd.setGsm7Encoding();
				break;
			}
			case VAL_UCS2:
			{
				match(VAL_UCS2);
				cmd.setUcs2Encoding();
				break;
			}
			default:
			{
				throw new NoViableAltException(LT(1), getFilename());
			}
			}
			}
			break;
		}
		case EOF:
		{
			break;
		}
		default:
		{
			throw new NoViableAltException(LT(1), getFilename());
		}
		}
		}
		return cmd;
	}
	
	public final RouteDeleteCommand  delroute() throws RecognitionException, TokenStreamException {
		RouteDeleteCommand cmd;
		
		
		cmd = new RouteDeleteCommand();
		
		
		{
		
				    cmd.setRoute(getnameid("Route name"));
				
		}
		return cmd;
	}
	
	public final AliasDeleteCommand  delalias() throws RecognitionException, TokenStreamException {
		AliasDeleteCommand cmd;
		
		Token  mask = null;
		
		cmd = new AliasDeleteCommand();
		
		
		{
		try { // for error handling
			mask = LT(1);
			match(STR);
		}
		catch (RecognitionException ex) {
			
				    throw new RecognitionException("Alias mask expected");
				
		}
		cmd.setAlias(mask.getText());
		}
		return cmd;
	}
	
	public final SubjectDeleteCommand  delsubject() throws RecognitionException, TokenStreamException {
		SubjectDeleteCommand cmd;
		
		
		cmd = new SubjectDeleteCommand();
		
		
		{
		
				    cmd.setSubject(getnameid("Subject name"));
				
		}
		return cmd;
	}
	
	public final RouteAlterCommand  altroute() throws RecognitionException, TokenStreamException {
		RouteAlterCommand cmd;
		
		Token  num = null;
		Token  pri = null;
		
		cmd = new RouteAlterCommand();
		
		
		{
		
				    cmd.setRoute(getnameid("Route name"));
				
		}
		altroute_flags(cmd);
		{
		switch ( LA(1)) {
		case OPT_SVCID:
		{
			match(OPT_SVCID);
			num = LT(1);
			match(STR);
			
					    try {
						cmd.setServiceId(Integer.parseInt(num.getText()));
					    } catch (NumberFormatException ex) {
						throw new NumberFormatException("Expecting integer value for <serviceid>");
					    }
					
			break;
		}
		case EOF:
		case ACT_ADD:
		case ACT_DELETE:
		case OPT_PRI:
		{
			break;
		}
		default:
		{
			throw new NoViableAltException(LT(1), getFilename());
		}
		}
		}
		{
		switch ( LA(1)) {
		case OPT_PRI:
		{
			match(OPT_PRI);
			pri = LT(1);
			match(STR);
			
					    try {
						cmd.setPriority(Integer.parseInt(pri.getText()));		    
					    } catch (NumberFormatException ex) {
						throw new NumberFormatException("Expecting integer value for <priority>");
					    }
					
			break;
		}
		case EOF:
		case ACT_ADD:
		case ACT_DELETE:
		{
			break;
		}
		default:
		{
			throw new NoViableAltException(LT(1), getFilename());
		}
		}
		}
		{
		switch ( LA(1)) {
		case ACT_ADD:
		{
			{
			match(ACT_ADD);
			cmd.setAction(RouteAlterCommand.ACTION_ADD);
			{
			switch ( LA(1)) {
			case OPT_SRC:
			{
				route_src(cmd);
				cmd.setTarget(RouteAlterCommand.TARGET_SRC);
				break;
			}
			case OPT_DST:
			{
				route_dst(cmd);
				cmd.setTarget(RouteAlterCommand.TARGET_DST);
				break;
			}
			default:
			{
				throw new NoViableAltException(LT(1), getFilename());
			}
			}
			}
			}
			break;
		}
		case ACT_DELETE:
		{
			{
			match(ACT_DELETE);
			cmd.setAction(RouteAlterCommand.ACTION_DEL);
			{
			switch ( LA(1)) {
			case OPT_SRC:
			{
				route_src(cmd);
				cmd.setTarget(RouteAlterCommand.TARGET_SRC);
				break;
			}
			case OPT_DST:
			{
				route_dst_x(cmd);
				cmd.setTarget(RouteAlterCommand.TARGET_DST);
				break;
			}
			default:
			{
				throw new NoViableAltException(LT(1), getFilename());
			}
			}
			}
			}
			break;
		}
		case EOF:
		{
			break;
		}
		default:
		{
			throw new NoViableAltException(LT(1), getFilename());
		}
		}
		}
		return cmd;
	}
	
	public final AliasAlterCommand  altalias() throws RecognitionException, TokenStreamException {
		AliasAlterCommand cmd;
		
		Token  mask = null;
		Token  addr = null;
		
		cmd = new AliasAlterCommand();
		
		
		{
		try { // for error handling
			mask = LT(1);
			match(STR);
		}
		catch (RecognitionException ex) {
			
				    throw new RecognitionException("Alias mask expected");
				
		}
		cmd.setAlias(mask.getText());
		}
		{
		try { // for error handling
			addr = LT(1);
			match(STR);
		}
		catch (RecognitionException ex) {
			
				    throw new RecognitionException("Alias address expected");
				
		}
		cmd.setAddress(addr.getText());
		}
		{
		switch ( LA(1)) {
		case OPT_HIDE:
		{
			match(OPT_HIDE);
			cmd.setHide(true);
			break;
		}
		case OPT_NOHIDE:
		{
			match(OPT_NOHIDE);
			cmd.setHide(false);
			break;
		}
		case EOF:
		{
			break;
		}
		default:
		{
			throw new NoViableAltException(LT(1), getFilename());
		}
		}
		}
		return cmd;
	}
	
	public final SubjectAlterCommand  altsubject() throws RecognitionException, TokenStreamException {
		SubjectAlterCommand cmd;
		
		
		cmd = new SubjectAlterCommand();
		
		
		{
		
				    cmd.setSubject(getnameid("Subject name"));
				
		}
		{
		switch ( LA(1)) {
		case ACT_ADD:
		case ACT_DELETE:
		{
			{
			{
			switch ( LA(1)) {
			case ACT_ADD:
			{
				match(ACT_ADD);
				
						    cmd.setActionAdd();
						
				break;
			}
			case ACT_DELETE:
			{
				match(ACT_DELETE);
				
						    cmd.setActionDelete();
						
				break;
			}
			default:
			{
				throw new NoViableAltException(LT(1), getFilename());
			}
			}
			}
			addsubj_masks(cmd);
			}
			break;
		}
		case OPT_DEFSME:
		{
			{
			match(OPT_DEFSME);
			
					    cmd.setDefaultSmeId(getnameid("SME id"));
					
			}
			break;
		}
		default:
		{
			throw new NoViableAltException(LT(1), getFilename());
		}
		}
		}
		return cmd;
	}
	
	public final ProfileAlterCommand  altprofile() throws RecognitionException, TokenStreamException {
		ProfileAlterCommand cmd;
		
		Token  addr = null;
		
		cmd = new ProfileAlterCommand();
		
		
		{
		try { // for error handling
			addr = LT(1);
			match(STR);
		}
		catch (RecognitionException ex) {
			
				    throw new RecognitionException("Profile address expected");
				
		}
		cmd.setAddress(addr.getText());
		}
		{
		match(OPT_REPORT);
		{
		switch ( LA(1)) {
		case VAL_FULL:
		{
			match(VAL_FULL);
			cmd.setFullReport();
			break;
		}
		case VAL_NONE:
		{
			match(VAL_NONE);
			cmd.setNoneReport();
			break;
		}
		default:
		{
			throw new NoViableAltException(LT(1), getFilename());
		}
		}
		}
		}
		{
		switch ( LA(1)) {
		case OPT_ENCODE:
		{
			match(OPT_ENCODE);
			{
			switch ( LA(1)) {
			case VAL_DEF:
			{
				match(VAL_DEF);
				cmd.setGsm7Encoding();
				break;
			}
			case VAL_UCS2:
			{
				match(VAL_UCS2);
				cmd.setUcs2Encoding();
				break;
			}
			default:
			{
				throw new NoViableAltException(LT(1), getFilename());
			}
			}
			}
			break;
		}
		case EOF:
		{
			break;
		}
		default:
		{
			throw new NoViableAltException(LT(1), getFilename());
		}
		}
		}
		return cmd;
	}
	
	public final RouteViewCommand  viewroute() throws RecognitionException, TokenStreamException {
		RouteViewCommand cmd;
		
		
		cmd = new RouteViewCommand();
		
		
		{
		
				    cmd.setRoute(getnameid("Route name"));
				
		}
		return cmd;
	}
	
	public final AliasViewCommand  viewalias() throws RecognitionException, TokenStreamException {
		AliasViewCommand cmd;
		
		Token  mask = null;
		
		cmd = new AliasViewCommand();
		
		
		{
		try { // for error handling
			mask = LT(1);
			match(STR);
		}
		catch (RecognitionException ex) {
			
				    throw new RecognitionException("Alias mask expected");
				
		}
		cmd.setAlias(mask.getText());
		}
		return cmd;
	}
	
	public final SubjectViewCommand  viewsubject() throws RecognitionException, TokenStreamException {
		SubjectViewCommand cmd;
		
		
		cmd = new SubjectViewCommand();
		
		
		{
		
				    cmd.setSubject(getnameid("Subject name"));
				
		}
		return cmd;
	}
	
	public final ProfileViewCommand  viewprofile() throws RecognitionException, TokenStreamException {
		ProfileViewCommand cmd;
		
		Token  addr = null;
		
		cmd = new ProfileViewCommand();
		
		
		{
		try { // for error handling
			addr = LT(1);
			match(STR);
		}
		catch (RecognitionException ex) {
			
				    throw new RecognitionException("Profile address expected");
				
		}
		cmd.setAddress(addr.getText());
		}
		return cmd;
	}
	
	public final String  getnameid(
		String msg
	) throws RecognitionException, TokenStreamException {
		String out;
		
		Token  qname = null;
		Token  name = null;
		
		out = null; 
		
		
		try {      // for error handling
			switch ( LA(1)) {
			case QSTR:
			{
				{
				qname = LT(1);
				match(QSTR);
				
						    out = qname.getText().trim();
						    out = out.substring(1,out.length()-1);
						
				}
				break;
			}
			case STR:
			{
				{
				name = LT(1);
				match(STR);
				
						    out = name.getText();
						
				}
				break;
			}
			default:
			{
				throw new NoViableAltException(LT(1), getFilename());
			}
			}
		}
		catch (RecognitionException ex) {
			
			throw new RecognitionException(ex.getMessage()+". "+
								  msg+" expected. ");
				
		}
		return out;
	}
	
	public final void srcdef(
		RouteGenCommand cmd
	) throws RecognitionException, TokenStreamException {
		
		Token  addr = null;
		// Special command required !!!
		RouteSrcDef def = new RouteSrcDef();
		
		
		{
		switch ( LA(1)) {
		case OPT_SUBJ:
		{
			{
			match(OPT_SUBJ);
			
					    def.setType(RouteSrcDef.TYPE_SUBJECT);
					    def.setSrc(getnameid("Subject name"));
					
			}
			break;
		}
		case OPT_MASK:
		{
			{
			match(OPT_MASK);
			addr = LT(1);
			match(STR);
			
					    def.setType(RouteSrcDef.TYPE_MASK); 
					    def.setSrc(addr.getText());
					
			}
			break;
		}
		default:
		{
			throw new NoViableAltException(LT(1), getFilename());
		}
		}
		}
		{
		
				    cmd.addSrcDef(def);	
				
		}
	}
	
	public final void dstdef(
		RouteGenCommand cmd
	) throws RecognitionException, TokenStreamException {
		
		Token  addr = null;
		// Special command required !!!
		RouteDstDef def = new RouteDstDef();
		
		
		{
		switch ( LA(1)) {
		case OPT_SUBJ:
		{
			{
			match(OPT_SUBJ);
			
					    def.setType(RouteDstDef.TYPE_SUBJECT);
					    def.setDst(getnameid("Subject name"));
					
			}
			break;
		}
		case OPT_MASK:
		{
			{
			match(OPT_MASK);
			addr = LT(1);
			match(STR);
			
					    def.setType(RouteDstDef.TYPE_MASK); 
					    def.setDst(addr.getText());
					
			}
			break;
		}
		default:
		{
			throw new NoViableAltException(LT(1), getFilename());
		}
		}
		}
		{
		
				    def.setSmeId(getnameid("SME System id"));
				    cmd.addDstDef(def);
				
		}
	}
	
	public final void dstdef_x(
		RouteGenCommand cmd
	) throws RecognitionException, TokenStreamException {
		
		Token  addr = null;
		// Special command required !!!
		RouteDstDef def = new RouteDstDef();
		
		
		{
		switch ( LA(1)) {
		case OPT_SUBJ:
		{
			{
			match(OPT_SUBJ);
			
					    def.setType(RouteDstDef.TYPE_SUBJECT);
					    def.setDst(getnameid("Subject name"));
					
			}
			break;
		}
		case OPT_MASK:
		{
			{
			match(OPT_MASK);
			addr = LT(1);
			match(STR);
			
					    def.setType(RouteDstDef.TYPE_MASK); 
					    def.setDst(addr.getText());
					
			}
			break;
		}
		default:
		{
			throw new NoViableAltException(LT(1), getFilename());
		}
		}
		}
		{
		
				    cmd.addDstDef(def);
				
		}
	}
	
	public final void route_src(
		RouteGenCommand cmd
	) throws RecognitionException, TokenStreamException {
		
		
		try {      // for error handling
			{
			match(OPT_SRC);
			{
			int _cnt28=0;
			_loop28:
			do {
				if ((LA(1)==OPT_MASK||LA(1)==OPT_SUBJ)) {
					srcdef(cmd);
				}
				else {
					if ( _cnt28>=1 ) { break _loop28; } else {throw new NoViableAltException(LT(1), getFilename());}
				}
				
				_cnt28++;
			} while (true);
			}
			}
		}
		catch (RecognitionException ex) {
			
			throw new RecognitionException("Route srcdef missed or invalid. Syntax: src (subj <subject_name>|mask <mask>)+");
				
		}
	}
	
	public final void route_dst(
		RouteGenCommand cmd
	) throws RecognitionException, TokenStreamException {
		
		
		try {      // for error handling
			{
			match(OPT_DST);
			{
			int _cnt32=0;
			_loop32:
			do {
				if ((LA(1)==OPT_MASK||LA(1)==OPT_SUBJ)) {
					dstdef(cmd);
				}
				else {
					if ( _cnt32>=1 ) { break _loop32; } else {throw new NoViableAltException(LT(1), getFilename());}
				}
				
				_cnt32++;
			} while (true);
			}
			}
		}
		catch (RecognitionException ex) {
			
			throw new RecognitionException("Route dstdef missed or invalid. Syntax: dst (subj <subject_name>|mask <mask> <systemid>)+");
				
		}
	}
	
	public final void route_dst_x(
		RouteGenCommand cmd
	) throws RecognitionException, TokenStreamException {
		
		
		try {      // for error handling
			{
			match(OPT_DST);
			{
			int _cnt36=0;
			_loop36:
			do {
				if ((LA(1)==OPT_MASK||LA(1)==OPT_SUBJ)) {
					dstdef_x(cmd);
				}
				else {
					if ( _cnt36>=1 ) { break _loop36; } else {throw new NoViableAltException(LT(1), getFilename());}
				}
				
				_cnt36++;
			} while (true);
			}
			}
		}
		catch (RecognitionException ex) {
			
			throw new RecognitionException("Route dstdef missed or invalid. Syntax: dst (subj <subject_name>|mask <mask>)+");
				
		}
	}
	
	public final void addroute_flags(
		RouteAddCommand cmd
	) throws RecognitionException, TokenStreamException {
		
		
		try {      // for error handling
			{
			switch ( LA(1)) {
			case OPT_BILL:
			{
				match(OPT_BILL);
				cmd.setBill(true);
				break;
			}
			case OPT_NOBILL:
			{
				match(OPT_NOBILL);
				cmd.setBill(false);
				break;
			}
			default:
			{
				throw new NoViableAltException(LT(1), getFilename());
			}
			}
			}
			{
			switch ( LA(1)) {
			case OPT_ARCH:
			{
				match(OPT_ARCH);
				cmd.setArc(true);
				break;
			}
			case OPT_NOARCH:
			{
				match(OPT_NOARCH);
				cmd.setArc(false);
				break;
			}
			default:
			{
				throw new NoViableAltException(LT(1), getFilename());
			}
			}
			}
			{
			switch ( LA(1)) {
			case OPT_ALLOW:
			{
				match(OPT_ALLOW);
				cmd.setAllow(true);
				break;
			}
			case OPT_DENY:
			{
				match(OPT_DENY);
				cmd.setAllow(false);
				break;
			}
			default:
			{
				throw new NoViableAltException(LT(1), getFilename());
			}
			}
			}
		}
		catch (RecognitionException ex) {
			
			throw new RecognitionException("Route flags expected. Syntax: (bill|nobill) (arc|noarc) (allow|deny)");
				
		}
	}
	
	public final void altroute_flags(
		RouteAlterCommand cmd
	) throws RecognitionException, TokenStreamException {
		
		
		try {      // for error handling
			{
			switch ( LA(1)) {
			case OPT_BILL:
			{
				match(OPT_BILL);
				cmd.setBill(true);
				break;
			}
			case OPT_NOBILL:
			{
				match(OPT_NOBILL);
				cmd.setBill(false);
				break;
			}
			case EOF:
			case ACT_ADD:
			case ACT_DELETE:
			case OPT_ARCH:
			case OPT_NOARCH:
			case OPT_ALLOW:
			case OPT_DENY:
			case OPT_SVCID:
			case OPT_PRI:
			{
				break;
			}
			default:
			{
				throw new NoViableAltException(LT(1), getFilename());
			}
			}
			}
			{
			switch ( LA(1)) {
			case OPT_ARCH:
			{
				match(OPT_ARCH);
				cmd.setArc(true);
				break;
			}
			case OPT_NOARCH:
			{
				match(OPT_NOARCH);
				cmd.setArc(false);
				break;
			}
			case EOF:
			case ACT_ADD:
			case ACT_DELETE:
			case OPT_ALLOW:
			case OPT_DENY:
			case OPT_SVCID:
			case OPT_PRI:
			{
				break;
			}
			default:
			{
				throw new NoViableAltException(LT(1), getFilename());
			}
			}
			}
			{
			switch ( LA(1)) {
			case OPT_ALLOW:
			{
				match(OPT_ALLOW);
				cmd.setAllow(true);
				break;
			}
			case OPT_DENY:
			{
				match(OPT_DENY);
				cmd.setAllow(false);
				break;
			}
			case EOF:
			case ACT_ADD:
			case ACT_DELETE:
			case OPT_SVCID:
			case OPT_PRI:
			{
				break;
			}
			default:
			{
				throw new NoViableAltException(LT(1), getFilename());
			}
			}
			}
		}
		catch (RecognitionException ex) {
			
			throw new RecognitionException("Route flags expected. Syntax: [bill|nobill] [arc|noarc] [allow|deny]");
				
		}
	}
	
	public final void addsubj_mask(
		SubjectGenCommand cmd
	) throws RecognitionException, TokenStreamException {
		
		Token  mask = null;
		
		{
		mask = LT(1);
		match(STR);
		cmd.addMask(mask.getText());
		}
	}
	
	public final void addsubj_masks(
		SubjectGenCommand cmd
	) throws RecognitionException, TokenStreamException {
		
		
		try {      // for error handling
			{
			addsubj_mask(cmd);
			{
			_loop79:
			do {
				if ((LA(1)==COMMA)) {
					match(COMMA);
					addsubj_mask(cmd);
				}
				else {
					break _loop79;
				}
				
			} while (true);
			}
			}
		}
		catch (RecognitionException ex) {
			
				    throw new RecognitionException("Subject mask list missed or invalid. Syntax: <subject_mask>(,<subject_mask>)*");
				
		}
	}
	
	public final ProfileDeleteCommand  delprofile() throws RecognitionException, TokenStreamException {
		ProfileDeleteCommand cmd;
		
		Token  mask = null;
		
		cmd = new ProfileDeleteCommand();
		
		
		{
		try { // for error handling
			mask = LT(1);
			match(STR);
		}
		catch (RecognitionException ex) {
			
				    throw new RecognitionException("Profile mask expected");
				
		}
		cmd.setMask(mask.getText());
		}
		return cmd;
	}
	
	
	public static final String[] _tokenNames = {
		"<0>",
		"EOF",
		"<2>",
		"NULL_TREE_LOOKAHEAD",
		"\"add\"",
		"\"alter\"",
		"\"delete\"",
		"\"list\"",
		"\"view\"",
		"\"apply\"",
		"\"alias\"",
		"\"route\"",
		"\"profile\"",
		"\"subject\"",
		"\"hide\"",
		"\"nohide\"",
		"\"bill\"",
		"\"nobill\"",
		"\"arc\"",
		"\"noarc\"",
		"\"allow\"",
		"\"deny\"",
		"\"src\"",
		"\"dst\"",
		"\"mask\"",
		"\"subj\"",
		"\"serviceid\"",
		"\"priority\"",
		"\"defaultsme\"",
		"\"report\"",
		"\"full\"",
		"\"none\"",
		"\"encoding\"",
		"\"default\"",
		"\"ucs2\"",
		"WS",
		"more input",
		"quoted string",
		"STR_WS",
		"STR_CHR",
		"comma character ','",
		"ESC",
		"DIGIT"
	};
	
	
	}
