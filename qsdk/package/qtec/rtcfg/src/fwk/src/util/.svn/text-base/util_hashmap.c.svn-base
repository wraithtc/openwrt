
#include "fwk.h"


/*Hash a string , return a hash key */

static UINT32 hash_string(const char *s, SINT32 len)
{
    UINT32 hashKey = 0;
    SINT32 i = 0;

    if (len < 0)
    {
        len = ((NULL == s) ? 0 : util_strlen(s));
    }

    while (i++ < len)
    {
        hashKey = 17*hashKey + *s++;
    }

    return hashKey;
}


static void hash_free_map_key(LIST_ENTRY_T *entry)
{
    LIST_ENTRY_T *oldEntry = NULL;

    while (entry)
    {
        oldEntry = entry;
        entry = oldEntry->next;

        VOS_FREE(oldEntry->data);
        VOS_FREE(oldEntry);
    }
}


static void hash_free_map_value(LIST_ENTRY_T *entry, hmap_value_free_hook pFreeHook)
{
    LIST_ENTRY_T *oldEntry = NULL;

    while (entry)
    {
        oldEntry = entry;
        entry = oldEntry->next;

        if (NULL != pFreeHook)
        {
            (*pFreeHook)(oldEntry->data);
        }
        VOS_FREE(oldEntry);
    }
}


/* ===================================================================================
                            Public Functions
====================================================================================*/

void hashmap_create(pHASH_MAP_T *hmap, SINT32 size)
{
    (*hmap) = (HASH_MAP_T *)VOS_MALLOC(sizeof(HASH_MAP_T));
    (*hmap)->size = size;
    (*hmap)->key = (LIST_ENTRY_T **)VOS_CALLOC(size, sizeof(LIST_ENTRY_T *));
    (*hmap)->value = (LIST_ENTRY_T **)VOS_CALLOC(size, sizeof(LIST_ENTRY_T *));
}


void hashmap_destroy(pHASH_MAP_T hmap, hmap_value_free_hook pFreeHook)
{
    UINT32 i = 0;

    for (i = 0; i < hmap->size; i++)
    {
        hash_free_map_key(hmap->key[i]);
        hash_free_map_value(hmap->value[i], pFreeHook);
    }

    VOS_FREE(hmap->key);
    VOS_FREE(hmap->value);
    VOS_FREE(hmap);
}


void hashmap_insert(pHASH_MAP_T hmap, const char *key, SINT32 keyLen, void *value)
{
    LIST_ENTRY_T *entryKey = NULL;
    LIST_ENTRY_T *entryVal = NULL;
    UINT32 h = 0;
    char *s = NULL;

    if (NULL == key)
    {
        vosLog_error("Invalid Param!");
        return;
    }

    if (keyLen < 0)
    {
        keyLen = util_strlen(key);
    }

    s = (char *)VOS_MALLOC(keyLen+1);
    if (NULL == s)
    {
        vosLog_error("Fail to malloc!");
        return;  
    }

    UTIL_STRNCPY(s, key, keyLen+1);
    s[keyLen] = 0;

    entryKey = list_create_entry((void *)s);
    entryVal = list_create_entry(value);
    if (NULL == entryKey || NULL == entryVal)
    {
        vosLog_error("Fail to list_create_entry!");
        return ;
    }

    h = hash_string(s, keyLen) % hmap->size;

    entryKey->next = hmap->key[h];
    hmap->key[h] = entryKey;

    entryVal->next = hmap->value[h];
    hmap->value[h] = entryVal;
    
}


void *hashmap_search(pHASH_MAP_T hmap, const char *key)
{
    UINT32 h = hash_string(key, -1) % hmap->size;
    LIST_ENTRY_T *entryKey = hmap->key[h];
    LIST_ENTRY_T *entryVal = hmap->value[h];

    while (NULL != entryKey)
    {
        if (0 == util_strcmp(key, (char *)(entryKey->data)))
        {
            return entryVal->data;
        }
        
        entryKey = entryKey->next;
        entryVal = entryVal->next;
    }

    return NULL;
}

