from typing import TypeVar, Generic, Dict, Any, Optional, Callable, Type, Union
import threading
import logging
from datetime import datetime
from enum import Enum
import weakref
from functools import wraps
import inspect

# 设置日志
logging.basicConfig(level=logging.INFO, format='%(asctime)s - %(name)s - %(levelname)s - %(message)s')
logger = logging.getLogger(__name__)

T = TypeVar('T')    # 组件类型

class ComponentStatus(Enum):
    """组件状态枚举类
    
    定义了组件可能的状态:
    - ENABLED: 启用状态
    - DISABLED: 禁用状态
    - DEPRECATED: 废弃状态
    """
    ENABLED = "enabled"
    DISABLED = "disabled"
    DEPRECATED = "deprecated"


class Version:
    """版本类
    
    用于管理组件的版本号,包含主版本号、次版本号和补丁版本号。
    
    Attributes:
        major (int): 主版本号
        minor (int): 次版本号
        patch (int): 补丁版本号
    """
    def __init__(self, major: int, minor: int, patch: int):
        """
        Args:
            major: 主版本号
            minor: 次版本号
            patch: 补丁版本号
        """
        self.major = major
        self.minor = minor
        self.patch = patch

    def __str__(self) -> str:
        """返回版本号的字符串表示
        
        Returns:
            str: 格式为 "major.minor.patch" 的版本号字符串
        """
        return f"{self.major}.{self.minor}.{self.patch}"

    def __lt__(self, other: 'Version') -> bool:
        """比较两个版本号的大小
        
        Args:
            other: 要比较的另一个版本号对象
            
        Returns:
            bool: 如果当前版本号小于other则返回True
        """
        return (self.major, self.minor, self.patch) < (other.major, other.minor, other.patch)

class RegistryEntry(Generic[T]):
    """注册表项类
    
    用于存储组件注册的详细信息,包括组件类、版本、状态等。
    
    Attributes:
        component_class: 组件类
        version: 组件版本
        status: 组件状态
        metadata: 组件元数据
        created_at: 创建时间
        last_accessed: 最后访问时间
    """
    def __init__(
        self,
        component_class: Type[T],
        version: Version,
        status: ComponentStatus = ComponentStatus.ENABLED,
        metadata: Optional[Dict[str, Any]] = None
    ):
        """
        Args:
            component_class: 组件类
            version: 组件版本
            status: 组件状态,默认为启用
            metadata: 组件元数据,默认为空字典
        """
        self.component_class = component_class  
        self.version = version
        self.status = status
        self.metadata = metadata or {}
        self.created_at = datetime.now()
        self.last_accessed = datetime.now()
        self._instance: Optional[T] = None

    @property
    def component(self) -> T:
        """获取组件实例
        
        如果组件实例不存在则创建新实例。
        
        Returns:
            T: 组件实例
        """
        if self._instance is None:
            self._instance = self.component_class()
        return self._instance

class GlobalRegistry(Generic[T]):
    """全局注册表类
    
    用于管理所有组件的注册、获取和状态管理。
    实现了单例模式,确保全局只有一个注册表实例。
    
    Attributes:
        _instance: 单例实例
        _lock: 线程锁
    """
    _instance = None
    _lock = threading.RLock()

    def __new__(cls):
        """创建或获取单例实例
        
        Returns:
            GlobalRegistry: 注册表单例实例
        """
        with cls._lock:
            if cls._instance is None:
                cls._instance = super(GlobalRegistry, cls).__new__(cls)
                cls._instance._initialize()
            return cls._instance

    def _initialize(self):
        """初始化注册表的内部数据结构"""
        self._registry: Dict[str, RegistryEntry[T]] = {}
        self._version_registry: Dict[str, Dict[Version, RegistryEntry[T]]] = {}
        self._type_registry: Dict[Type, Dict[str, RegistryEntry[T]]] = {}
        self._weak_refs = weakref.WeakValueDictionary() # 弱引用字典

    def register(
        self,
        name: Optional[str] = None,
        version: Optional[Union[str, Version]] = None,
        status: ComponentStatus = ComponentStatus.ENABLED,
        metadata: Optional[Dict[str, Any]] = None,
        component_type: Optional[Type] = None
    ) -> Callable:
        """注册组件装饰器
        
        用于注册组件到注册表中。可以作为装饰器使用。
        
        Args:
            name: 组件名称,默认为组件类名
            version: 组件版本,可以是字符串或Version对象,默认为"1.0.0"
            status: 组件状态,默认为启用
            metadata: 组件元数据
            component_type: 组件类型,默认为组件类本身
            
        Returns:
            Callable: 装饰器函数
        """
        def decorator(target: Type[T]) -> Type[T]:
            nonlocal name, version, component_type
            
            if name is None:
                name = target.__name__
            
            if version is None:
                version = Version(1, 0, 0)
            elif isinstance(version, str):
                major, minor, patch = map(int, version.split('.'))
                version = Version(major, minor, patch)

            if component_type is None:
                component_type = target

            entry = RegistryEntry(
                component_class=target,
                version=version,
                status=status,
                metadata=metadata
            )

            with self._lock:
                # 注册到主注册表
                self._registry[name] = entry
                
                # 注册到版本注册表
                if name not in self._version_registry:
                    self._version_registry[name] = {}
                self._version_registry[name][version] = entry
                
                # 注册到类型注册表
                if component_type not in self._type_registry:
                    self._type_registry[component_type] = {}
                self._type_registry[component_type][name] = entry

                # 保存弱引用
                self._weak_refs[name] = target

            logger.info(f"Registered component '{name}' (version {version}) with status {status}")
            return target

        return decorator

    def get(
        self,
        name: str,
        version: Optional[Union[str, Version]] = None,
        component_type: Optional[Type] = None
    ) -> Optional[T]:
        """获取组件实例
        
        根据名称、版本和类型获取组件实例。
        
        Args:
            name: 组件名称
            version: 组件版本,可以是字符串或Version对象
            component_type: 组件类型
            
        Returns:
            Optional[T]: 组件实例,如果未找到则返回None
        """
        with self._lock:
            try:
                if version is not None:
                    if isinstance(version, str):
                        major, minor, patch = map(int, version.split('.'))
                        version = Version(major, minor, patch)
                    
                    if name in self._version_registry and version in self._version_registry[name]:
                        entry = self._version_registry[name][version]
                        if entry.status == ComponentStatus.ENABLED:
                            entry.last_accessed = datetime.now()
                            logger.info(f"Retrieved component '{name}' version {version}")
                            return entry.component
                else:
                    if component_type is not None and component_type in self._type_registry:
                        if name in self._type_registry[component_type]:
                            entry = self._type_registry[component_type][name]
                            if entry.status == ComponentStatus.ENABLED:
                                entry.last_accessed = datetime.now()
                                logger.info(f"Retrieved component '{name}' of type {component_type.__name__}")
                                return entry.component
                    elif name in self._registry:
                        entry = self._registry[name]
                        if entry.status == ComponentStatus.ENABLED:
                            entry.last_accessed = datetime.now()
                            logger.info(f"Retrieved component '{name}'")
                            return entry.component
            except Exception as e:
                logger.error(f"Error retrieving component '{name}': {str(e)}")
                return None

    def set_status(self, name: str, status: ComponentStatus) -> bool:
        """设置组件状态
        
        Args:
            name: 组件名称
            status: 要设置的状态
            
        Returns:
            bool: 设置是否成功
        """
        with self._lock:
            if name in self._registry:
                self._registry[name].status = status
                logger.info(f"Set status of component '{name}' to {status}")
                return True
            return False

    def get_all_versions(self, name: str) -> Dict[Version, RegistryEntry[T]]:
        """获取组件的所有版本信息
        
        返回指定名称组件的所有已注册版本及其对应的注册表项。
        
        Args:
            name: 组件名称
            
        Returns:
            Dict[Version, RegistryEntry[T]]: 版本号到注册表项的映射字典,
                如果组件不存在则返回空字典
        """
        return self._version_registry.get(name, {})

    def get_by_type(self, component_type: Type) -> Dict[str, RegistryEntry[T]]:
        """按类型获取组件
        
        返回指定类型的所有组件及其注册表项。
        
        Args:
            component_type: 组件类型
            
        Returns:
            Dict[str, RegistryEntry[T]]: 组件名称到注册表项的映射字典,
                如果该类型没有注册组件则返回空字典
        """
        return self._type_registry.get(component_type, {})

    def unregister(self, name: str) -> bool:
        """注销组件
        
        从注册表中完全移除指定名称的组件,包括所有版本和类型注册信息。
        同时清理相关的弱引用。
        
        Args:
            name: 要注销的组件名称
            
        Returns:
            bool: 注销是否成功,如果组件不存在则返回False
            
        Note:
            此操作会同时从版本注册表和类型注册表中移除组件
        """
        with self._lock:
            if name in self._registry:
                del self._registry[name]
                if name in self._version_registry:
                    del self._version_registry[name]
                for type_dict in self._type_registry.values():
                    if name in type_dict:
                        del type_dict[name]
                if name in self._weak_refs:
                    del self._weak_refs[name]
                logger.info(f"Unregistered component '{name}'")
                return True
            return False

    def list_components(self, status: Optional[ComponentStatus] = None) -> Dict[str, RegistryEntry[T]]:
        """列出组件
        
        获取所有注册的组件,可以按状态进行过滤。
        
        Args:
            status: 可选的组件状态过滤器,如果为None则返回所有组件
            
        Returns:
            Dict[str, RegistryEntry[T]]: 组件名称到注册表项的映射字典
            
        Note:
            返回的是注册表的副本,修改返回值不会影响原始注册表
        """
        with self._lock:
            if status is None:
                return self._registry.copy()
            return {k: v for k, v in self._registry.items() if v.status == status}


# 创建全局注册表实例
registry = GlobalRegistry()