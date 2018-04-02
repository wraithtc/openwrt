#ifndef __CLI_BUF_H__
#define __CLI_BUF_H__


#define PUTC(val, pnt) do{ (*(unsigned char *)(pnt)++) = (unsigned char)(val) }while(0)
#define PUTW(val, pnt) do {   unsigned short t = htons((unsigned short)(val));   linux_memcpy ((pnt), &t, 2);    (pnt) += 2; } while (0)
#define PUTL(val, pnt) do {  unsigned long t = htonl((unsigned long)(val));   linux_memcpy ((pnt), &t, 4);    (pnt) += 4; } while (0)

/* Buffer type index. */
#define BUFFER_STRING      0
#define BUFFER_STREAM      1
#define BUFFER_VTY         2

#define BAC_BATCH_MSG_LEN    (64 * 1024 ) /*备份命令消息的长度*/


/* Buffer master. */
struct buffer_data ;

struct buffer
{
    /* Type of this buffer. */
    int type;

    /* Data list. */
    struct buffer_data *head;
    struct buffer_data *tail;

    /* Current allocated data. */
    unsigned long alloc;

    /* Total length of buffer. */
    unsigned long size;

    /* For allocation. */
    struct buffer_data *unused_head;
    struct buffer_data *unused_tail;

    /* Current total length of this buffer. */
    unsigned long length;
};

/* Data container. */
struct buffer_data
{
  struct buffer *parent;
  struct buffer_data *next;
  struct buffer_data *prev;

  /* Acctual data stream. */
  unsigned char *data;

  /* Current pointer. 新的数据冲这个位置输入缓冲区。*/
  unsigned long cp;

  /* Start pointer.指的是本缓冲区还没输出的数据的起始地址。 */
  unsigned long sp;
};


/* Buffer prototypes. */
void buffer_data_free (struct buffer_data *d) ;
/*int buffer_flush_all (struct buffer *, int);*/
int buffer_flush_window (struct buffer *, int, int, int, int);
int buffer_empty (struct buffer *);
struct buffer *buffer_new (int, unsigned long);
int buffer_write (struct buffer *, unsigned char *, unsigned long );
int buffer_write_vty_out (struct buffer *b, unsigned char * ptr, unsigned long size);
void buffer_free (struct buffer *);
char *buffer_getstr (struct buffer *);
int buffer_putc (struct buffer *, unsigned char );
int buffer_putstr (struct buffer *, unsigned char * );
void buffer_reset (struct buffer *);


#endif /* __CLI_BUF_H__ */
