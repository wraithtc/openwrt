#include "fwk.h"


pLIST_ENTRY_T list_create_entry(void *data)
{
    pLIST_ENTRY_T entry = (pLIST_ENTRY_T)VOS_MALLOC(sizeof(LIST_ENTRY_T));
    entry->next = NULL;
    entry->data = data;

    return entry;
}

/* this is called with list, allocated entry */
VOS_RET_E list_prepend(pLIST_ENTRY_T entry, pLIST_T list)
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
    return VOS_RET_SUCCESS;
}


VOS_RET_E list_append(pLIST_ENTRY_T entry, pLIST_T list)
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


void* list_deleteHead(pLIST_T list)
{
    pLIST_ENTRY_T ptr = NULL;

    if (list->head == NULL)
    {
        return((void*)ptr);
    }
   
    ptr = list->head;
    list->head = list->head->next;
   
    return ((void*)ptr);
}


void* list_deleteTail(pLIST_T list)
{
    pLIST_ENTRY_T ptr = NULL;
    pLIST_ENTRY_T prevPtr = NULL;

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


void  *list_delete(pLIST_ENTRY_T entry, pLIST_T list)
{
    pLIST_ENTRY_T ptr = NULL;
    pLIST_ENTRY_T prevPtr = NULL;
    
    prevPtr = ptr = list->head;
    while (NULL != ptr)
    {
        if (ptr == entry)
        {
            break;
        }
        prevPtr = ptr;
        ptr = ptr->next;
    }
    
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

