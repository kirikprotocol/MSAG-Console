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
		case ACT_SHOW:
		{
			match(ACT_SHOW);
			cmd=show();
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
		case TGT_PRINCIPAL:
		{
			match(TGT_PRINCIPAL);
			cmd=addprincipal();
			break;
		}
		case TGT_DL:
		{
			match(TGT_DL);
			cmd=adddl();
			break;
		}
		case TGT_DLSUB:
		{
			match(TGT_DLSUB);
			cmd=adddlsubmitter();
			break;
		}
		case TGT_DLMEM:
		{
			match(TGT_DLMEM);
			cmd=adddlmember();
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
		case TGT_PRINCIPAL:
		{
			match(TGT_PRINCIPAL);
			cmd=delprincipal();
			break;
		}
		case TGT_DL:
		{
			match(TGT_DL);
			cmd=deldl();
			break;
		}
		case TGT_DLSUB:
		{
			match(TGT_DLSUB);
			cmd=deldlsubmitter();
			break;
		}
		case TGT_DLMEM:
		{
			match(TGT_DLMEM);
			cmd=deldlmember();
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
		case TGT_PRINCIPAL:
		{
			match(TGT_PRINCIPAL);
			cmd=altprincipal();
			break;
		}
		case TGT_DL:
		{
			match(TGT_DL);
			cmd=altdl();
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
		case TGT_PRINCIPAL:
		{
			match(TGT_PRINCIPAL);
			cmd = new PrincipalListCommand();
			break;
		}
		case TGT_DL:
		{
			match(TGT_DL);
			cmd = new DistributionListListCommand();
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
		case TGT_PRINCIPAL:
		{
			match(TGT_PRINCIPAL);
			cmd=viewprincipal();
			break;
		}
		case TGT_DL:
		{
			match(TGT_DL);
			cmd=viewdl();
			break;
		}
		default:
		{
			throw new NoViableAltException(LT(1), getFilename());
		}
		}
		return cmd;
	}
	
	public final AliasShowCommand  show() throws RecognitionException, TokenStreamException {
		AliasShowCommand cmd;
		
		Token  addr = null;
		
		cmd = null;
		
		
		match(TGT_ALIAS);
		{
		try { // for error handling
			addr = LT(1);
			match(STR);
		}
		catch (RecognitionException ex) {
			
				    throw new RecognitionException("Target address for aliases expected");
				
		}
		
				    cmd = new AliasShowCommand();
				    cmd.setAddress(addr.getText());   
				
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
		route_dst(cmd, true);
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
	
	public final PrincipalAddCommand  addprincipal() throws RecognitionException, TokenStreamException {
		PrincipalAddCommand cmd;
		
		Token  addr = null;
		Token  numl = null;
		Token  nume = null;
		
		cmd = new PrincipalAddCommand();
		
		
		try {      // for error handling
			{
			try { // for error handling
				addr = LT(1);
				match(STR);
			}
			catch (RecognitionException ex) {
				
					    throw new RecognitionException("Principal address expected");
					
			}
			cmd.setAddress(addr.getText());
			}
			{
			match(OPT_NLIST);
			try { // for error handling
				numl = LT(1);
				match(STR);
			}
			catch (RecognitionException ex) {
				
					    throw new RecognitionException("'numlist' integer value expected");
					
			}
			
					    try {
						cmd.setMaxLists(Integer.parseInt(numl.getText()));
					    } catch (NumberFormatException ex) {
						throw new NumberFormatException("Expecting integer value for <numlist>");
					    }
					
			}
			{
			match(OPT_NELEM);
			try { // for error handling
				nume = LT(1);
				match(STR);
			}
			catch (RecognitionException ex) {
				
					    throw new RecognitionException("'numelem' integer value expected");
					
			}
			
					    try {
						cmd.setMaxElements(Integer.parseInt(nume.getText()));
					    } catch (NumberFormatException ex) {
						throw new NumberFormatException("Expecting integer value for <numelem>");
					    }
					
			}
		}
		catch (RecognitionException ex) {
			
				    throw new RecognitionException("Command invalid. Syntax: add principal <principal_address> numlist <number> numelem <number>");
				
		}
		return cmd;
	}
	
	public final DistributionListAddCommand  adddl() throws RecognitionException, TokenStreamException {
		DistributionListAddCommand cmd;
		
		Token  nume = null;
		Token  owner = null;
		
		cmd = new DistributionListAddCommand();
		
		
		try {      // for error handling
			{
				
					    cmd.setName(getnameid("Distribution list name"));
					
			}
			{
			match(OPT_NELEM);
			try { // for error handling
				nume = LT(1);
				match(STR);
			}
			catch (RecognitionException ex) {
				
					    throw new RecognitionException("'numelem' integer value expected");
					
			}
			
					    try {
						cmd.setMaxElements(Integer.parseInt(nume.getText()));
					    } catch (NumberFormatException ex) {
						throw new NumberFormatException("Expecting integer value for <numelem>");
					    }
					
			}
			{
			switch ( LA(1)) {
			case OPT_OWNER:
			{
				match(OPT_OWNER);
				try { // for error handling
					owner = LT(1);
					match(STR);
				}
				catch (RecognitionException ex) {
					
						    throw new RecognitionException("Owner address expected");
						
				}
				
						    cmd.setOwner(owner.getText());
						
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
		}
		catch (RecognitionException ex) {
			
				    throw new RecognitionException("Command invalid. Syntax: add dl <dl_name> [owner <owner_address>] numelem <number>");
				
		}
		return cmd;
	}
	
	public final SubmitterAddCommand  adddlsubmitter() throws RecognitionException, TokenStreamException {
		SubmitterAddCommand cmd;
		
		Token  submitter = null;
		
		cmd = new SubmitterAddCommand();
		
		
		{
			
				    cmd.setName(getnameid("Distribution list name"));
				
		}
		{
		try { // for error handling
			submitter = LT(1);
			match(STR);
		}
		catch (RecognitionException ex) {
			
				    throw new RecognitionException("Submitter address expected");
				
		}
		
				    cmd.setSubmitter(submitter.getText());
				
		}
		return cmd;
	}
	
	public final MemberAddCommand  adddlmember() throws RecognitionException, TokenStreamException {
		MemberAddCommand cmd;
		
		Token  member = null;
		
		cmd = new MemberAddCommand();
		
		
		{
			
				    cmd.setName(getnameid("Distribution list name"));
				
		}
		{
		try { // for error handling
			member = LT(1);
			match(STR);
		}
		catch (RecognitionException ex) {
			
				    throw new RecognitionException("Member address expected");
				
		}
		
				    cmd.setMember(member.getText());
				
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
	
	public final PrincipalDeleteCommand  delprincipal() throws RecognitionException, TokenStreamException {
		PrincipalDeleteCommand cmd;
		
		Token  addr = null;
		
		cmd = new PrincipalDeleteCommand();
		
		
		{
		try { // for error handling
			addr = LT(1);
			match(STR);
		}
		catch (RecognitionException ex) {
			
				    throw new RecognitionException("Principal address expected");
				
		}
		cmd.setAddress(addr.getText());
		}
		return cmd;
	}
	
	public final DistributionListDeleteCommand  deldl() throws RecognitionException, TokenStreamException {
		DistributionListDeleteCommand cmd;
		
		
		cmd = new DistributionListDeleteCommand();
		
		
		{
			
				    cmd.setName(getnameid("Distribution list name"));
				
		}
		return cmd;
	}
	
	public final SubmitterDeleteCommand  deldlsubmitter() throws RecognitionException, TokenStreamException {
		SubmitterDeleteCommand cmd;
		
		Token  submitter = null;
		
		cmd = new SubmitterDeleteCommand();
		
		
		{
			
				    cmd.setName(getnameid("Distribution list name"));
				
		}
		{
		try { // for error handling
			submitter = LT(1);
			match(STR);
		}
		catch (RecognitionException ex) {
			
				    throw new RecognitionException("Submitter address expected");
				
		}
		
				    cmd.setSubmitter(submitter.getText());
				
		}
		return cmd;
	}
	
	public final MemberDeleteCommand  deldlmember() throws RecognitionException, TokenStreamException {
		MemberDeleteCommand cmd;
		
		Token  member = null;
		
		cmd = new MemberDeleteCommand();
		
		
		{
			
				    cmd.setName(getnameid("Distribution list name"));
				
		}
		{
		try { // for error handling
			member = LT(1);
			match(STR);
		}
		catch (RecognitionException ex) {
			
				    throw new RecognitionException("Member address expected");
				
		}
		
				    cmd.setMember(member.getText());
				
		}
		return cmd;
	}
	
	public final RouteAlterCommand  altroute() throws RecognitionException, TokenStreamException {
		RouteAlterCommand cmd;
		
		Token  num = null;
		Token  pri = null;
		
		cmd = new RouteAlterCommand();
		boolean addAction = true;
		
		
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
		case ACT_DELETE:
		{
			{
			switch ( LA(1)) {
			case ACT_ADD:
			{
				{
				match(ACT_ADD);
				cmd.setAction(RouteAlterCommand.ACTION_ADD); addAction=true;
				}
				break;
			}
			case ACT_DELETE:
			{
				{
				match(ACT_DELETE);
				cmd.setAction(RouteAlterCommand.ACTION_DEL); addAction=false;
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
			switch ( LA(1)) {
			case OPT_SRC:
			{
				route_src(cmd);
				
							cmd.setTarget(RouteAlterCommand.TARGET_SRC);
						
				break;
			}
			case OPT_DST:
			{
				route_dst(cmd, addAction);
				
							cmd.setTarget(RouteAlterCommand.TARGET_DST); 
						
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
		switch ( LA(1)) {
		case OPT_REPORT:
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
			break;
		}
		case EOF:
		case OPT_ENCODE:
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
	
	public final PrincipalAlterCommand  altprincipal() throws RecognitionException, TokenStreamException {
		PrincipalAlterCommand cmd;
		
		Token  addr = null;
		Token  numl = null;
		Token  nume = null;
		
		cmd = new PrincipalAlterCommand();
		
		
		try {      // for error handling
			{
			try { // for error handling
				addr = LT(1);
				match(STR);
			}
			catch (RecognitionException ex) {
				
					    throw new RecognitionException("Principal address expected");
					
			}
			cmd.setAddress(addr.getText());
			}
			{
			switch ( LA(1)) {
			case OPT_NLIST:
			{
				match(OPT_NLIST);
				numl = LT(1);
				match(STR);
				
						    try {
							cmd.setMaxLists(Integer.parseInt(numl.getText()));
						    } catch (NumberFormatException ex) {
							throw new NumberFormatException("Expecting integer value for <numlist>");
						    }
						
				break;
			}
			case EOF:
			case OPT_NELEM:
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
			case OPT_NELEM:
			{
				match(OPT_NELEM);
				nume = LT(1);
				match(STR);
				
						    try {
							cmd.setMaxElements(Integer.parseInt(nume.getText()));
						    } catch (NumberFormatException ex) {
							throw new NumberFormatException("Expecting integer value for <numelem>");
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
		}
		catch (RecognitionException ex) {
			
				    throw new RecognitionException("Command invalid. Syntax: alter principal <principal_address> [numlist <number>] [numelem <number>]");
				
		}
		return cmd;
	}
	
	public final DistributionListAlterCommand  altdl() throws RecognitionException, TokenStreamException {
		DistributionListAlterCommand cmd;
		
		Token  nume = null;
		
		cmd = new DistributionListAlterCommand();
		
		
		{
			
				    cmd.setName(getnameid("Distribution list name"));
				
		}
		{
		match(OPT_NELEM);
		try { // for error handling
			nume = LT(1);
			match(STR);
		}
		catch (RecognitionException ex) {
			
				    throw new RecognitionException("'numelem' integer value expected");
				
		}
		
				    try {
					cmd.setMaxElements(Integer.parseInt(nume.getText()));
				    } catch (NumberFormatException ex) {
					throw new NumberFormatException("Expecting integer value for <numelem>");
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
	
	public final PrincipalViewCommand  viewprincipal() throws RecognitionException, TokenStreamException {
		PrincipalViewCommand cmd;
		
		Token  addr = null;
		
		cmd = new PrincipalViewCommand();
		
		
		{
		try { // for error handling
			addr = LT(1);
			match(STR);
		}
		catch (RecognitionException ex) {
			
				    throw new RecognitionException("Principal address expected");
				
		}
		cmd.setAddress(addr.getText());
		}
		return cmd;
	}
	
	public final DistributionListViewCommand  viewdl() throws RecognitionException, TokenStreamException {
		DistributionListViewCommand cmd;
		
		
		cmd = new DistributionListViewCommand();
		
		
		{
			
				    cmd.setName(getnameid("Distribution list name"));
				
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
		RouteGenCommand cmd, boolean needSmeId
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
		
				    if (needSmeId) def.setSmeId(getnameid("SME System id"));
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
			int _cnt25=0;
			_loop25:
			do {
				if ((LA(1)==OPT_MASK||LA(1)==OPT_SUBJ)) {
					srcdef(cmd);
				}
				else {
					if ( _cnt25>=1 ) { break _loop25; } else {throw new NoViableAltException(LT(1), getFilename());}
				}
				
				_cnt25++;
			} while (true);
			}
			}
		}
		catch (RecognitionException ex) {
			
			throw new RecognitionException(
				    "Route srcdef missed or invalid. Syntax: src (subj <subject_name>|mask <mask>)+");
				
		}
	}
	
	public final void route_dst(
		RouteGenCommand cmd, boolean needSmeId
	) throws RecognitionException, TokenStreamException {
		
		
		try {      // for error handling
			{
			match(OPT_DST);
			{
			int _cnt29=0;
			_loop29:
			do {
				if ((LA(1)==OPT_MASK||LA(1)==OPT_SUBJ)) {
					dstdef(cmd, needSmeId);
				}
				else {
					if ( _cnt29>=1 ) { break _loop29; } else {throw new NoViableAltException(LT(1), getFilename());}
				}
				
				_cnt29++;
			} while (true);
			}
			}
		}
		catch (RecognitionException ex) {
			
			throw new RecognitionException(
				    "Route dstdef missed or invalid. Syntax: dst (subj <subject_name>|mask <mask>"+
				    ((needSmeId) ? " <systemid>)+":")+"));
				
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
			{
			switch ( LA(1)) {
			case OPT_RCPT:
			{
				match(OPT_RCPT);
				cmd.setReceipt(true);
				break;
			}
			case OPT_NORCPT:
			{
				match(OPT_NORCPT);
				cmd.setReceipt(false);
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
			
			throw new RecognitionException("Route flags expected. Syntax: (bill|nobill) (arc|noarc) (allow|deny) (receipt|noreceipt)");
				
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
			case OPT_RCPT:
			case OPT_NORCPT:
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
			case OPT_RCPT:
			case OPT_NORCPT:
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
			case OPT_RCPT:
			case OPT_NORCPT:
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
			case OPT_RCPT:
			{
				match(OPT_RCPT);
				cmd.setReceipt(true);
				break;
			}
			case OPT_NORCPT:
			{
				match(OPT_NORCPT);
				cmd.setReceipt(false);
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
			
			throw new RecognitionException("Route flags expected. Syntax: [bill|nobill] [arc|noarc] [allow|deny] [receipt|noreceipt]");
				
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
			_loop74:
			do {
				if ((LA(1)==COMMA)) {
					match(COMMA);
					addsubj_mask(cmd);
				}
				else {
					break _loop74;
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
		"\"show\"",
		"\"alias\"",
		"\"route\"",
		"\"profile\"",
		"\"subject\"",
		"\"principal\"",
		"\"dl\"",
		"\"dlsubmitter\"",
		"\"dlmember\"",
		"\"hide\"",
		"\"nohide\"",
		"\"bill\"",
		"\"nobill\"",
		"\"arc\"",
		"\"noarc\"",
		"\"allow\"",
		"\"deny\"",
		"\"receipt\"",
		"\"noreceipt\"",
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
		"\"numlist\"",
		"\"numelem\"",
		"\"owner\"",
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
