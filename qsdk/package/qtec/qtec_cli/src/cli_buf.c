#include "fwk.h"
#include "cli_buf.h"


//typedef struct buffer CLI_BUFF_T;
/* Make buffer data. */
struct buffer_data *buffer_data_new ( unsigned long size )
{
    struct buffer_data *d;

    d = ( struct buffer_data * ) VOS_MALLOC ( sizeof ( struct buffer_data ) );
    if ( d == NULL )    /* add by liuyanjun 2002/08/14 */
    {
        return NULL;
    }           /* end */
    memset ( ( char * ) d, 0, sizeof ( struct buffer_data ) );
    d->data = ( unsigned char * ) VOS_MALLOC ( size );
    if ( d->data == NULL )    /* add by liuyanjun 2002/08/14 */
    {
        VOS_FREE( d );
        return NULL;
    }           /* end */
    memset( d->data, 0, size );
    return d;
}


void buffer_data_free ( struct buffer_data * d )
{
    if ( d->data )
    {
        VOS_FREE( d->data );
    }
    VOS_FREE( d );
}


/* Make new buffer. */
struct buffer *buffer_new ( int type, unsigned long size )
{
    struct buffer *b;

    b = ( struct buffer * ) VOS_MALLOC ( sizeof ( struct buffer ) );
    if ( b == NULL )    /* add by liuyanjun 2002/08/14 */
    {
        return NULL;
    }           /* end */
    memset ( ( char * ) b, 0, sizeof ( struct buffer ) );

    b->type = type;
    b->size = size;

    return b;
}


/* Free buffer. */
void buffer_free ( struct buffer * b )
{
    struct buffer_data *d;
    struct buffer_data *next;

    d = b->head;
    while ( d )
    {
        next = d->next;
        buffer_data_free ( d );
        d = next;
    }

    d = b->unused_head;
    while ( d )
    {
        next = d->next;
        buffer_data_free ( d );
        d = next;
    }

    VOS_FREE ( b );
}


/* Make string clone. */
char *buffer_getstr ( struct buffer * b )
{
    return VOS_STRDUP( ( char * ) b->head->data );
}


/* Return 1 if buffer is empty. */
int buffer_empty ( struct buffer * b )
{
    if ( b == NULL || b->tail == NULL || b->tail->cp == b->tail->sp )
    {
        return 1;
    }
    else
    {
        return 0;
    }
}


/* Clear and free all allocated data. */
void buffer_reset ( struct buffer * b )
{
    struct buffer_data *data;
    struct buffer_data *next;

    for ( data = b->head; data; data = next )
    {
        next = data->next;
        buffer_data_free ( data );
    }
    b->head = b->tail = NULL;
    b->alloc = 0;
    b->length = 0;
}


/* Add buffer_data to the end of buffer. */
int buffer_add ( struct buffer * b )
{
    struct buffer_data *d;

    d = buffer_data_new ( b->size );
    if ( d == NULL )    /* add by liuyanjun 2002/08/14 */
    {
        return 0 ;
    }           /* end */

    if ( b->tail == NULL )
    {
        d->prev = NULL;
        d->next = NULL;
        b->head = d;
        b->tail = d;
    }
    else
    {
        d->prev = b->tail;
        d->next = NULL;

        b->tail->next = d;
        b->tail = d;
    }

    b->alloc++;

    return 1 ;
}


/* Write data to buffer. */
int buffer_write_vty_out ( struct buffer * b, unsigned char * ptr, unsigned long size )
{
    int nDivide1 = 0;
    int nDivide2 = 0;

    nDivide1 = b->length / BAC_BATCH_MSG_LEN ;
    nDivide2 = ( b->length + size ) / BAC_BATCH_MSG_LEN ;

    if ( nDivide2 > nDivide1 )/*填充64k空间的剩余部分，据说是打包需要*/
    {
        long size_space = 0;
        unsigned char * spaceptr = NULL ;

        size_space = nDivide2 * BAC_BATCH_MSG_LEN - b->length ;
        if ( size_space < 0 )
        {
            UTIL_Assert( 0 );
            return 0;
        }
        spaceptr = VOS_MALLOC( size_space  );
        if ( spaceptr == NULL )
        {
            buffer_reset( b ) ;
            return 0;
        }

        memset ( spaceptr, 0, size_space );
        {
            int i = 0;
            spaceptr[size_space-1] = '\n';
            for(i = 0; i < size_space-1; i++)
            {
                spaceptr[i] = ' ';
            }
        }

        if ( buffer_write( b, spaceptr , size_space ) == 0 )
        {
            VOS_FREE( spaceptr );
            return 0;
        }

        VOS_FREE( spaceptr );
    }

    if ( buffer_write( b, ptr , size ) == 0 )
    {
        return 0 ;
    }

    return 1;

}


/* Write data to buffer. If no memery , free all memory of  b */
int buffer_write ( struct buffer * b, unsigned char * ptr, unsigned long size )
{
    struct buffer_data *data;

    data = b->tail;
    b->length += size;

    /* We use even last one byte of data buffer. */
    while ( size )
    {
        /* If there is no data buffer add it. */
        if ( data == NULL || data->cp == b->size )
        {
            if ( buffer_add ( b ) == 0 )
            {
                buffer_reset( b ) ;
                UTIL_Assert( 0 );
                return 0 ;
            }
            data = b->tail;
        }

        /* Last data. */
        /* 如果缓冲区空间够用，将所有的新的数据全部拷贝到缓冲区尾部。 */
        if ( size <= ( b->size - data->cp ) )
        {
            memcpy ( ( data->data + data->cp ), ptr, size );

            data->cp += size;
            size = 0;
        }
        /* 否则，将部分数据拷贝到缓冲区尾部，之后再分配新的缓冲区。 */
        else
        {
            memcpy ( ( data->data + data->cp ), ptr, ( b->size - data->cp ) );

            size -= ( b->size - data->cp );
            ptr += ( b->size - data->cp );

            data->cp = b->size;
        }
    }

    return 1;
}


/* Insert character into the buffer. */
int buffer_putc ( struct buffer * b, unsigned char c )
{
    if ( buffer_write ( b, &c, 1 ) == 0 )
    {
        return 0 ;
    }

    return 1;
}


/* Insert word (2 octets) into ther buffer. */
int buffer_putw ( struct buffer * b, unsigned short c )
{
    if ( buffer_write ( b, ( unsigned char * ) &c, 2 ) == 0 )
    {
        return 0 ;
    }
    return 1;
}


/* Put string to the buffer. */
int buffer_putstr ( struct buffer * b, unsigned char * c )
{
    unsigned long size;

    size = strlen ( ( char * ) c );

    if ( buffer_write ( b, c, size ) == 0 )
    {
        return 0 ;
    }

    return 1;
}


int buffer_flush_vty ( struct buffer * b, unsigned int size,
                       int erase_flag, int no_more_flag )
{
    int nbytes = 0;

    char more[] = " --Press any key to continue Ctrl+c to stop--";
    char erase[] = { 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08,
                     0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08,
                     0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08,
                     0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08,
                     0x08, 0x08, 0x08, 0x08, 0x08, 0x08,
                     ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',
                     ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',
                     ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',
                     ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',
                     ' ', ' ', ' ', ' ', ' ', ' ',
                     0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08,
                     0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08,
                     0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08,
                     0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08,
                     0x08, 0x08, 0x08, 0x08, 0x08, 0x08
                   };
    struct buffer_data *data;
    struct buffer_data *out;
    struct buffer_data *next;

    data = b->head;

    /* Previously print out is performed. */
    if ( erase_flag )
    {
        write( STDOUT_FILENO, erase, sizeof erase );
    }

    /* Output data. */
    for ( data = b->head; data; data = data->next )
    {
        /* 如果输出的数据的大小或等于当前缓冲区中的数据的     */
        /* 长度，那么在当前缓冲区中取出size大小的数据来即可。 */
        /* data是最后一个需要取数据的缓冲区。                 */
        if ( size <= ( data->cp - data->sp ) )
        {
            nbytes = write( STDOUT_FILENO, ( char * ) ( data->data + data->sp ), size );
            data->sp += size;
            /* 如果当前缓冲区所有的数据已经取完，取下个缓冲区的数据。 */
            if ( data->sp == data->cp )
            {
                data = data->next;
            }
            break;
        }
        /* 否则，取出当前缓冲区中的所有数据。 */
        else
        {
            nbytes = write( STDOUT_FILENO, ( char * ) ( data->data + data->sp ), ( int ) ( data->cp - data->sp ) );
            size -= ( data->cp - data->sp );
            data->sp = data->cp;
        }
    }
    /* In case of `more' display need. */
    if ( !buffer_empty ( b ) && !no_more_flag )
    {
        write( STDOUT_FILENO, ( char * ) more, sizeof more );
    }

    /* Free printed buffer data. */
    /* 释放从链表头到data的所有的缓冲区。 */
    for ( out = b->head; out && out != data; out = next )
    {
        next = out->next;
        if ( next )
        {
            next->prev = NULL;
        }
        else
        {
            b->tail = next;
        }
        b->head = next;

        b->alloc--;
        b->length -= out->cp;
        buffer_data_free ( out );
    }

    return nbytes;
}


/* Calculate size of outputs then flush buffer to the file
   descriptor. */
int buffer_flush_window ( struct buffer * b, int width, int height,
                          int erase, int no_more )
{
    unsigned long cp;
    unsigned long size;
    int lineno;
    int ret;
    struct buffer_data *data;

    if ( height >= 2 )
    {
        height--;
    }

    /* We have to calculate how many bytes should be written. */
    lineno = 0;
    size = 0;
    if ( width == 0 )
        lineno++;

    for ( data = b->head; data; data = data->next )
    {
        cp = data->sp;

        while ( cp < data->cp )
        {
            if ( data->data[ cp ] == '\n' )
            {
                lineno++;
                if ( lineno == height )
                {
                    cp++;
                    size++;
                    goto flush;
                }
            }
            cp++;
            size++;
        }
    }

    /* Write data to the file descriptor. */
flush:

    ret = buffer_flush_vty ( b, size, erase, no_more );

    return ret;
}


void buffer_dump ( struct buffer * b )
{
    if (NULL == b)
    {
        return;
    }
}
