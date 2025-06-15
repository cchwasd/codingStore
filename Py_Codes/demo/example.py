from typing import Protocol, runtime_checkable
from registry import registry, ComponentStatus, Version

# 定义组件接口
# @runtime_checkable 装饰器允许在运行时检查一个类是否实现了某个协议
# 这使得我们可以使用 isinstance() 来检查对象是否实现了 Component 协议
@runtime_checkable
class Component(Protocol):
    # Protocol 是一个特殊的类，用于定义接口/协议
    # 这里定义了一个必须实现的方法 process
    # 参数 data 类型为 str，返回值类型为 str
    # ... 表示这是一个抽象方法，具体实现由子类提供
    def process(self, data: str) -> str:
        ...

# 实现一些组件
@registry.register(version="1.0.0", metadata={"category": "text"})
class TextProcessor:
    def process(self, data: str) -> str:
        return data.upper()

@registry.register(version="1.1.0", metadata={"category": "text"})
class AdvancedTextProcessor:
    def process(self, data: str) -> str:
        return data.lower()

@registry.register(name="special_processor", version="2.0.0", metadata={"category": "special"})
class SpecialProcessor:
    def process(self, data: str) -> str:
        return f"*** {data} ***"

def main():
    # 获取组件示例
    processor = registry.get("TextProcessor", version="1.0.0")
    if processor:
        print(processor.process("hello"))  # 输出: HELLO

    # 获取最新版本
    latest_processor = registry.get("TextProcessor")
    if latest_processor:
        print(latest_processor.process("hello"))  # 输出: hello

    # 按类型获取组件
    text_processors = registry.get_by_type(TextProcessor)
    for name, entry in text_processors.items():
        print(f"Found text processor: {name} (version {entry.version})")

    # 禁用组件
    registry.set_status("TextProcessor", ComponentStatus.DISABLED)
    
    # 尝试获取已禁用的组件
    disabled_processor = registry.get("TextProcessor")
    print(f"Disabled processor available: {disabled_processor is not None}")  # 输出: False

    # 列出所有启用的组件
    enabled_components = registry.list_components(ComponentStatus.ENABLED)
    print("\nEnabled components:")
    for name, entry in enabled_components.items():
        print(f"- {name} (version {entry.version}, category: {entry.metadata.get('category')})")

    # 获取组件的所有版本
    all_versions = registry.get_all_versions("TextProcessor")
    print("\nAll versions of TextProcessor:")
    for version, entry in all_versions.items():
        print(f"- Version {version}: {entry.status.value}")

if __name__ == "__main__":
    main() 