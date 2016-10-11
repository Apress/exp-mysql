/* Copyright (C) 2003 MySQL AB

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA 
*/

#ifdef USE_PRAGMA_IMPLEMENTATION
#pragma implementation        // gcc: Class implementation
#endif

#include "mysql_priv.h"
#include "ha_spartan.h"

#include <mysql/plugin.h>

static handler* spartan_create_handler(TABLE_SHARE *table);
static int spartan_init_func();

handlerton spartan_hton= {
  MYSQL_HANDLERTON_INTERFACE_VERSION,
  "SPARTAN",
  SHOW_OPTION_YES,
  "Spartan storage engine", 
  DB_TYPE_SPARTAN_DB,
  (bool (*)()) spartan_init_func,
  0,       /* slot */
  0,       /* savepoint size. */
  NULL,    /* close_connection */
  NULL,    /* savepoint */
  NULL,    /* rollback to savepoint */
  NULL,    /* release savepoint */
  NULL,    /* commit */
  NULL,    /* rollback */
  NULL,    /* prepare */
  NULL,    /* recover */
  NULL,    /* commit_by_xid */
  NULL,    /* rollback_by_xid */
  NULL,    /* create_cursor_read_view */
  NULL,    /* set_cursor_read_view */
  NULL,    /* close_cursor_read_view */
  spartan_create_handler,    /* Create a new handler */
  NULL,    /* Drop a database */
  NULL,    /* Panic call */
  NULL,    /* Start Consistent Snapshot */
  NULL,    /* Flush logs */
  NULL,    /* Show status */
  NULL,    /* Partition flags */
  NULL,    /* Alter table flags */
  NULL,    /* Alter tablespace */
  NULL,    /* Fill Files table */
  HTON_CAN_RECREATE
};

/* Variables for spartan share methods */
static HASH spartan_open_tables; // Hash used to track open tables
pthread_mutex_t spartan_mutex;   // This is the mutex we use to init the hash
static int spartan_init= 0;      // Variable for checking the init state of hash


/*
  Function we use in the creation of our hash to get key.
*/
static byte* spartan_get_key(SPARTAN_SHARE *share,uint *length,
                             my_bool not_used __attribute__((unused)))
{
  *length=share->table_name_length;
  return (byte*) share->table_name;
}


static int spartan_init_func()
{
  if (!spartan_init)
  {
    spartan_init++;
    VOID(pthread_mutex_init(&spartan_mutex,MY_MUTEX_INIT_FAST));
    (void) hash_init(&spartan_open_tables,system_charset_info,32,0,0,
                     (hash_get_key) spartan_get_key,0,0);
  }
  return 0;
}

static int spartan_done_func()
{
  if (spartan_init)
  {
    if (spartan_open_tables.records)
    {
      return 1;
    }
    hash_free(&spartan_open_tables);
    pthread_mutex_destroy(&spartan_mutex);
    spartan_init--;
  }
  return 0;
}


/*
  Example of simple lock controls. The "share" it creates is structure we will
  pass to each spartan handler. Do you have to have one of these? Well, you have
  pieces that are used for locking, and they are needed to function.
*/
static SPARTAN_SHARE *get_share(const char *table_name, TABLE *table)
{
  SPARTAN_SHARE *share;
  uint length;

  if (!spartan_init)
     spartan_init_func();

  pthread_mutex_lock(&spartan_mutex);
  length=(uint) strlen(table_name);

  if (!(share=(SPARTAN_SHARE*) hash_search(&spartan_open_tables,
                                           (byte*) table_name,
                                           length)))
  {
    /*
      Allocate several memory blocks at one time.
      Note: my_multi_malloc takes MySQL flags 
      (set to zero fill and with extra error checking),
      one or more pairs of addresses and size of memory to allocate.
    */
    if (!my_multi_malloc(MYF(MY_WME | MY_ZEROFILL),
                          &share, sizeof(*share),
                          NullS))
    {
      pthread_mutex_unlock(&spartan_mutex);
      return NULL;
    }
    /*
      Set the initial variables to defaults.
    */
    share->use_count=0;
    share->table_name_length=length;
    share->table_name = (char *)my_malloc(length + 1, MYF(0));
    strcpy(share->table_name,table_name);
    /*
      Insert table name into hash for future reference.
    */
    if (my_hash_insert(&spartan_open_tables, (byte*) share))
      goto error;
    thr_lock_init(&share->lock);
    /*
      Create an instance of data class
    */
    share->data_class = new Spartan_data();
    /*
      Create an instance of index class
    */
    share->index_class = new Spartan_index();
    pthread_mutex_init(&share->mutex,MY_MUTEX_INIT_FAST);
  }
  share->use_count++;
  pthread_mutex_unlock(&spartan_mutex);

  return share;

error:
  pthread_mutex_destroy(&share->mutex);
  my_free((gptr) share, MYF(0));

  return NULL;
}


/*
  Free lock controls. We call this whenever we close a table. If the table had
  the last reference to the share then we free memory associated with it.
*/
static int free_share(SPARTAN_SHARE *share)
{
  DBUG_ENTER("ha_spartan::free_share");
  pthread_mutex_lock(&spartan_mutex);
  if (!--share->use_count)
  {
    if (share->data_class != NULL)
      delete share->data_class;
    share->data_class = NULL;
    if (share->index_class != NULL)
      delete share->index_class;
    share->index_class = NULL;
    hash_delete(&spartan_open_tables, (byte*) share);
    thr_lock_delete(&share->lock);
    pthread_mutex_destroy(&share->mutex);
    my_free((gptr)share->table_name, MYF(0));
  }
  pthread_mutex_unlock(&spartan_mutex);

  DBUG_RETURN(0);
}


static handler* spartan_create_handler(TABLE_SHARE *table)
{
  return new ha_spartan(table);
}


ha_spartan::ha_spartan(TABLE_SHARE *table_arg)
  :handler(&spartan_hton, table_arg)
{
}

#define SDE_EXT ".sde"
#define SDI_EXT ".sdi"

/*
  If frm_error() is called then we will use this to to find out what file extentions
  exist for the storage engine. This is also used by the default rename_table and
  delete_table method in handler.cc.
*/
static const char *ha_spartan_exts[] = {
  SDE_EXT,
  SDI_EXT,
  NullS
};

const char **ha_spartan::bas_ext() const
{
  return ha_spartan_exts;
}

 
/*
  Used for opening tables. The name will be the name of the file.
  A table is opened when it needs to be opened. For instance
  when a request comes in for a select on the table (tables are not
  open and closed for each request, they are cached).

  Called from handler.cc by handler::ha_open(). The server opens all tables by
  calling ha_open() which then calls the handler specific open().
*/
int ha_spartan::open(const char *name, int mode, uint test_if_locked)
{
  DBUG_ENTER("ha_spartan::open");
  char name_buff[FN_REFLEN];

  if (!(share = get_share(name, table)))
    DBUG_RETURN(1);
  /*
    Call the data class open table method.
    Note: the fn_format() method correctly creates a file name from the
    name passed into the method.
  */
  share->data_class->open_table(fn_format(name_buff, name, "", SDE_EXT,
                                MY_REPLACE_EXT|MY_UNPACK_FILENAME));
  /*
    Call the data class open index method.
    Note: the fn_format() method correctly creates a file name from the
    name passed into the method.
  */
  share->index_class->open_index(fn_format(name_buff, name, "", SDI_EXT,
                                MY_REPLACE_EXT|MY_UNPACK_FILENAME));
  share->index_class->load_index();
  current_position = 0;
  thr_lock_data_init(&share->lock,&lock,NULL);
  DBUG_RETURN(0);
}


/*
  Closes a table. We call the free_share() function to free any resources
  that we have allocated in the "shared" structure.

  Called from sql_base.cc, sql_select.cc, and table.cc.
  In sql_select.cc it is only used to close up temporary tables or during
  the process where a temporary table is converted over to being a
  myisam table.
  For sql_base.cc look at close_data_tables().
*/
int ha_spartan::close(void)
{
  DBUG_ENTER("ha_spartan::close");
  share->data_class->close_table();
  share->index_class->save_index();
  share->index_class->close_index();
  DBUG_RETURN(free_share(share));
}


/*
  write_row() inserts a row. No extra() hint is given currently if a bulk load
  is happeneding. buf() is a byte array of data. You can use the field
  information to extract the data from the native byte array type.
  Example of this would be:
  for (Field **field=table->field ; *field ; field++)
  {
    ...
  }

  See ha_tina.cc for an spartan of extracting all of the data as strings.
  ha_berekly.cc has an spartan of how to store it intact by "packing" it
  for ha_berkeley's own native storage type.

  See the note for update_row() on auto_increments and timestamps. This
  case also applied to write_row().

  Called from item_sum.cc, item_sum.cc, sql_acl.cc, sql_insert.cc,
  sql_insert.cc, sql_select.cc, sql_table.cc, sql_udf.cc, and sql_update.cc.
*/
int ha_spartan::write_row(byte * buf)
{
  long long pos;
  SDE_INDEX ndx;

  DBUG_ENTER("ha_spartan::write_row");
  ha_statistic_increment(&SSV::ha_write_count);
  ndx.length = get_key_len();
  memcpy(ndx.key, get_key(), get_key_len());
  pthread_mutex_lock(&spartan_mutex);
  pos = share->data_class->write_row(buf, table->s->rec_buff_length);
  ndx.pos = pos;
  if (ndx.key != 0)
    share->index_class->insert_key(&ndx, false);
  pthread_mutex_unlock(&spartan_mutex);
  DBUG_RETURN(0);
}

byte *ha_spartan::get_key()
{
  byte *key = 0;

  DBUG_ENTER("ha_spartan::get_key");
  /*
    For each field in the table, check to see if it is the key
    by checking the key_start variable. (1 = is a key).
  */
  for (Field **field=table->field ; *field ; field++)
  {
    if ((*field)->key_start.to_ulonglong() == 1)
    {
      /*
        Copy field value to key value (save key)
      */
      key = (byte *)my_malloc((*field)->field_length, 
                                  MYF(MY_ZEROFILL | MY_WME));
      memcpy(key, (*field)->ptr, (*field)->key_length());
    }
  }
  DBUG_RETURN(key);
}

int ha_spartan::get_key_len()
{
  int length = 0;

  DBUG_ENTER("ha_spartan::get_key");
  /*
    For each field in the table, check to see if it is the key
    by checking the key_start variable. (1 = is a key).
  */
  for (Field **field=table->field ; *field ; field++)
  {
    if ((*field)->key_start.to_ulonglong() == 1)
      /*
        Copy field length to key length
      */
      length = (*field)->key_length();
  }
  DBUG_RETURN(length);
}

/*
  Yes, update_row() does what you expect, it updates a row. old_data will have
  the previous row record in it, while new_data will have the newest data in
  it.
  Keep in mind that the server can do updates based on ordering if an ORDER BY
  clause was used. Consecutive ordering is not guarenteed.
  Currently new_data will not have an updated auto_increament record, or
  and updated timestamp field. You can do these for spartan by doing these:
  if (table->timestamp_field_type & TIMESTAMP_AUTO_SET_ON_UPDATE)
    table->timestamp_field->set_time();
  if (table->next_number_field && record == table->record[0])
    update_auto_increment();

  Called from sql_select.cc, sql_acl.cc, sql_update.cc, and sql_insert.cc.
*/
int ha_spartan::update_row(const byte * old_data, byte * new_data)
{
  DBUG_ENTER("ha_spartan::update_row");
  pthread_mutex_lock(&spartan_mutex);
  share->data_class->update_row((byte *)old_data, new_data, 
                 table->s->rec_buff_length, current_position -
                 share->data_class->row_size(table->s->rec_buff_length)); 
  if (get_key() != 0)
  {
    share->index_class->update_key(get_key(), current_position -
                   share->data_class->row_size(table->s->rec_buff_length),
                   get_key_len());
    share->index_class->save_index();
    share->index_class->load_index();
  }
  pthread_mutex_unlock(&spartan_mutex);
  DBUG_RETURN(0);
}


/*
  This will delete a row. buf will contain a copy of the row to be deleted.
  The server will call this right after the current row has been called (from
  either a previous rnd_nexT() or index call).
  If you keep a pointer to the last row or can access a primary key it will
  make doing the deletion quite a bit easier.
  Keep in mind that the server does no guarentee consecutive deletions. ORDER BY
  clauses can be used.

  Called in sql_acl.cc and sql_udf.cc to manage internal table information.
  Called in sql_delete.cc, sql_insert.cc, and sql_select.cc. In sql_select it is
  used for removing duplicates while in insert it is used for REPLACE calls.
*/
int ha_spartan::delete_row(const byte * buf)
{
  long long pos;

  DBUG_ENTER("ha_spartan::delete_row");
  if (current_position > 0)
    pos = current_position -
      share->data_class->row_size(table->s->rec_buff_length);
  else
    pos = 0;
  pthread_mutex_lock(&spartan_mutex);
  share->data_class->delete_row((byte *)buf, 
                                table->s->rec_buff_length, pos);
  if (get_key() != 0)
    share->index_class->delete_key(get_key(), pos, get_key_len());
  pthread_mutex_unlock(&spartan_mutex);
  DBUG_RETURN(0);
}


/*
  Positions an index cursor to the index specified in the handle. Fetches the
  row if available. If the key value is null, begin at the first key of the
  index.
*/
int ha_spartan::index_read(byte * buf, const byte * key,
                           uint key_len __attribute__((unused)),
                           enum ha_rkey_function find_flag
                           __attribute__((unused)))
{
  long long pos;

  DBUG_ENTER("ha_spartan::index_read");
  if (key == NULL)
    pos = share->index_class->get_first_pos();
  else
    pos = share->index_class->get_index_pos((byte *)key, key_len);
  if (pos == -1)
    DBUG_RETURN(HA_ERR_KEY_NOT_FOUND);
  share->data_class->read_row(buf, table->s->rec_buff_length, pos);
  current_position = pos + share->data_class->row_size(table->s->rec_buff_length);
  share->index_class->get_next_key();
  DBUG_RETURN(0);
}


/*
  Positions an index cursor to the index specified in key. Fetches the
  row if any.  This is only used to read whole keys.
*/
int ha_spartan::index_read_idx(byte * buf, uint index, const byte * key,
                               uint key_len __attribute__((unused)),
                               enum ha_rkey_function find_flag
                               __attribute__((unused)))
{
  long long pos;

  DBUG_ENTER("ha_spartan::index_read_idx");
  pos = share->index_class->get_index_pos((byte *)key, key_len);
  if (pos == -1)
    DBUG_RETURN(HA_ERR_KEY_NOT_FOUND);
  share->data_class->read_row(buf, table->s->rec_buff_length, pos);
  DBUG_RETURN(0);
}


/*
  Used to read forward through the index.
*/
int ha_spartan::index_next(byte * buf)
{
  byte *key = 0;
  long long pos;

  DBUG_ENTER("ha_spartan::index_next");
  key = share->index_class->get_next_key();
  if (key == 0)
    DBUG_RETURN(HA_ERR_END_OF_FILE);
  pos = share->index_class->get_index_pos((byte *)key, get_key_len());
  share->index_class->seek_index(key, get_key_len());
  share->index_class->get_next_key();
  if (pos == -1)
    DBUG_RETURN(HA_ERR_KEY_NOT_FOUND);
  share->data_class->read_row(buf, table->s->rec_buff_length, pos);
  DBUG_RETURN(0);
}


/*
  Used to read backwards through the index.
*/
int ha_spartan::index_prev(byte * buf)
{
  byte *key = 0;
  long long pos;

  DBUG_ENTER("ha_spartan::index_prev");
  key = share->index_class->get_prev_key();
  if (key == 0)
    DBUG_RETURN(HA_ERR_END_OF_FILE);
  pos = share->index_class->get_index_pos((byte *)key, get_key_len());
  share->index_class->seek_index(key, get_key_len());
  share->index_class->get_prev_key();
  if (pos == -1)
    DBUG_RETURN(HA_ERR_KEY_NOT_FOUND);
  share->data_class->read_row(buf, table->s->rec_buff_length, pos);
  DBUG_RETURN(0);
}


/*
  index_first() asks for the first key in the index.

  Called from opt_range.cc, opt_sum.cc, sql_handler.cc,
  and sql_select.cc.
*/
int ha_spartan::index_first(byte * buf)
{
  byte *key = 0;

  DBUG_ENTER("ha_spartan::index_first");
  key = share->index_class->get_first_key();
  if (key == 0)
    DBUG_RETURN(HA_ERR_END_OF_FILE);
  memcpy(buf, key, get_key_len());
  DBUG_RETURN(0);
}


/*
  index_last() asks for the last key in the index.

  Called from opt_range.cc, opt_sum.cc, sql_handler.cc,
  and sql_select.cc.
*/
int ha_spartan::index_last(byte * buf)
{
  byte *key = 0;

  DBUG_ENTER("ha_spartan::index_last");
  key = share->index_class->get_last_key();
  if (key == 0)
    DBUG_RETURN(HA_ERR_END_OF_FILE);
  memcpy(buf, key, get_key_len());
  DBUG_RETURN(0);
}


/*
  rnd_init() is called when the system wants the storage engine to do a table
  scan.
  See the spartan in the introduction at the top of this file to see when
  rnd_init() is called.

  Called from filesort.cc, records.cc, sql_handler.cc, sql_select.cc, sql_table.cc,
  and sql_update.cc.
*/
int ha_spartan::rnd_init(bool scan)
{
  DBUG_ENTER("ha_spartan::rnd_init");
  current_position = 0;
  records = 0;
  ref_length = sizeof(long long);
  DBUG_RETURN(0);
}

int ha_spartan::rnd_end()
{
  DBUG_ENTER("ha_spartan::rnd_end");
  DBUG_RETURN(0);
}

/*
  This is called for each row of the table scan. When you run out of records
  you should return HA_ERR_END_OF_FILE. Fill buff up with the row information.
  The Field structure for the table is the key to getting data into buf
  in a manner that will allow the server to understand it.

  Called from filesort.cc, records.cc, sql_handler.cc, sql_select.cc, sql_table.cc,
  and sql_update.cc.
*/
int ha_spartan::rnd_next(byte *buf)
{
  int rc;

  DBUG_ENTER("ha_spartan::rnd_next"); 
  ha_statistic_increment(&SSV::ha_read_rnd_next_count);
  /*
    Read the row from the data file.
  */
  rc = share->data_class->read_row(buf, table->s->rec_buff_length,
                                   current_position); 
  if (rc != -1)
    current_position = (off_t)share->data_class->cur_position();
  else
    DBUG_RETURN(HA_ERR_END_OF_FILE);
  records++;  
  DBUG_RETURN(0);
}


/*
  position() is called after each call to rnd_next() if the data needs
  to be ordered. You can do something like the following to store
  the position:
  my_store_ptr(ref, ref_length, current_position);

  The server uses ref to store data. ref_length in the above case is
  the size needed to store current_position. ref is just a byte array
  that the server will maintain. If you are using offsets to mark rows, then
  current_position should be the offset. If it is a primary key like in
  BDB, then it needs to be a primary key.

  Called from filesort.cc, sql_select.cc, sql_delete.cc and sql_update.cc.
*/
void ha_spartan::position(const byte *record)
{
  DBUG_ENTER("ha_spartan::position"); 
  my_store_ptr(ref, ref_length, current_position);
  DBUG_VOID_RETURN;
}


/*
  This is like rnd_next, but you are given a position to use
  to determine the row. The position will be of the type that you stored in
  ref. You can use ha_get_ptr(pos,ref_length) to retrieve whatever key
  or position you saved when position() was called.
  Called from filesort.cc records.cc sql_insert.cc sql_select.cc sql_update.cc.
*/
int ha_spartan::rnd_pos(byte * buf, byte *pos)
{
  DBUG_ENTER("ha_spartan::rnd_pos");
  ha_statistic_increment(&SSV::ha_read_rnd_next_count);
  current_position = (off_t)my_get_ptr(pos,ref_length);
  share->data_class->read_row(buf, current_position, -1);
  DBUG_RETURN(0);
}


/*
  ::info() is used to return information to the optimizer.
  see my_base.h for the complete description

  Currently this table handler doesn't implement most of the fields
  really needed. SHOW also makes use of this data
  Another note, you will probably want to have the following in your
  code:
  if (records < 2)
    records = 2;
  The reason is that the server will optimize for cases of only a single
  record. If in a table scan you don't know the number of records
  it will probably be better to set records to two so you can return
  as many records as you need.
  Along with records a few more variables you may wish to set are:
    records
    deleted
    data_file_length
    index_file_length
    delete_length
    check_time
  Take a look at the public variables in handler.h for more information.

  Called in:
    filesort.cc
    ha_heap.cc
    item_sum.cc
    opt_sum.cc
    sql_delete.cc
    sql_delete.cc
    sql_derived.cc
    sql_select.cc
    sql_select.cc
    sql_select.cc
    sql_select.cc
    sql_select.cc
    sql_show.cc
    sql_show.cc
    sql_show.cc
    sql_show.cc
    sql_table.cc
    sql_union.cc
    sql_update.cc

*/
void ha_spartan::info(uint flag)
{
  DBUG_ENTER("ha_spartan::info");
  /* This is a lie, but you don't want the optimizer to see zero or 1 */
  if (records < 2) 
    records= 2;
  DBUG_VOID_RETURN;
}


/*
  extra() is called whenever the server wishes to send a hint to
  the storage engine. The myisam engine implements the most hints.
  ha_innodb.cc has the most exhaustive list of these hints.
*/
int ha_spartan::extra(enum ha_extra_function operation)
{
  DBUG_ENTER("ha_spartan::extra");
  DBUG_RETURN(0);
}


/*
  Deprecated and likely to be removed in the future. Storage engines normally
  just make a call like:
  ha_spartan::extra(HA_EXTRA_RESET);
  to handle it.
*/
int ha_spartan::reset(void)
{
  DBUG_ENTER("ha_spartan::reset");
  DBUG_RETURN(0);
}

/*
  Used to delete all rows in a table. Both for cases of truncate and
  for cases where the optimizer realizes that all rows will be
  removed as a result of a SQL statement.

  Called from item_sum.cc by Item_func_group_concat::clear(),
  Item_sum_count_distinct::clear(), and Item_func_group_concat::clear().
  Called from sql_delete.cc by mysql_delete().
  Called from sql_select.cc by JOIN::reinit().
  Called from sql_union.cc by st_select_lex_unit::exec().
*/
int ha_spartan::delete_all_rows()
{
  DBUG_ENTER("ha_spartan::delete_all_rows");
  pthread_mutex_lock(&spartan_mutex);
  share->data_class->trunc_table();
  share->index_class->destroy_index();
  share->index_class->trunc_index();
  pthread_mutex_unlock(&spartan_mutex);
  DBUG_RETURN(0);
}


/*
  First you should go read the section "locking functions for mysql" in
  lock.cc to understand this.
  This create a lock on the table. If you are implementing a storage engine
  that can handle transacations look at ha_berkely.cc to see how you will
  want to goo about doing this. Otherwise you should consider calling flock()
  here.

  Called from lock.cc by lock_external() and unlock_external(). Also called
  from sql_table.cc by copy_data_between_tables().
*/
int ha_spartan::external_lock(THD *thd, int lock_type)
{
  DBUG_ENTER("ha_spartan::external_lock");
  DBUG_RETURN(0);
}


/*
  The idea with handler::store_lock() is the following:

  The statement decided which locks we should need for the table
  for updates/deletes/inserts we get WRITE locks, for SELECT... we get
  read locks.

  Before adding the lock into the table lock handler (see thr_lock.c)
  mysqld calls store lock with the requested locks.  Store lock can now
  modify a write lock to a read lock (or some other lock), ignore the
  lock (if we don't want to use MySQL table locks at all) or add locks
  for many tables (like we do when we are using a MERGE handler).

  Berkeley DB for spartan  changes all WRITE locks to TL_WRITE_ALLOW_WRITE
  (which signals that we are doing WRITES, but we are still allowing other
  reader's and writer's.

  When releasing locks, store_lock() are also called. In this case one
  usually doesn't have to do anything.

  In some exceptional cases MySQL may send a request for a TL_IGNORE;
  This means that we are requesting the same lock as last time and this
  should also be ignored. (This may happen when someone does a flush
  table when we have opened a part of the tables, in which case mysqld
  closes and reopens the tables and tries to get the same locks at last
  time).  In the future we will probably try to remove this.

  Called from lock.cc by get_lock_data().
*/
THR_LOCK_DATA **ha_spartan::store_lock(THD *thd,
                                       THR_LOCK_DATA **to,
                                       enum thr_lock_type lock_type)
{
  if (lock_type != TL_IGNORE && lock.type == TL_UNLOCK)
    lock.type=lock_type;
  *to++= &lock; 
  return to;
}

/*
  Used to delete a table. By the time delete_table() has been called all
  opened references to this table will have been closed (and your globally
  shared references released. The variable name will just be the name of
  the table. You will need to remove any files you have created at this point.

  If you do not implement this, the default delete_table() is called from
  handler.cc and it will delete all files with the file extentions returned
  by bas_ext().

  Called from handler.cc by delete_table and  ha_create_table(). Only used
  during create if the table_flag HA_DROP_BEFORE_CREATE was specified for
  the storage engine.
*/
int ha_spartan::delete_table(const char *name)
{
  DBUG_ENTER("ha_spartan::delete_table");
  char name_buff[FN_REFLEN];

  /*
    Begin critical section by locking the spatan mutex variable.
  */
  pthread_mutex_lock(&spartan_mutex);
  if (!(share = get_share(name, table)))
    DBUG_RETURN(1);
  share->data_class->close_table();
  /*
    Destroy the index in memory and close it.
  */
  share->index_class->destroy_index();
  share->index_class->close_index();
  /*
    Call the mysql delete file method.
    Note: the fn_format() method correctly creates a file name from the
    name passed into the method.
  */
  my_delete(fn_format(name_buff, name, "", SDE_EXT,
            MY_REPLACE_EXT|MY_UNPACK_FILENAME), MYF(0));
  /*
    Call the mysql delete file method.
    Note: the fn_format() method correctly creates a file name from the
    name passed into the method.
  */
  my_delete(fn_format(name_buff, name, "", SDI_EXT,
            MY_REPLACE_EXT|MY_UNPACK_FILENAME), MYF(0));
  /*
    End critical section by unlocking the spatan mutex variable.
  */
  pthread_mutex_unlock(&spartan_mutex);
  DBUG_RETURN(0);
}

/*
  Renames a table from one name to another from alter table call.

  If you do not implement this, the default rename_table() is called from
  handler.cc and it will delete all files with the file extentions returned
  by bas_ext().

  Called from sql_table.cc by mysql_rename_table().
*/
int ha_spartan::rename_table(const char * from, const char * to)
{
  DBUG_ENTER("ha_spartan::rename_table ");
  char data_from[FN_REFLEN];
  char data_to[FN_REFLEN];
  char index_from[FN_REFLEN];
  char index_to[FN_REFLEN];

  if (!(share = get_share(from, table)))
    DBUG_RETURN(1);
  /*
    Begin critical section by locking the spatan mutex variable.
  */
  pthread_mutex_lock(&spartan_mutex);
  share->data_class->close_table();
  /*
    Close the table then copy it then reopen new file.
  */
  my_copy(fn_format(data_from, from, "", SDE_EXT,
          MY_REPLACE_EXT|MY_UNPACK_FILENAME),
          fn_format(data_to, to, "", SDE_EXT,
          MY_REPLACE_EXT|MY_UNPACK_FILENAME), MYF(0));
  share->data_class->open_table(data_to);
  share->index_class->close_index();
  /*
    Close the table then copy it then reopen new file.
  */
  my_copy(fn_format(index_from, from, "", SDI_EXT,
          MY_REPLACE_EXT|MY_UNPACK_FILENAME),
          fn_format(index_to, to, "", SDI_EXT,
          MY_REPLACE_EXT|MY_UNPACK_FILENAME), MYF(0));
  share->index_class->open_index(index_to);
  /*
    End critical section by unlocking the spatan mutex variable.
  */
  pthread_mutex_unlock(&spartan_mutex);
  /*
    Delete the file using MySQL's delete file method.
  */
  my_delete(data_from, MYF(0));
  /*
    Delete the file using MySQL's delete file method.
  */
  my_delete(index_from, MYF(0));
  DBUG_RETURN(0);
}

/*
  Given a starting key, and an ending key estimate the number of rows that
  will exist between the two. end_key may be empty which in case determine
  if start_key matches any rows.

  Called from opt_range.cc by check_quick_keys().
*/
ha_rows ha_spartan::records_in_range(uint inx, key_range *min_key,
                                     key_range *max_key)
{
  DBUG_ENTER("ha_spartan::records_in_range");
  DBUG_RETURN(10);                         // low number to force index usage
}


/*
  create() is called to create a database. The variable name will have the name
  of the table. When create() is called you do not need to worry about opening
  the table. Also, the FRM file will have already been created so adjusting
  create_info will not do you any good. You can overwrite the frm file at this
  point if you wish to change the table definition, but there are no methods
  currently provided for doing that.

  Called from handle.cc by ha_create_table().
*/
int ha_spartan::create(const char *name, TABLE *table_arg,
                       HA_CREATE_INFO *create_info)
{
  DBUG_ENTER("ha_spartan::create");
  char name_buff[FN_REFLEN];

  if (!(share = get_share(name, table)))
    DBUG_RETURN(1);
  /*
    Call the data class create table method.
    Note: the fn_format() method correctly creates a file name from the
    name passed into the method.
  */
  if (share->data_class->create_table(fn_format(name_buff, name, "", SDE_EXT,
                                      MY_REPLACE_EXT|MY_UNPACK_FILENAME)))
    DBUG_RETURN(-1);
  /*
    Call the data class create index method.
    Note: the fn_format() method correctly creates a file name from the
    name passed into the method.
  */
  if (share->index_class->create_index(fn_format(name_buff, name, "", SDI_EXT,
                                      MY_REPLACE_EXT|MY_UNPACK_FILENAME),
                                      128))
    DBUG_RETURN(-1);
  share->index_class->close_index();
  share->data_class->close_table();
  DBUG_RETURN(0);
}

#ifdef MYSQL_PLUGIN
mysql_declare_plugin
{
  MYSQL_STORAGE_ENGINE_PLUGIN,
  &spartan_hton,
  spartan_hton.name,
  "Dr. Bell",
  "Spartan Storage Engine -- Expert MySQL",
  spartan_init_func, /* Plugin Init */
  spartan_done_func, /* Plugin Deinit */
  0x0001 /* 0.1 */,
}
mysql_declare_plugin_end;
#endif
