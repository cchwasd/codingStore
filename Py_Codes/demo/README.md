# 组件注册机制 (Component Registry)

一个功能强大的Python组件注册机制实现，支持版本管理、热插拔、类型安全等特性。

## 主要特性

### 1. 全局注册表设计
- 使用单例模式实现全局注册表
- 线程安全的实现
- 支持多版本并存
- 统一的组件管理入口

### 2. 版本兼容机制
- 支持语义化版本号 (major.minor.patch)
- 可以同时维护多个版本的组件
- 支持按版本号获取特定版本的组件
- 版本比较和排序功能

### 3. 装饰器注册机制
- 使用 `@registry.register()` 装饰器简化注册过程
- 支持自定义组件名称
- 支持版本号指定
- 支持元数据添加
- 支持组件状态设置

### 4. 动态实例化
- 自动实例化组件
- 单例模式管理组件实例
- 使用弱引用避免内存泄漏
- 延迟加载机制

### 5. 类型安全
- 完整的类型注解支持
- 使用泛型实现类型安全
- 运行时类型检查
- Protocol 接口定义

### 6. 热插拔支持
- 动态启用/禁用组件
- 组件状态管理 (ENABLED/DISABLED/DEPRECATED)
- 运行时组件替换
- 组件生命周期管理

### 7. 线程安全
- 使用 RLock 实现线程安全
- 所有关键操作都在锁的保护下执行
- 支持并发访问和修改
- 避免竞态条件

### 8. 日志跟踪
- 完整的日志记录
- 记录组件注册/获取操作
- 记录状态变更
- 错误追踪和调试支持

## 使用示例

### 基本用法

```python
from registry import registry, ComponentStatus

# 注册组件
@registry.register(version="1.0.0", metadata={"category": "text"})
class TextProcessor:
    def __init__(self):
        self.name = "TextProcessor"

    def process(self, data: str) -> str:
        return data.upper()

# 获取组件
processor = registry.get("TextProcessor", version="1.0.0")
result = processor.process("hello")  # 返回 "HELLO"

# 禁用组件
registry.set_status("TextProcessor", ComponentStatus.DISABLED)
```

### 高级用法

```python
# 按类型获取组件
text_processors = registry.get_by_type(TextProcessor)

# 获取所有版本
all_versions = registry.get_all_versions("TextProcessor")

# 列出所有启用的组件
enabled_components = registry.list_components(ComponentStatus.ENABLED)

# 注销组件
registry.unregister("TextProcessor")
```

## 组件状态

组件支持三种状态：
- `ENABLED`: 组件可用
- `DISABLED`: 组件被禁用
- `DEPRECATED`: 组件已废弃

## 最佳实践

1. **组件设计**
   - 为组件添加清晰的接口定义
   - 使用 Protocol 定义组件接口
   - 实现适当的初始化方法
   - 添加必要的元数据

2. **版本管理**
   - 遵循语义化版本规范
   - 为每个组件指定合适的版本号
   - 在升级时保持向后兼容

3. **错误处理**
   - 正确处理组件获取失败的情况
   - 使用日志记录关键操作
   - 实现适当的错误恢复机制

4. **性能优化**
   - 合理使用组件实例化
   - 避免不必要的组件创建
   - 及时清理不需要的组件

## 注意事项

1. **线程安全**
   - 所有组件操作都是线程安全的
   - 避免在组件内部维护全局状态
   - 注意并发访问的性能影响

2. **内存管理**
   - 使用弱引用避免内存泄漏
   - 及时注销不再使用的组件
   - 注意组件实例的生命周期

3. **类型安全**
   - 使用类型注解提高代码可维护性
   - 运行时检查组件接口实现
   - 注意泛型类型的使用

4. **版本兼容**
   - 注意版本号的管理
   - 保持向后兼容性
   - 合理处理版本升级

## 扩展建议

1. **配置管理**
   - 添加组件配置管理
   - 支持配置热重载
   - 实现配置验证

2. **依赖管理**
   - 添加组件依赖管理
   - 支持依赖注入
   - 处理循环依赖

3. **监控和统计**
   - 添加性能监控
   - 统计组件使用情况
   - 实现健康检查

4. **自动发现**
   - 实现组件自动发现
   - 支持插件机制
   - 动态加载组件

## 贡献指南

欢迎提交 Issue 和 Pull Request 来改进这个项目。在提交代码时，请确保：

1. 添加适当的测试
2. 更新文档
3. 遵循代码风格指南
4. 添加必要的类型注解

## 许可证

MIT License  