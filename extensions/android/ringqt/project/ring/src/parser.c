/* Copyright (c) 2013-2024 Mahmoud Fayed <msfclipper@yahoo.com> */

#include "ring.h"

int ring_parser_start ( List *pTokens,RingState *pRingState )
{
	Parser *pParser  ;
	int nResult,RingActiveFile  ;
	pParser = ring_parser_new(pTokens,pRingState);
	/* Parse Tokens */
	ring_parser_nexttoken(pParser);
	do {
		nResult = ring_parser_class(pParser);
		if ( nResult == 0 ) {
			ring_parser_error(pParser,"");
			/* Important check to avoid missing the line number counter */
			if ( ring_parser_isendline(pParser) == 0 ) {
				/* Move next trying to avoid the error */
				ring_parser_nexttoken(pParser);
			}
		}
	} while (pParser->ActiveToken !=pParser->TokensCount)  ;
	/* Display Errors Count */
	RingActiveFile = ring_list_getsize(pParser->pRingState->pRingFilesStack);
	if ( pParser->nErrorsCount == 0 ) {
		ring_parser_delete(pParser);
		return 1 ;
	}
	else {
		printf( "\n%s errors count : %d \n",ring_list_getstring(pParser->pRingState->pRingFilesStack,RingActiveFile),pParser->nErrorsCount ) ;
	}
	ring_parser_delete(pParser);
	return 0 ;
}

Parser * ring_parser_new ( List *pTokens,RingState *pRingState )
{
	Parser *pParser  ;
	pParser = (Parser *) ring_state_malloc(pRingState,sizeof(Parser));
	/* Ring State */
	pParser->pRingState = pRingState ;
	pParser->Tokens = pTokens ;
	pParser->ActiveToken = 0 ;
	pParser->TokensCount = ring_list_getsize(pParser->Tokens) ;
	pParser->nTokenIndex = 0 ;
	pParser->nLineNumber = 1 ;
	pParser->nErrorLine = 0 ;
	pParser->nErrorsCount = 0 ;
	if ( pRingState->pRingGenCode == NULL ) {
		pRingState->pRingGenCode = ring_list_new(RING_ZERO);
		pRingState->pRingFunctionsMap = ring_list_new_gc(pRingState,RING_ZERO);
		pRingState->pRingClassesMap = ring_list_new_gc(pRingState,RING_ZERO);
		pRingState->pRingPackagesMap = ring_list_new_gc(pRingState,RING_ZERO);
	}
	pParser->GenCode = pRingState->pRingGenCode ;
	pParser->FunctionsMap = pRingState->pRingFunctionsMap ;
	pParser->ActiveGenCodeList = NULL ;
	pParser->nAssignmentFlag = 1 ;
	pParser->nClassStart = 0 ;
	pParser->ClassesMap = pRingState->pRingClassesMap ;
	pParser->PackagesMap = pRingState->pRingPackagesMap ;
	pParser->nClassMark = 0 ;
	pParser->nPrivateFlag = 0 ;
	pParser->nBraceFlag = 0 ;
	pParser->nInsertFlag = 0 ;
	pParser->nInsertCounter = 0 ;
	pParser->nNewObject = 0 ;
	pParser->nFuncCallOnly = 0 ;
	pParser->nControlStructureExpr = 0 ;
	pParser->nControlStructureBrace = 0 ;
	pParser->nThisOrSelfLoadA = 0 ;
	pParser->nThisLoadA = 0 ;
	pParser->nLoopOrExitCommand = 0 ;
	pParser->nCheckLoopAndExit = 1 ;
	pParser->nLoopFlag = 0 ;
	pParser->pForInVars = ring_list_new_gc(pRingState,RING_ZERO);
	return pParser ;
}

Parser * ring_parser_delete ( Parser *pParser )
{
	pParser->pForInVars = ring_list_delete_gc(pParser->pRingState,pParser->pForInVars);
	ring_state_free(pParser->pRingState,pParser);
	return NULL ;
}
/* Check Token */

void ring_parser_loadtoken ( Parser *pParser )
{
	List *pList  ;
	pList = ring_list_getlist(pParser->Tokens,pParser->ActiveToken);
	pParser->TokenType = ring_list_getint(pList,RING_SCANNER_TOKENTYPE) ;
	pParser->TokenText = ring_list_getstring(pList,RING_SCANNER_TOKENVALUE) ;
	pParser->nTokenIndex = ring_list_getint(pList,RING_SCANNER_TOKENINDEX) ;
}

int ring_parser_nexttoken ( Parser *pParser )
{
	if ( pParser->ActiveToken < pParser->TokensCount ) {
		pParser->ActiveToken++ ;
		ring_parser_loadtoken(pParser);
		return 1 ;
	}
	return 0 ;
}

int ring_parser_iskeyword ( Parser *pParser,SCANNER_KEYWORD x )
{
	if ( pParser->TokenType == SCANNER_TOKEN_KEYWORD ) {
		if ( ((unsigned int) atoi(pParser->TokenText)) == ((unsigned int) x) ) {
			return 1 ;
		}
	}
	return 0 ;
}

int ring_parser_isoperator ( Parser *pParser,const char *cStr )
{
	return (pParser->TokenType == SCANNER_TOKEN_OPERATOR) && (strcmp( pParser->TokenText,cStr) == 0 ) ;
}

int ring_parser_isliteral ( Parser *pParser )
{
	return (pParser->TokenType == SCANNER_TOKEN_LITERAL) ;
}

int ring_parser_isnumber ( Parser *pParser )
{
	return (pParser->TokenType ==SCANNER_TOKEN_NUMBER) ;
}

int ring_parser_isidentifier ( Parser *pParser )
{
	return (pParser->TokenType ==SCANNER_TOKEN_IDENTIFIER) ;
}

int ring_parser_isendline ( Parser *pParser )
{
	return (pParser->TokenType == SCANNER_TOKEN_ENDLINE) ;
}

int ring_parser_settoken ( Parser *pParser,int x )
{
	if ( (x >= 1) && (x <= pParser->TokensCount) ) {
		pParser->ActiveToken = x ;
		ring_parser_loadtoken(pParser);
		return 1 ;
	}
	return 0 ;
}

int ring_parser_isanykeyword ( Parser *pParser )
{
	return (pParser->TokenType == SCANNER_TOKEN_KEYWORD) ;
}

int ring_parser_isoperator2 ( Parser *pParser,SCANNER_OPERATOR nType )
{
	return (pParser->TokenType == SCANNER_TOKEN_OPERATOR) && ( pParser->nTokenIndex == (int) nType ) ;
}
/* Display Errors */

void ring_parser_error ( Parser *pParser,const char *cStr )
{
	int RingActiveFile  ;
	ring_state_cgiheader(pParser->pRingState);
	RingActiveFile = ring_list_getsize(pParser->pRingState->pRingFilesStack);
	if ( pParser->nErrorLine != pParser->nLineNumber ) {
		pParser->nErrorLine = pParser->nLineNumber ;
		printf( "\n%s Line (%d) ",ring_list_getstring(pParser->pRingState->pRingFilesStack,RingActiveFile),pParser->nLineNumber ) ;
		pParser->nErrorsCount++ ;
		if ( strcmp(cStr,"") != 0 ) {
			printf( "%s",cStr ) ;
		}
		else {
			printf( RING_PARSER_ERROR_SYNTAXERROR ) ;
		}
		return ;
	}
	else if ( strcmp(cStr,"") != 0 ) {
		pParser->nErrorsCount++ ;
	}
	if ( strcmp(cStr,"") != 0 ) {
		printf( "\n%s Line (%d) ",ring_list_getstring(pParser->pRingState->pRingFilesStack,RingActiveFile),pParser->nLineNumber ) ;
		printf( "%s",cStr ) ;
	}
}
