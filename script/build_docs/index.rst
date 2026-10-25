toylib
======

用C实现的一组基础数据结构, 接口都在 ``include/toylib`` 下, 每个函数都注明了参数约束和可能返回的错误码。

所有容器遵循同一套约定:

- 创建或初始化返回 ``ToylibError``, 失败时不改动传入的结构体字段
- 容量在创建时确定且不可增长, 上限是各模块的 ``MAX_CAPACITY`` / ``MAX_SIZE``
- 元素类型和相等判定通过宏在编译时替换, 默认是 ``int``

.. 下面这个 toctree 的条目由 script/build_docs/build_docs.py 按头文件自动填充, 不用手改

.. toctree::
   :maxdepth: 2
   :caption: 模块

   @MODULES@
