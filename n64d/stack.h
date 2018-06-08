// small stack implemation for our jump saving
#include <stdio.h>
#include <stdlib.h>


// stack datastruct
struct StackNode
{
    unsigned int pc;
    struct StackNode* next;
};

// allocate a new node
struct StackNode* newNode(unsigned int pc)
{
    struct StackNode* stackNode = (struct StackNode*) malloc(sizeof(struct StackNode));
    stackNode->pc = pc;
    stackNode->next = NULL;
    return stackNode;
}

// check if our stack is empty
int isEmpty(struct StackNode *base)
{
    return !base;
}

// put an address on the stack
void push(struct StackNode** root, unsigned int pc)
{
    struct StackNode* stackNode = newNode(pc);
    stackNode->next = *root;
    *root = stackNode;
    
}

// take a saved address of the stack
int pop(struct StackNode** root)
{
    if (isEmpty(*root))
	{
        return 0;
	}
    struct StackNode* temp = *root;
    *root = (*root)->next;
    unsigned int pc = temp->pc;
    free(temp);
 
    return pc;
}

