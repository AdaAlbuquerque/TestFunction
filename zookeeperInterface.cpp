
zooKeeper.h

连接状态

extern ZOOAPI const int ZOO_EXPIRED_SESSION_STATE;
extern ZOOAPI const int ZOO_AUTH_FAILED_STATE;
extern ZOOAPI const int ZOO_CONNECTING_STATE;
extern ZOOAPI const int ZOO_ASSOCIATING_STATE;
extern ZOOAPI const int ZOO_CONNECTED_STATE;

事件类型
extern ZOOAPI const int ZOO_CREATED_EVENT;
extern ZOOAPI const int ZOO_DELETED_EVENT;
extern ZOOAPI const int ZOO_CHANGED_EVENT;
extern ZOOAPI const int ZOO_CHILD_EVENT;
extern ZOOAPI const int ZOO_SESSION_EVENT;
extern ZOOAPI const int ZOO_NOTWATCHING_EVENT;

错误码
  ZSYSTEMERROR = -1,
  ZRUNTIMEINCONSISTENCY = -2, /*!< A runtime inconsistency was found */
  ZDATAINCONSISTENCY = -3, /*!< A data inconsistency was found */
  ZCONNECTIONLOSS = -4, /*!< Connection to the server has been lost */
  ZMARSHALLINGERROR = -5, /*!< Error while marshalling or unmarshalling data */
  ZUNIMPLEMENTED = -6, /*!< Operation is unimplemented */
  ZOPERATIONTIMEOUT = -7, /*!< Operation timeout */
  ZBADARGUMENTS = -8, /*!< Invalid arguments */
  ZINVALIDSTATE = -9, /*!< Invliad zhandle state */

  ZAPIERROR = -100,
  ZNONODE = -101, /*!< Node does not exist */
  ZNOAUTH = -102, /*!< Not authenticated */
  ZBADVERSION = -103, /*!< Version conflict */
  ZNOCHILDRENFOREPHEMERALS = -108, /*!< Ephemeral nodes may not have children */
  ZNODEEXISTS = -110, /*!< The node already exists */
  ZNOTEMPTY = -111, /*!< The node has children */
  ZSESSIONEXPIRED = -112, /*!< The session has been expired by the server */
  ZINVALIDCALLBACK = -113, /*!< Invalid callback specified */
  ZINVALIDACL = -114, /*!< Invalid ACL specified */
  ZAUTHFAILED = -115, /*!< Client authentication failed */
  ZCLOSING = -116, /*!< ZooKeeper is closing */
  ZNOTHING = -117, /*!< (not error) no server responses to process */
  ZSESSIONMOVED = -118 /*!<session moved to another server, so operation is ignored */ 

ACL权限控制
extern ZOOAPI const int ZOO_PERM_READ;
extern ZOOAPI const int ZOO_PERM_WRITE;
extern ZOOAPI const int ZOO_PERM_CREATE;
extern ZOOAPI const int ZOO_PERM_DELETE;
extern ZOOAPI const int ZOO_PERM_ADMIN;
extern ZOOAPI const int ZOO_PERM_ALL;

extern ZOOAPI struct Id ZOO_ANYONE_ID_UNSAFE;
extern ZOOAPI struct Id ZOO_AUTH_IDS;
extern ZOOAPI struct ACL_vector ZOO_OPEN_ACL_UNSAFE;
extern ZOOAPI struct ACL_vector ZOO_OPEN_ACL_UNSAFE;
extern ZOOAPI struct ACL_vector ZOO_CREATOR_ALL_ACL;

watcher 回调
typedef void (*watcher_fn)(zhandle_t *zh, int type, 
        int state, const char *path,void *watcherCtx);

ZOOAPI zhandle_t *zookeeper_init(const char *host, watcher_fn fn,
  int recv_timeout, const clientid_t *clientid, void *context, int flags);

ZOOAPI int zookeeper_close(zhandle_t *zh);

ZOOAPI const void *zoo_get_context(zhandle_t *zh);
ZOOAPI void zoo_set_context(zhandle_t *zh, void *context);

ZOOAPI watcher_fn zoo_set_watcher(zhandle_t *zh,watcher_fn newFn);
ZOOAPI struct sockaddr* zookeeper_get_connected_host(zhandle_t *zh,
        struct sockaddr *addr, socklen_t *addr_len);

ZOOAPI int zookeeper_interest(zhandle_t *zh, int *fd, int *interest, 
	struct timeval *tv);



同步接口
ZOOAPI int zoo_create(zhandle_t *zh, const char *path, const char *value,
        int valuelen, const struct ACL_vector *acl, int flags,
        char *path_buffer, int path_buffer_len);

ZOOAPI int zoo_exists(zhandle_t *zh, const char *path, int watch, struct Stat *stat);
ZOOAPI int zoo_wexists(zhandle_t *zh, const char *path,
        watcher_fn watcher, void* watcherCtx, struct Stat *stat);

ZOOAPI int zoo_get(zhandle_t *zh, const char *path, int watch, char *buffer,   
                   int* buffer_len, struct Stat *stat);
ZOOAPI int zoo_wget(zhandle_t *zh, const char *path, 
        watcher_fn watcher, void* watcherCtx, 
        char *buffer, int* buffer_len, struct Stat *stat);

ZOOAPI int zoo_delete(zhandle_t *zh, const char *path, int version);

ZOOAPI int zoo_get_children(zhandle_t *zh, const char *path, int watch,
                            struct String_vector *strings);


异步接口

typedef void (*void_completion_t)(int rc, const void *data);

ZOOAPI int zoo_aget(zhandle_t *zh, const char *path, int watch, 
        data_completion_t completion, const void *data);

ZOOAPI int zoo_awexists(zhandle_t *zh, const char *path, 
        watcher_fn watcher, void* watcherCtx, 
        stat_completion_t completion, const void *data);

ZOOAPI int zoo_aexists(zhandle_t *zh, const char *path, int watch, 
        stat_completion_t completion, const void *data);

ZOOAPI struct sockaddr* zookeeper_get_connected_host(zhandle_t *zh,
        struct sockaddr *addr, socklen_t *addr_len);

ZOOAPI int zoo_acreate(zhandle_t *zh, const char *path, const char *value, 
        int valuelen, const struct ACL_vector *acl, int flags,
        string_completion_t completion, const void *data);

ZOOAPI int zoo_adelete(zhandle_t *zh, const char *path, int version, 
        void_completion_t completion, const void *data);

zookeeper.apache.org

集群管理
Leader选举
分布式锁 （watcher实时性差）
队列（watcher 实时性差）


配置管理

配置信息写入Node Value，统一更新。

