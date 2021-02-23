#ifndef LINKEDLIST_H
#define LINKEDLIST_H

typedef struct node
{
   int value;
   int frame;
   struct node *next;
} ListNode;

ListNode* addHead(ListNode *list, int value);

ListNode* addTail(ListNode *list, int value, int frame);

int getMaxIndex(ListNode *list);

ListNode* deleteNode(ListNode *list, int index);

int getIndex(ListNode *list, int value);

void printList(ListNode *list);

#endif

