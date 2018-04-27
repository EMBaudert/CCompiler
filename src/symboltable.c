#include "symboltable.h"

#include "logger.h"
#include "stack.h"
#include "structs.h"
#include "checker.h"
#include <string.h>
#include "tempcode.h"

STACK* programstack;
STRUCTFUNC* functions = NULL;
STRUCTVAR* variables = NULL;
SCOPESTACK* scopes = NULL;
STRUCTPARAM* parameters = NULL;

void addFunc(char* id, char* type, int numberOfParams)
{
	if (!funcExists(id)&&!varExists(id,1))
	{
		STRUCTFUNC* s;
		s = (STRUCTFUNC*)malloc(sizeof(STRUCTFUNC));
		s->id = (char*)malloc(sizeof(id));
		s->type = (char*)malloc(sizeof(type));
		strcpy(s->id, id);
		strcpy(s->type, type);
		s->paramcount = numberOfParams;
		s->isDefined = 0;
		STRUCTPARAM* p = NULL;
		STRUCTPARAM* tempstruct;
		char* temptype;
		char* tempid;
		char* templength;
		while (numberOfParams > 0)
		{
			pop(&programstack, &templength);
			pop(&programstack, &temptype);
			pop(&programstack, &tempid);
			if (!strcmp(temptype,"VOID"))
			{
				errorLogger("Type-Error: Parameter \"",tempid,"\" can't be declared as VOID!\n");
			}
			else 
			{
				if (atoi(templength) > 1)
				{
					strcat(temptype, "-ARR");
				}
				tempstruct = p;
				p = (STRUCTPARAM*)malloc(sizeof(STRUCTPARAM));
				p->type = (char*)malloc(sizeof(temptype));
				p->name = (char*)malloc(sizeof(tempid));
				strcpy(p->type, temptype);
				strcpy(p->name, tempid);
				p->paramNr = numberOfParams;
				p->size = atoi(templength);
				p->next = tempstruct;
				numberOfParams--;
			}
			
		}
		parameters = p;
		s->funcparams = p;
		HASH_ADD_INT(functions, id, s);
		// log_funcs();
	}
	else
	{
		errorLogger("Name-Error: Identifier \"",id,"\" is already in use!\n");
	}
}
void identifierDeclaration(int length, char* type)
{
	if (!strcmp(type, "0"))
	{
		char* tempid;
		char* temptype;
		pop(&programstack, &tempid);
		if(!strcmp(tempid,"functionsReturnParameter")){
			errorLogger("Name-Error: Variable can't be called \"",tempid,"\" because this name is reserved for the compiler!\n");
		}
		peek(programstack, &temptype);
		if (strcmp(temptype, "VOID"))
		{
			if (!varExists(tempid,0)&&!funcExists(tempid))
			{
				if (length == 1)
				{
					addVar(tempid, "INT", 0, length);
				}
				else
				{
					addVar(tempid, "INT-ARR", 0, length);
					char* tempId;
					char* tempI;
					tempId = (char*)malloc(sizeof(tempid)+2*sizeof(char)+sizeof(int));
					tempI = (char*)malloc(sizeof(int));
					for(int i = 0;i<length;i++)
					{
						strcpy(tempId,tempid);
						strcat(tempId,"[");
						sprintf(tempI,"%d",i);
						strcat(tempId,tempI);
						strcat(tempId,"]");
						addVar(tempId,"ARRAY-ELEMENT",0,1);
					}
				}
			}
			else
			{
				errorLogger("Name-Error: Identifier \"" , tempid, "\" already exists!");
			}
		}
	}
	else
	{
		if (!strcmp(type, "VOID"))
		{
			char* id;
			pop(&programstack, &id);
			errorLogger("Type-Error: Can't declare variable \"",id,"\" as VOID!\n");
		}
		else
		{
			char* id;
			pop(&programstack, &id);
			if(!strcmp(id,"functionsReturnParameter")){
				errorLogger("Name-Error: Variable can't be called \"",id,"\" because this name is reserved for the compiler!\n");
			}
			push(&programstack, type);
			if (!varExists(id,0)&&!funcExists(id))
			{
				if (length == 1)
				{
					addVar(id, "INT", 0, length);
				}
				else
				{
					addVar(id, "INT-ARR", 0, length);
					char* tempId;
					char* tempI;
					tempId = (char*)malloc(sizeof(id)+2*sizeof(char)+sizeof(int));
					tempI = (char*)malloc(sizeof(int));
					for(int i = 0;i<length;i++)
					{
						strcpy(tempId,id);
						strcat(tempId,"[");
						sprintf(tempI,"%d",i);
						strcat(tempId,tempI);
						strcat(tempId,"]");
						addVar(tempId,"ARRAY-ELEMENT",0,1);
					}
				}
			}
			else
			{
				errorLogger("Name-Error: Identifier \"",id,"\" is already in use!\n");
			}
		}
	}
}

void pushSomething()
{
	push(&programstack, "test1");
	char* temp;
	peek(programstack, &temp);
	pop(&programstack, &temp);
	printf("Stack is: %s\n", temp);
	peek(programstack, &temp);
}

void typeReplace(char** type)
{		// Noch nicht implementiert, kommt vlt. noch
	if (!strcmp((*type), "0"))
	{
		printf("\nType is undefined!\n\n");
	}
	else if (!strcmp((*type), "1"))
	{
		printf("\nType is INT!\n\n");
	}
	else if (!strcmp((*type), "2"))
	{
		printf("\nType is VOID!\n\n");
	}
	else if (!strcmp((*type), "3"))
	{
		printf("\nType is INT-ARR!\n\n");
	}
}

void defineFunc(char* id, char* type, int numberOfParams)
{
	if (funcExists(id))
	{	
		if(checkFuncType(id,type)){
			if(funcIsDefined(id))
			{
				errorLogger("Multiple function-Definition: Function \"",id,"\" is already defined!\n");
			}	
			checkFuncParams(id,numberOfParams);
			addVariablesToFunction(id);
		}
		else
		{
			errorLogger("Type-Error: Function \"",id,"\" is defined in different Types!");
		}		
	}
	else if(!varExists(id,1))
	{
		addFunc(id, type, numberOfParams);
		STRUCTFUNC* tempFunc = functions;
		while (strcmp(tempFunc->id,id))
		{
			tempFunc = tempFunc->hh.next;
		}
		tempFunc->funcvars = variables;
		tempFunc->isDefined = 1;
	}
	else
	{
		errorLogger("Name-Error: Identifier \"",id,"\" is already in use!\n");
	}
	checkReturnParam(id,type);
}

void addVar(char* id, char* type, int value, int size)
{
	STRUCTVAR* s;
	s = (STRUCTVAR*)malloc(sizeof(STRUCTVAR));
	s->id = (char*)malloc(sizeof(id));
	s->type = (char*)malloc(sizeof(type));
	strcpy(s->id, id);
	strcpy(s->type, type);
	s->value = value;
	s->size = size;
	HASH_ADD_INT(variables, id, s);
	// logVars();
}

void startScope()
{
	// push variables on scopestack
	SCOPESTACK* temp;
	temp = (SCOPESTACK*)malloc(sizeof(SCOPESTACK));
	temp->scope = variables;
	temp->next = scopes;
	scopes = temp;
	variables = NULL;
}

void endScope()
{
	if (scopes != NULL)
	{
		variables = scopes->scope;
		SCOPESTACK* temp;
		temp = scopes->next;
		scopes = temp;
	}
	parameters = NULL;
}

void addVariablesToFunction(char* id)
{
	STRUCTFUNC* temp = functions;
	while (strcmp(temp->id, id))
	{
		temp = temp->hh.next;
	}
	temp->isDefined = 1;
	temp->funcvars = variables;
}

void lookupFunctionType(char* funcId, char** ret)
{
	STRUCTFUNC* temp;
	for (temp = functions; temp != NULL; temp = temp->hh.next)
	{
		if (!strcmp(temp->id, funcId))
		{
			break;
		}
	}
	if (temp != NULL)
		(*ret) = temp->type;
}

void lookupVariableType(char* varId, char** ret)
{
	SCOPESTACK* tempscope;
	STRUCTPARAM* tempParam;
	STRUCTVAR* temp;
	tempscope = scopes;
	for (temp = variables; temp != NULL; temp = temp->hh.next)
	{
		if (!strcmp(temp->id, varId))
		{
			break;
		}
	}
	if (temp != NULL)
	{
		(*ret) = temp->type;
		return;
	}
	while(tempscope!=NULL)
	{
		for (temp = tempscope->scope; temp != NULL; temp = temp->hh.next)
		{
			if (!strcmp(temp->id, varId))
			{
				break;
			}
		}
		if (temp != NULL)
		{
			(*ret) = temp->type;
			return;
		}
		tempscope = tempscope->next;
	}
	
	for (tempParam = parameters; tempParam != NULL; tempParam = tempParam->next)
	{
		if (!strcmp(tempParam->name, varId))
		{
			break;
		}
	}
	if (tempParam != NULL)
	{
		(*ret) = tempParam->type;
		return;
	}
}
