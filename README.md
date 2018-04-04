# phpco
## extension for php corotinue

- 正常抛异常（测试通过）
- 多进程支持 (测试通过)
- 单协程多fd唤醒可写测试 (测试通过)
- socket资源var_dump的id释放(done)
- socket需要超时控制(done)

- 对象操作支持(redis,mysql),待实现
st_mysqlnd_connection_data dbh->server->data->vio->data->stream

stmt->dbh
pdo
pdo_dbh_object_t *dbh_obj = Z_PDO_OBJECT_P(obj);
pdo_dbh_t *dbh = dbh_obj->inner;  
