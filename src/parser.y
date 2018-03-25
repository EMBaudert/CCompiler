/* 
 * parser.y - Parser utility for the DHBW compiler
 */
 
%{	
	// Project-specific includes
	#include "diag.h"
  #include "uthash.h"
  #include <string.h>

  typedef struct paramstruct {
    char* type;
  } STRUCTPARAM;
  typedef struct StackNode {
    char* data;
    struct StackNode* next;
  } STACK;
  typedef struct varstruct {
    char* id;
    char* type;
    char* size;
    char* value;
    UT_hash_handle hh;
  } STRUCTVAR;
  typedef struct funcstruct {
    char* id;
    char* type;
    STRUCTPARAM* funcparams;//array
    STRUCTPARAM returnparam;
    UT_hash_handle hh;
  } STRUCTFUNC;
  void find_var(char* id);
  void add_var(char *id, char *type, char *value);
  void log_struct();
  void message_logger(char* msg);
  STRUCTVAR *variables = NULL;
  STACK* programstack;
%}

%union {
  int i;
  char *id;
}
 
// Verbose error messages
%error-verbose

%locations
%start program

// Compiler in debugging information
%debug

// Enable debug output
%initial-action
{
	yydebug = 1;
};

/*
 * One shift/reduce conflict is expected for the "dangling-else" problem. This
 * conflict however is solved by the default behavior of bison for shift/reduce 
 * conflicts (shift action). The default behavior of bison corresponds to what
 * we want bison to do: SHIFT if the lookahead is 'ELSE' in order to bind the 'ELSE' to
 * the last open if-clause. 
 */
%expect 1

%token DO WHILE
%token IF ELSE
%token INT VOID
%token RETURN

%token COLON COMMA SEMICOLON

%token BRACE_OPEN BRACE_CLOSE
%token BRACKET_OPEN BRACKET_CLOSE
%token PARA_OPEN PARA_CLOSE

%token <id>ID
%token <i> NUM

%token ASSIGN 
%token LOGICAL_OR
%token LOGICAL_AND
%token EQ NE     
%token LS LSEQ GTEQ GT 
%token SHIFT_LEFT SHIFT_RIGHT
%token PLUS MINUS     
%token MUL DIV MOD
%token LOGICAL_NOT UNARY_MINUS UNARY_PLUS

%right ASSIGN
%left LOGICAL_AND LOGICAL_OR
%left LS GT LSEQ GTEQ EQ NE
%left SHIFT_LEFT SHIFT_RIGHT
%left PLUS MINUS
%left MUL DIV MOD
%left LOGICAL_NOT UNARY_MINUS UNARY_PLUS

%%

program
     : program_element_list
     ;

program_element_list
     : program_element_list program_element 
     | program_element 
     ;

program_element
     : variable_declaration SEMICOLON
     | function_declaration SEMICOLON
     | function_definition
     | SEMICOLON
     ;
									
type
     : INT
     | VOID
     ;

variable_declaration
     : variable_declaration COMMA identifier_declaration
     | type identifier_declaration {printf("Params : none");}
     ;

identifier_declaration
     : ID BRACKET_OPEN NUM BRACKET_CLOSE
     | ID {add_var($1,"int","0");log_struct();yylval.id = $1;find_var("test1");push_something();}
     ;

function_definition
     : type ID PARA_OPEN PARA_CLOSE BRACE_OPEN stmt_list BRACE_CLOSE
     | type ID PARA_OPEN function_parameter_list PARA_CLOSE BRACE_OPEN stmt_list BRACE_CLOSE
     ;

function_declaration
     : type ID PARA_OPEN PARA_CLOSE
     | type ID PARA_OPEN function_parameter_list PARA_CLOSE
     ;

function_parameter_list
     : function_parameter
     | function_parameter_list COMMA function_parameter
     ;
	
function_parameter
     : type identifier_declaration
     ;
									
stmt_list
     : /* empty: epsilon */
     | stmt_list stmt
     ;

stmt
     : stmt_block
     | variable_declaration SEMICOLON
     | expression SEMICOLON
     | stmt_conditional
     | stmt_loop
     | RETURN expression SEMICOLON
     | RETURN SEMICOLON
     | SEMICOLON /* empty statement */
     ;

stmt_block
     : BRACE_OPEN stmt_list BRACE_CLOSE
     ;
	
stmt_conditional
     : IF PARA_OPEN expression PARA_CLOSE stmt
     | IF PARA_OPEN expression PARA_CLOSE stmt ELSE stmt
     ;
									
stmt_loop
     : WHILE PARA_OPEN expression PARA_CLOSE stmt
     | DO stmt WHILE PARA_OPEN expression PARA_CLOSE SEMICOLON
     ;
									
expression
     : expression ASSIGN expression
     | expression LOGICAL_OR expression
     | expression LOGICAL_AND expression
     | LOGICAL_NOT expression
     | expression EQ expression
     | expression NE expression
     | expression LS expression 
     | expression LSEQ expression 
     | expression GTEQ expression 
     | expression GT expression
     | expression PLUS expression
     | expression MINUS expression
     | expression SHIFT_LEFT expression
     | expression SHIFT_RIGHT expression
     | expression MUL expression
     | expression DIV expression
     | MINUS expression %prec UNARY_MINUS
     | PLUS expression %prec UNARY_PLUS
     | ID BRACKET_OPEN primary BRACKET_CLOSE
     | PARA_OPEN expression PARA_CLOSE
     | function_call
     | primary
     ;

primary
     : NUM {printf("Parsed : %d", $1);}
     | ID {printf("Parsed : %s", $1);}
     ;

function_call
      : ID PARA_OPEN PARA_CLOSE
      | ID PARA_OPEN function_call_parameters PARA_CLOSE
      ;

function_call_parameters
     : function_call_parameters COMMA expression
     | expression
     ;

%%

void yyerror (const char *msg)
{
	FATAL_COMPILER_ERROR(INVALID_SYNTAX, 0, "(%d.%d-%d.%d): %s\n", yylloc.first_line, yylloc.first_column, yylloc.last_line, yylloc.last_column, msg);
}

/*int lookup_ID(char *id){
  return table.find(id);
}*/
 
const char* getString(){
  return "Hallo!";
}

void push_something(){
  printf("Try to push\n");
  push(&programstack,"test1");
  printf("Try to peek\n");
  char* temp = "Hallo Welt!";
  printf("temp is: %s\n",temp);
  temp = getString2();//peek(programstack);
  char* temp2 = &temp;
  printf("Got peek\n");
  printf("Stack: %s\n",*temp);
}

void add_var(char *id, char *type, char *value){
  STRUCTVAR *s;
  s = (STRUCTVAR*)malloc(sizeof(STRUCTVAR));
  s->id = (char*)malloc(sizeof(id));
  s->type = (char*)malloc(sizeof(type));
  s->value = (char*)malloc(sizeof(value));
  strcpy(s->id, id);
  strcpy(s->type, type);
  strcpy(s->value, value);
  HASH_ADD_INT(variables,id,s);
}

void find_var(char* var_id){
  STRUCTVAR *temp;
  for(temp = variables; temp!=NULL;temp=temp->hh.next){
    if(!strcmp(temp->id,var_id)){
      break;
    }
  }
  if(temp!=NULL)
    printf("Test: %s %s\n",temp->id,temp->type);
}

void log_struct(){
  STRUCTVAR *temp;
  for(temp = variables; temp!=NULL;temp=temp->hh.next){
    printf("Entry: id: %s, type: %s, value: %s\n",temp->id,temp->type,temp->value);
  }
}

void message_logger(char* msg){
  printf("Following Message is sent:\n(%d.%d-%d.%d): %s\n", yylloc.first_line, yylloc.first_column, yylloc.last_line, yylloc.last_column, msg);
}