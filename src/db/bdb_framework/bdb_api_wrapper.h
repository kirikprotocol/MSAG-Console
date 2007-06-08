#ifndef __BDB_API_WRAPPER_H__
# define __BDB_API_WRAPPER_H__ 1

# include <db.h>

# ifdef __cplusplus
extern "C" {
# endif

/*
** all standalone functions is declared in this header is wrappers for the Berkeley Db API.
*/
const char* get_error_message();

int open_db(DB **dbpp,
            DB_ENV *dbEnv,
            const char *file_name,
            int is_secondary=0);
int close_db(DB *dbp);

DB_ENV *db_setup();
void close_env(DB_ENV *dbEnv);

/*
** Insert key/value pair into db.
** Return value:
** 1 - record was inserted
** 0 - value can't be inserted because of dup index value existence
** < 0 - error occured
*/
int insert_record(DB *dbp,
                  const void* key, size_t keySize,
                  const void* value, size_t valueSize);

/*
** Find value by primary key.
** Return value:
** 1 - record found
** 0 - record not found
** < 0 - error occured
*/
int find_at_most_one_record_by_key(DB *dbp,
                                   const void* key, size_t keySize,
                                   void* value, size_t maxValueSize, size_t* realValueSize);

/*
** Fetch first record by nonuniq key value
** Return value:
** 1 - record found
** 0 - record not found
** < 0 - error occured
*/
int fetch_first_record_by_key(DB *secondary_dbp,
                              DBC **cursorpp,
                              const void* key, size_t keySize,
                              void* value, size_t maxValueSize, size_t* realValueSize);


/*
** Fetch next record by nonuniq key value
** Return value:
** 1 - record found
** 0 - record not found
** < 0 - error occured
*/
int fetch_next_record_by_key(DB *secodary_dbp,
                             DBC **cursorpp,
                             const void* key, size_t keySize,
                             void* value, size_t maxValueSize, size_t* realValueSize);

/*
** Delete key/value pair by primary key
** Return value:
** 1 - record was deleted
** 0 - record not found
** < 0 - error occured
*/
int delete_record_by_key(DB *dbp,
                         const void* key, size_t keySize);

/*
** Update record data value by primary key
** Return value:
** 1 - record was updated
** 0 - record not found
** < 0 - error occured
*/
int update_record_by_primary_key(DB *dbp,
                                 const void* key, size_t keySize,
                                 const void* value, size_t valueSize);


typedef int (*index_key_creator_func_t)(DB *secondary,
                                        const DBT *primary_key,
                                        const DBT *record_data,
                                        DBT *secondary_key);

int create_index(DB **idx_dbpp,
                 DB *dbp,
                 DB_ENV *dbEnv,
                 const char *idxFileName,
                 index_key_creator_func_t callback);


/*
** Position cursor for first record with specified key. Used by join query realization.
**** Return value:
** 1 - cursor was positioned to record with specified key
** 0 - record not found
** < 0 - error occured
*/
int position_cursor_to_first_record(DB *secondary_dbp,
                                    DBC **cursorpp,
                                    const void* key, size_t keySize);


int iterate_over_join(DBC* join_curs,
                      void* key, size_t maxKeySize, size_t* realKeySize,
                      void* value, size_t maxValueSize, size_t* realValueSize);

# ifdef __cplusplus
}
# endif

#endif
