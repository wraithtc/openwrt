/**********************************************************************************************************************
*     Copyright (C), 2008, T&W. Ltd. All rights reserved.
*
*     FileName    : vos_vector.c
*     Author      : Cai Suyuan
*     Version     : V1.0       
*     Date        : 2008/10/21
*     Description : the vector data structure implement
*     Other       :
*     History     :        
*********************************************************************************************************************/

#include "fwk.h"


void *UTIL_VectorRealloc( void * ptr, UINT32 old_size, UINT32 new_size )
{
    void * new_ptr;
    new_ptr = ( void * ) VOS_MALLOC_FLAGS( new_size , 0);
    if ( new_ptr == NULL )        
        return NULL;          
    memcpy( new_ptr, ptr, ( new_size > old_size ) ? old_size : new_size );
    VOS_FREE( ptr );
    ptr = new_ptr;

    return new_ptr;
}

UTIL_VECTOR UTIL_VectorInit( UINT32 size )
{
    UTIL_VECTOR v = ( UTIL_VECTOR ) VOS_MALLOC_FLAGS( sizeof ( struct _util_vector) , 0 );
    if ( v == NULL )       
        return NULL;
    /* allocate at least one slot */
    if ( size == 0 )
    {
        size = 1;
    }

    v->alloced = size;
    v->max = 0;
    v->index = ( void * ) VOS_MALLOC_FLAGS( sizeof ( void * ) * size, 0 );
    if ( v->index == NULL )
    {
        VOS_FREE( v );
        return NULL;   
    }
    memset( v->index, 0, sizeof ( void * ) * size );

    return v;
}


void UTIL_VectorOnlyWapperFree( UTIL_VECTOR v )
{
    VOS_FREE ( v );
}


void UTIL_VectorOnlyIndexFree( void * idx )
{
    VOS_FREE ( idx );
}


void UTIL_VectorFree( UTIL_VECTOR v )
{
    VOS_FREE( v->index );
    VOS_FREE ( v );
}


UTIL_VECTOR UTIL_VectorCopy( UTIL_VECTOR v )
{
    UINT32 size;
    UTIL_VECTOR newVector = NULL;
    if ( NULL == v )
    {
        UTIL_Assert( 0 );
        return NULL;
    }
    newVector = ( UTIL_VECTOR ) VOS_MALLOC_FLAGS( sizeof ( struct _util_vector ), 0 );
    if ( newVector == NULL )        
        return NULL;

    newVector->max = v->max;
    newVector->alloced = v->alloced;

    size = sizeof ( void * ) * ( v->alloced );
    newVector->index = ( void * ) VOS_MALLOC_FLAGS( size , 0);
    if ( newVector->index == NULL )
    {
        VOS_FREE( newVector );
        return NULL;
    }
    memcpy( newVector->index, v->index, size );

    return newVector;
}


VOS_RET_E UTIL_VectorEnsure( UTIL_VECTOR v, UINT32 num )
{
    void * new_ptr = NULL;

    if ( NULL == v )
    {
        UTIL_Assert( 0 );
        return VOS_RET_INVALID_ARGUMENTS;
    }
    if ( v->alloced > num )
    {
        return VOS_RET_SUCCESS;
    }

    new_ptr = UTIL_VectorRealloc ( v->index, sizeof ( void * ) * v->alloced, sizeof ( void * ) * ( v->alloced * 2 ) );
    if ( new_ptr == NULL )
    {
        return VOS_RET_INTERNAL_ERROR;                 
    }
    v->index = new_ptr;
    memset ( &v->index[ v->alloced ], 0, sizeof ( void * ) * v->alloced );
    v->alloced *= 2;

    if ( v->alloced <= num )
    {
        VOS_RET_E ret = VOS_RET_SUCCESS;
        ret = UTIL_VectorEnsure ( v, num );
        return ret;
    }
    
    return VOS_RET_SUCCESS;
}


VOS_RET_E UTIL_VectorEmptySlot( UTIL_VECTOR v, UINT32 * num )
{
    UINT32 i;
    if ( NULL == v )
        return VOS_RET_INVALID_ARGUMENTS;

    if ( v->max == 0 )
    {
        *num = 0;
        return VOS_RET_SUCCESS;
    }

    for ( i = 0; i < v->max; i++ )
    {
        if ( v->index[ i ] == 0 )
        {
            *num = i;
            return VOS_RET_SUCCESS;
        }
    }

    *num = i;
    return VOS_RET_SUCCESS;
}


VOS_RET_E UTIL_VectorSet( UTIL_VECTOR v, void * val )
{
    UINT32 i;

    if ( NULL == v )
    {
        UTIL_Assert( 0 );
        return VOS_RET_INVALID_ARGUMENTS;
    }

    if ( ( VOS_RET_SUCCESS != UTIL_VectorEmptySlot( v, &i ) ) ||
            ( VOS_RET_SUCCESS != UTIL_VectorEnsure( v, i ) )
       )  
    {
        UTIL_Assert( 0 );
        return VOS_RET_INTERNAL_ERROR;
    }

    v->index[ i ] = val;

    if ( v->max <= i )
    {
        v->max = i + 1;
    }

    return VOS_RET_SUCCESS;
}


VOS_RET_E UTIL_VectorSetIndex( UTIL_VECTOR v, UINT32 i, void * val )
{
    if ( VOS_RET_SUCCESS != UTIL_VectorEnsure( v, i ) )  
    {
        UTIL_Assert( 0 );
        return VOS_RET_INVALID_ARGUMENTS;
    }

    v->index[ i ] = val;

    if ( v->max <= i )
    {
        v->max = i + 1;
    }

    return VOS_RET_SUCCESS;
}


void *UTIL_VectorLookupIndex( UTIL_VECTOR v, UINT32 i )
{
    if ( VOS_RET_SUCCESS != UTIL_VectorEnsure( v, i ) )
    {
        UTIL_Assert( 0 );
        return NULL;
    }
    return v->index[ i ];
}


VOS_RET_E UTIL_VectorUnset( UTIL_VECTOR v, UINT32 i )
{
    if ( NULL == v )
    {
        UTIL_Assert( 0 );
        return VOS_RET_INVALID_ARGUMENTS;
    }
    if ( i >= v->alloced )
    {
        return VOS_RET_INTERNAL_ERROR;
    }

    v->index[ i ] = NULL;

    if ( i + 1 == v->max )
    {
        v->max--;
        while ( i && v->index[ --i ] == NULL && v->max-- )
            ; 
    }

    return VOS_RET_SUCCESS;
}


UINT32 UTIL_VectorCount( UTIL_VECTOR v )
{
    UINT32 i;
    UINT32 count = 0;

    if ( NULL == v )
    {
        UTIL_Assert( 0 );
        return 0;
    }

    for ( i = 0; i < v->max; i++ )
    {
        if ( v->index[ i ] != NULL )
        {
            count++;
        }
    }

    return count;
}
