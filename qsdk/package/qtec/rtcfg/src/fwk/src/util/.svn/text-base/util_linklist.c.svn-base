#include "fwk.h"

/* add entry to the end of the list */
VOS_RET_E addEnd(pENTRY_TYPE entry, pLIST_TYPE list)
{
   if (entry == NULL)
   {
      vosLog_debug("Entry is NULL, do nothing. \n");
      return VOS_RET_INVALID_ARGUMENTS;
   }
   entry->next = NULL;
   if (list->head == NULL)
   {
      list->head = entry;
      list->tail = entry;
   }
   else
   {
      /* always add to the end of the list */
      list->tail->next = entry;
      list->tail = entry;
   }
   return VOS_RET_SUCCESS;
}

/* this is called with list, allocated entry */
VOS_RET_E addFront(pENTRY_TYPE entry, pLIST_TYPE list)
{
   if (entry == NULL)
   {
      vosLog_debug("Entry is NULL, do nothing. \n");
      return VOS_RET_INVALID_ARGUMENTS;
   }
   if (list->head == NULL)
   {
      entry->next = NULL;
      list->head = entry;
      list->tail = entry;
   }
   else
   {
      /* always add to the front of the list */
      entry->next = list->head;
      list->head = entry;
   }
   return (VOS_RET_SUCCESS);
}

/* always remove first entry of the list, and return it caller */
void *removeFront(pLIST_TYPE list)
{
   pENTRY_TYPE ptr=NULL;

   if (list->head == NULL)
   {
      return((void*)ptr);
   }
   
   ptr = list->head;
   list->head = list->head->next;
   
   return ((void*)ptr);
}

/* always remove last entry of the list, and return it caller */
void *removeEnd(pLIST_TYPE list)
{
   pENTRY_TYPE ptr = NULL;
   pENTRY_TYPE prevPtr = NULL;

   if (list->head == NULL)
   {
      return ((void*)ptr);
   }

   if (list->head == list->tail)
   {
      /* this is the only entry left */
      ptr = list->head;
      list->head = list->tail = NULL;
   }
   else
   {
      ptr = list->head;
      prevPtr = ptr;
      while (ptr != NULL)
      {
         if (prevPtr->next == list->tail)
         {
            break;
         }
         prevPtr = ptr;
         ptr = ptr->next;
      }
      ptr = list->tail;
      list->tail = prevPtr;
   }
   return (ptr);
}

/* remove a particular entry with input key */
void *removeEntry(pLIST_TYPE list, void *key, LIST_KEY_TYPE type)
{
   void *ptr=NULL;

   if (list == NULL)
   {
      return ptr;
   }

   if (type == KEY_INT)
   {
      ptr = removeIntEntry(list, (*(int*)key));
   }
   else
   {
      ptr = removeStrEntry(list, (char*)key);
   }
   return (ptr);
}

void *removeFoundEntry(pENTRY_TYPE ptr, pENTRY_TYPE prevPtr, pLIST_TYPE list)
{
   if (list->head == list->tail)
   {
      /*entry is the only one */
      list->head = NULL;
      list->tail = NULL;
   }
   else if (ptr == list->tail)
   {
      /* entry is the last one */
      list->tail = prevPtr;
      prevPtr->next = NULL;
   }
   else if (ptr == prevPtr)
   {
      /* entry is the first one in the list */
      list->head = ptr->next;
   }
   else
   {
      /* entry is in the middle */
      prevPtr->next = ptr->next;
   }
   return (ptr);
}

int findIntEntry(pLIST_TYPE list, int key, pENTRY_TYPE *prevPtr, pENTRY_TYPE *ptr)
{
   int found=0;
   pENTRY_TYPE p, prev;
   
   p = list->head;
   prev = list->head;
   
   while (p != NULL)
   {
      if (*((int*)p->key) == key)
      {
         found = 1;
         break;
      }
      if (p->next != NULL)
      {
         prev = p;
      }
      p = p->next;
   }
   if (!found)
   {
      *ptr = NULL;
      *prevPtr = NULL;
   }
   else
   {
      *ptr = p;
      *prevPtr = prev;
   }

   return (found);
}

int findStrEntry(pLIST_TYPE list, char *key, pENTRY_TYPE *prevPtr, pENTRY_TYPE *ptr)
{
   int found = 0;
   pENTRY_TYPE prev, p;

   p = list->head;
   prev = list->head;

   while (p != NULL)
   {
      if ((strcmp((const char*)(p->key),key)) == 0)
      {
         found = 1;
         break;
      }
      if (p->next != NULL)
      {
         prev = p;
      }
      p = p->next;
   }      
   if (!found)
   {
      *ptr = NULL;
      *prevPtr = NULL;
   }
   else
   {
      *ptr = p;
      *prevPtr = prev;
   }
   return (found);
}


void *removeIntEntry(pLIST_TYPE list, int key)
{
   pENTRY_TYPE ptr;
   pENTRY_TYPE prevPtr;
   int found = 0;

   found = findIntEntry(list,key,&prevPtr,&ptr);

   if (found)
   {
      ptr = removeFoundEntry(ptr,prevPtr, list);
   }
   else
   {
      ptr = NULL;
   }
   return (ptr);
}

void *removeStrEntry(pLIST_TYPE list, char *key)
{
   pENTRY_TYPE ptr, prevPtr;
   int found = 0;

   found = findStrEntry(list,key,&prevPtr,&ptr);
   if (found)
   {
      ptr = removeFoundEntry(ptr,prevPtr, list);
   }
   else
   {
      ptr = NULL;
   }
   return (ptr);
}

/* sort the list in ascending order */
void sortIntList(pLIST_TYPE list)
{
   pENTRY_TYPE smallestPtr;
   pENTRY_TYPE ptr;
   void *saveNumber;

   smallestPtr = list->head;
   ptr = list->head->next;

   if ((smallestPtr == NULL) || (ptr == NULL))
   {
      /* there is no entry or only 1 entry */
      vosLog_debug("smallestPtr is NULL or ptr is NULL\n");
      return;
   }
   while ( smallestPtr != NULL)
   {
      while (ptr != NULL)
      {
         /* comparing to smallest element */
         if (*((int*)ptr->key) < *((int*)smallestPtr->key))
         {
            saveNumber = smallestPtr->key;
            smallestPtr->key = ptr->key;
            ptr->key = saveNumber;
         }
         ptr = ptr->next;
      }
      smallestPtr = smallestPtr->next;
      if (smallestPtr != NULL)
      {
         ptr = smallestPtr->next;
      }
   }  /* loop through all */
}

void printList(const pLIST_TYPE list, LIST_KEY_TYPE type)
{
   pENTRY_TYPE ptr=list->head;
   vosLog_debug("\nThe List: list->head %p, list->tail %p\n",list->head, list->tail);
   
   while (ptr != NULL)
   {
      if (ptr->key != NULL) 
      {
         if (type == KEY_INT)
         {
            vosLog_debug("ptr %p, %d\n",ptr,*((int*)(ptr->key)));
         }
         else
         {
            vosLog_debug("%s\n",(char*)ptr->key);
         }
      }
      else
      {
         /* key is NULL, there is nothing to display, just exit */
         break;
      }
      ptr = ptr->next;
   } /* while */
}

/* find a particular entry with input key, return found or not found */
int findEntry(pLIST_TYPE list, void *key, LIST_KEY_TYPE type, pENTRY_TYPE *prevPtr, pENTRY_TYPE *ptr)
{
   int found = 0;

   if (list == NULL)
   {
      return found;
   }

   if (type == KEY_INT)
   {
      found = findIntEntry(list, (*(int*)key),prevPtr,ptr);
   }
   else
   {
      found = findStrEntry(list, (char*)key,prevPtr,ptr);
   }
   return (found);
}

#if 0
main(int argc, char **argv)
{
   int i;
   int number;
   int *data, *key;
   int array[11];
   pENTRY_TYPE testEntry;
   LIST_TYPE  testList = {NULL,NULL};
   
   for (i =1; i<=10; i++)
   {
      testEntry = (pENTRY_TYPE)VOS_MALLOC(sizeof(ENTRY_TYPE));
      if (testEntry == NULL)
      {
         vosLog_debug("error allocating testEntry\n");
      }
      data = VOS_MALLOC(sizeof(int));
      if (data == NULL)
      {
         vosLog_debug("error allocating data\n");
         VOS_FREE(testEntry);
         return;
      }
      *data = i;
      key = VOS_MALLOC(sizeof(int));
      if (key == NULL)
      {
         vosLog_debug("error allocating Key\n");
         VOS_FREE(testEntry);
         VOS_FREE(data);
         return;
      }
      *key = i;
      testEntry->next=NULL;
      testEntry->data = (void*) data;
      testEntry->keyType = KEY_INT;
      testEntry->key = (void*)key;
      addEnd(testEntry,&testList);
   }
   printList(&testList,KEY_INT);

   vosLog_debug("\n========Removing entries ===========\n");
   
   while (1)
   {
      //      testEntry = removeFront(&testList);
      testEntry = removeEnd(&testList);
      if (testEntry == NULL)
      {
         vosLog_debug("there is no more entry to remove\n");
         break;
      }

      vosLog_debug("removed ptr %p, entry->data: %d\n",testEntry,*((int*)(testEntry->key)));
      VOS_FREE(testEntry->data);
      VOS_FREE(testEntry->key);
      VOS_FREE(testEntry);
   } /* while */

   vosLog_debug("\n---------Add Random number to List -----------\n");

   for (i =0; i<=10; i++)
   {
      array[i] = 0;
   }
   for (i =1; i<=10; i++)
   {
      number = rand() % 10 + 1;
      while (array[number] != 0)
      {
         /* number is already used */
         number = rand() % 10 + 1;
      }
      array[number] = number;


      testEntry = (pENTRY_TYPE)VOS_MALLOC(sizeof(ENTRY_TYPE));
      if (testEntry == NULL)
      {
         vosLog_debug("error allocating testEntry\n");
      }
      data = VOS_MALLOC(sizeof(int));
      if (data == NULL)
      {
         vosLog_debug("error allocating data\n");
         VOS_FREE(testEntry);
         return;
      }
      *data = number;
      key = VOS_MALLOC(sizeof(int));
      if (key == NULL)
      {
         vosLog_debug("error allocating Key\n");
         VOS_FREE(testEntry);
         VOS_FREE(data);
         return;
      }
      *key = number;
      testEntry->next=NULL;
      testEntry->data = (void*) data;
      testEntry->keyType = KEY_INT;
      testEntry->key = (void*)key;
      addFront(testEntry,&testList);
   }
   printList(&testList,KEY_INT);


   vosLog_debug("\n================= SORTED ================\n");
   sortIntList(&testList);

   printList(&testList,KEY_INT);
}

#endif
