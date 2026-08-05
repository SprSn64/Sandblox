#ifndef BLOCKCODE_STRUCTS_H
#define BLOCKCODE_STRUCTS_H

typedef enum CodeBlockFlags{
	CODEBLOCK_NORMAL,
	CODEBLOCK_CAP, //blocks next item
	CODEBLOCK_PULL, //pull piece off of block into new code block (New var block -> variable item)

	CODEBLOCK_VAR,
	CODEBLOCK_OPERATOR,
	CODEBLOCK_CONST, //string, number, whateverb
} CodeBlockFlags;

typedef struct CodeBlockClass{
	Uint32 id;
	SDL_FColor colour;

	void* func;
	Uint32 flags; //capped, operation block, pull thing, etc
} CodeBlockClass;

typedef struct CodeBlock{
	//class type (not sure if Uint32 or struct is better option)
	CodeBlockClass* blockClass;

	void* content;
	void* output;

	struct CodeBlock* prev;
	struct CodeBlock* next;
	struct CodeBlock* child;
	struct CodeBlock* parent;
} CodeBlock;

typedef struct CodeBlockHeader{
	CodeBlock* block;
	SDL_FPoint pos; 
} CodeBlockHeader;

#endif