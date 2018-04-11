# phpco
## extension for php corotinue

- 正常抛异常（测试通过）
- 多进程支持 (测试通过)
- 单协程多fd唤醒可写测试 (测试通过)
- socket资源var_dump的id释放(done)
- socket需要超时控制(done)

- 对象操作支持(redis,mysql),待实现
- timeout设定,invoke需要把持久化的timeout进行更新、将非持久化的删掉
