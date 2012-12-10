/*************************************************************************																		** 			University of Utah - Jim Schimpf Std Support Stuff			**					Nu-Metrics - OBJECT CODE SUPPORT					**  					Copyright 1987 University of Utah					**					Copyright 1995 Nu-Metrics, Inc.						**					Copyright 1998 Jim Schimpf							**  																		**					MAC FILE LEXICAL ANALYZER FOR C						**																		**		Module:CcLexFile.cp												**																		**	Version		Date	Person  	Description							**-----------------------------------------------------------------------**	0.1		 12-Feb-88	J.Schimpf	Initial Version						**	0.2		29-Jul-91	J.Schimpf	Add rewind function					**	0.3		 3-Aug-91	J.Schimpf	Add ftell,fseek & rewind			**	0.4		31-Dec-92	J.Schimpf	Convert for THINK C 5.0				**	0.5		 1-Jan-93	J.Schimpf	Don't return file name in input		**	0.6		16-Feb-93	J.Schimpf	Add fprintf							**	0.7		16-Feb-93	J.Schimpf	Put in correction for FTELL			**	0.8		12-Mar-93	J.Schimpf	Accept \n as line terminator in 	**									FGETS								**	0.9		 8-Aug-95	J.Schimpf	Convert to object					**	1.0		10-Aug-95	J.Schimpf	Allow user control of type,signature**									file type lists						**	1.1		22-Sep-95	J.Schimpf	Convert to file tokenizer			**	1.2		28-Dec-95	J.Schimpf	Add QUOTE mark for tokens			**	1.3		18-Aug-96	J.Schimpf	Remove inherited					**	1.4		22-Jan-97	J.Schimpf	Fix exit of " loop that would		**									leave an extra char in token		**	1.5		27-Sep-98	J.Schimpf	Remove COMMENT rtn and return EOL	**	1.6		27-Sep-98	J.Schimpf	Convert to Lexical analyzer			**	1.7		27-Sep-98	J.Schimpf	Convert to C Lexical analyzer		**	1.8		28-Dec-98	J.Schimpf	Add specifics to LUCENT use			**									test class and name					**	1.9		28-Dec-98	J.Schimpf	Add Lucent values for DDCMP			**	2.0		10-Jan-99	J.Schimpf	Add DEFAULT value 					**	2.1		28-Jan-99	J.Schimpf	Add DEVTYPE (RMU/CMU)				**	2.2		 1-Feb-99	J.Schimpf	Add STRING/LSTRING support			**	2.3		20-May-99	J.Schimpf	Add packed/unpacked types			**																		**	DESCRIPTION:													    **		Converts file to a stream of tokens								**		TOKEN := [<WS chars>]<Contigious characters = TOKEN>[<WS>]		**				or <QUOTE><charcters with embedded spaces><QUOTE>		**				 Maximum # characters in token in MAX_TOK				**		Characters in a line following <COMMENT_CHAR> are ignored to EOL**		NOTE: Files are treated as lines and TOKENS cannot be extend	**		over line breaks												**																		**	Methods:															**																		**	Name					Description 								**-----------------------------------------------------------------------**																		*************************************************************************//******* INCLUDE FILES ******/#include "CcLexFile.h"#include <ctype.h>// *** C TOKEN LIST ***LEX_TOKEN ctlist[] = {		C_TOK_CHAR,			"char",		C_TOK_STRING,		"string",		C_TOK_LSTRING,		"lstring",		C_TOK_SHORT,		"short",		C_TOK_INT,			"int",		C_TOK_LONG,			"long",		C_TOK_UINT32,		"UNIT32",		C_TOK_INT32,		"INT32",		C_TOK_FLOAT,		"float",		C_TOK_UNSIGNED,		"unsigned",		C_TOK_STRUCT,		"struct",		C_TOK_OPEN_BRACE,	"{",		C_TOK_CLOSE_BRACE,	"}",		C_TOK_OPEN_BRACKET,	"[",		C_TOK_CLOSE_BRACKET,"]",		C_TOK_NAME,			"*",		// Nothing here now		C_TOK_DEF,			":",		// DEFAULT value		C_TOK_LINE_CMT,		"\/\/",		C_TOK_CMT_OPEN,		"\/\*",		C_TOK_CMT_CLOSE,	"\*\/",				C_TOK_TYPE,			"#type",		C_TOK_CLASS,		"#class",		C_TOK_CNAME,		"#cname",		C_TOK_TEST,			"#test",		C_TOK_TNAME,		"#tname",		C_TOK_DEVTYPE,		"#devtype",		C_TOK_PACKED,		"#packed",};const int SIZE_CTLIST = sizeof( ctlist ) / sizeof( LEX_TOKEN );const int NO_PENDING = -1; /*************************************************************************  init()	- Initializer for token run**	INPUT:	cnt		- # Lexical tokens to be found*			tlist	- List of tokens**	OUTPUT:	NONE*			Initialize all internal variables to default states************************************************************************/void CcLexFile::init(void){	// Init Lexical analyzer for this run	CLexFile::init( SIZE_CTLIST,ctlist );	// initialize current token stuff	tok_psn = NO_PENDING;}	/************************************************************************		TOKEN FILE I/O OPERATIONS************************************************************************//************************************************************************* bool get_token( char *val )	- Get the next token from the*												  file**	INPUT:	val	- Returned token, NOTE: Must have TOK_MAX+1 spaces**	OUTPUT:	token type************************************************************************/int CcLexFile::get_token( char *token ){	int type;	// (1) Do we have something pending ?	if( tok_psn != NO_PENDING )	{		type = this->get_pending_token( token );		if( type != C_TOK_NOISE )			return( type );	}	// (2) Ok, we have nothing pending so get the next thing	type = CLexFile::get_token( token );	if( type != LEX_BUF_TOK )		return( type );	// (3) We have a name so set it into the pending slot and	// re-extract what is left	tok_psn = 0;	strcpy(cur_token,token );	type = this->get_pending_token( token );	return( type );}/************************************************************************* bool get_char(  )		- Get next character from pending token**	INPUT:	NONE**	OUTPUT:	NEXT character from pending token or TOK_EOL if no pending************************************************************************/int CcLexFile::get_char(  ){	int ch;		// If nothing pending then return EOL	if( tok_psn == NO_PENDING )		return( TOK_BUF_EOL );		// Ok we have something so return it		ch = cur_token[tok_psn++];	if( iscntrl( ch ) )	{		tok_psn = NO_PENDING;		return( TOK_BUF_EOL );	}	else		return( ch );}/************************************************************************* bool unget_char(   )	- Unget last character from pending buffer**	INPUT:	NONE**	OUTPUT:	NONE*			Back up pending buffer pointer************************************************************************/void CcLexFile::unget_char(  ){	if( tok_psn != NO_PENDING )	{		tok_psn--;		if( tok_psn < 0 )			tok_psn = 0;	}}/****************** INTERNAL PRIVATE OPERATIONS ************************//************************************************************************* bool get_pending_token( char *val )	- Get the next token from the*												  pending token**	INPUT:	val	- Returned token, NOTE: Must have TOK_MAX+1 spaces**	OUTPUT:	Token type************************************************************************/int CcLexFile::get_pending_token( char *token ){	int type;	int i;	// (1) Scan through and stop on first non-alpha thing	for( i=0; i< MAX_C_TOKEN; i++ )	{		if( isalnum(cur_token[tok_psn]) || cur_token[tok_psn] == '_' )		{			token[i] = cur_token[tok_psn++];		}		else			break;			// Quit on NON-ALPHA	}	// (2) If i == 0 then we found a non-alpha on first pass and	// we are dealing with that	if( i != 0 )	{		token[i] = '\0';		// Terminate the string and		type = C_TOK_NAME;		// return that it's a ID			}	else	{		// (3) We have hit a punct mark so determine which is was and return		// that		token[0] = cur_token[tok_psn++];		token[1] = '\0';		switch( token[0] ) {			case '[':		type = C_TOK_OPEN_BRACKET;							break;			case ']':		type = C_TOK_CLOSE_BRACKET;							break;			case '{':		type = C_TOK_OPEN_BRACE;							break;			case '}':		type = C_TOK_CLOSE_BRACE;							break;			case ':':		type = C_TOK_DEF;							break;			case ';':		type = C_TOK_NOISE;							break;		}	}	// (4) Now advance pointer, if there is no more pending left say so	// in all cases return result	if( iscntrl( cur_token[tok_psn] ) )		tok_psn = NO_PENDING;	return( type );}