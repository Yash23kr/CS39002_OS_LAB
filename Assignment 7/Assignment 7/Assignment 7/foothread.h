#define FOOTHREAD_DEFAULT_STACK_SIZE 2097152
#define FOOTHREAD_JOINABLE 0
#define FOOTHREAD_DETACHED 1
typedef struct foothread_t foothread_t;
typedef struct foothread_attr_t foothread_attr_t;
typedef struct foothread_mutex_t foothread_mutex_t;
struct foothread_t {
    int id;
    int stack_size;
    int joinable;
};
struct foothread_attr_t {
    int join_type;
    int stack_size;
};
typedef struct foothread_barrier_t foothread_barrier_t;
struct foothread_barrier_t
{
    int semid;
    int count;
    int orig_count;
};
struct foothread_mutex_t
{
    int semid;
    int tid;
};

#define FOOTHREAD_ATTR_INIIALIZER {0, FOOTHREAD_DEFAULT_STACK_SIZE}
void foothread_create ( foothread_t * , foothread_attr_t * , int (*)(void *) , void * ) ;
void foothread_attr_setjointype ( foothread_attr_t * , int ) ;
void foothread_exit ( ) ;
void foothread_attr_setstacksize ( foothread_attr_t * , int ) ;
void foothread_mutex_init ( foothread_mutex_t * ) ;
void foothread_mutex_lock ( foothread_mutex_t * ) ;
void foothread_mutex_unlock ( foothread_mutex_t * ) ;
void foothread_mutex_destroy ( foothread_mutex_t * ) ;
void foothread_barrier_init ( foothread_barrier_t * , int ) ;
void foothread_barrier_wait ( foothread_barrier_t * ) ;
void foothread_barrier_destroy ( foothread_barrier_t * ) ;