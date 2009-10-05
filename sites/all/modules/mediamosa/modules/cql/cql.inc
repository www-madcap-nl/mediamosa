<?php
// $Id$

/**
 * Copyright (c) 2008 Madcap BV (http://www.madcap.nl)
 * All rights reserved.
 *
 * Permission is granted for use, copying, modification, distribution,
 * and distribution of modified versions of this work as long as the
 * above copyright notice is included.
 */

/**
 * cql               ::= cqlQuery ['sortby' sortSpec]
 * cqlQuery          ::= prefixAssignment cqlQuery | scopedClause
 * prefixAssignment  ::= '>' prefix '=' uri | '>' uri
 * scopedClause  	   ::= scopedClause booleanGroup searchClause | searchClause
 * booleanGroup  	   ::= boolean [modifierList]
 * boolean           ::= 'and' | 'or' | 'not' | 'prox'
 * searchClause   	 ::= '(' cqlQuery ')' | index relation searchTerm | searchTerm
 * relation          ::= comparitor [modifierList]
 * comparitor     	 ::= comparitorSymbol | namedComparitor
 * comparitorSymbol  ::= '=' | '>' | '<' | '>=' | '<=' | '<>' | '=='
 * namedComparitor   ::= identifier
 * modifierList      ::= modifierList modifier | modifier
 * modifier          ::= '/' modifierName [comparitorSymbol modifierValue]
 * sortSpec  	       ::= sortSpec singleSpec | singleSpec
 * singleSpec  	     ::= index [modifierList]
 * refix, uri,
 * modifierName,
 * modifierValue,
 * searchTerm, index ::= term
 * term              ::= identifier | 'and' | 'or' | 'not' | 'prox' | 'sortby'
 * identifier        ::= charString1 | charString2
 * charString1  	    := Any sequence of characters that does not include any of the following:
 *                       whitespace
 *                       ( (open parenthesis)
 *                       ) (close parenthesis)
 *                       =
 *                       <
 *                       >
 *                       '"' (double quote)
 *                       /
 * If the final sequence is a reserved word, that token is returned instead. Note that '.' (period)
 * may be included, and a sequence of digits is also permitted. Reserved words are 'and', 'or',
 * 'not', and 'prox' (case insensitive). When a reserved word is used in a search term,
 * case is preserved.
 *
 * charString2        := Souble quotes enclosing a sequence of any characters except double quote
 *                       (unless preceded by backslash (\)). Backslash escapes the character
 *                       following it. The resultant value includes all backslash characters except
 *                       those releasing a double quote (this allows other systems to interpret the
 *                       backslash character). The surrounding double quotes are not included.
 *
 *
 *
 * [[index] relation[/modifier]] [term] [sortBy index[/modifier]]
 */

/**
 * @file
 * Contains all non-drupal functions
 */

/**
 * Key defines for the context array
 *
 */
define('CQL_CONTEXT_KEY_INDEXES', 'indexes');// CQL index
define('CQL_CONTEXT_KEY_COLUMNS', 'columns');// SQL column
define('CQL_CONTEXT_KEY_INDEX_REFERENCES', 'index_references'); // List of index => index, multiple indexes can be references
define('CQL_CONTEXT_KEY_DEFAULT_RELATION', 'default_relation');// required
define('CQL_CONTEXT_KEY_DEFAULT_SEARCH_TERMS', 'default_search_terms');// required

/**
 * Includes we need
 */
require_once('cql_class.inc');
require_once('cql_context_class.inc');
require_once('cql_exceptions.inc');
require_once('cql_part_class.inc');
require_once('cql_part_group_class.inc');
require_once('cql_prefix_assignment_class.inc');

define('CQL_BOOLEAN_OR', 'OR');
define('CQL_BOOLEAN_AND', 'AND');
define('CQL_BOOLEAN_NOT', 'NOT');
define('CQL_BOOLEAN_PROX', 'PROX');
