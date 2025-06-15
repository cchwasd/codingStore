
class Register(dict):
    def __init__(self, *args, **kwargs):
        super(Register, self).__init__(*args, **kwargs)
        self._dict = {}

    def __call__(self, target):
        return self.register(target)

    def register(self, target):
        def add_item(key, value):
            if not callable(value):
                raise Exception(f"Error:{value} must be callable!")
            if key in self._dict:
                print(f"\033[31mWarning:\033[0m {value.__name__} already exists and will be overwritten!")
            self[key] = value
            return value

        if callable(target):    # 传入的target可调用 --> 没有给注册名 --> 传入的函数名或类名作为注册名
            return add_item(target.__name__, target)
        else:                   # 不可调用 --> 传入了注册名 --> 作为可调用对象的注册名 
            return lambda x : add_item(target, x)

    def __setitem__(self, key, value):
        self._dict[key] = value

    def __getitem__(self, key):
        return self._dict[key]

    def __contains__(self, key):
        return key in self._dict

    def __str__(self):
        return str(self._dict)

    def keys(self):
        return self._dict.keys()

    def values(self):
        return self._dict.values()

    def items(self):
        return self._dict.items()

def test_reg():
    # 测试装饰器注册功能
    print("测试装饰器注册功能:")
    
    register_func = Register()

    # 测试基本函数注册
    @register_func
    def add(a, b):
        return a + b

    @register_func
    def multiply(a, b):
        return a * b

    @register_func('matrix multiply')
    def multiply(a, b):
        pass

    @register_func
    def minus(a, b):
        return a - b
    
    
    # 测试带自定义名称的函数注册
    @register_func("custom_name")
    def test_multiply(a, b):
        return a * b
    
    # 测试重复注册（应该会显示警告）
    @register_func
    def test_add(a, b):
        return a + b + 1
    
    # 测试注册的函数调用
    print("\n测试注册的函数调用:")
    print(f"test_add(1, 2) = {register_func['test_add'](1, 2)}")
    print(f"test_add(1, 2) = {register_func['test_add'](1, 2)}")
    print(f"custom_name(2, 3) = {register_func['custom_name'](2, 3)}")
    
    # 测试字典方法
    print("\n测试字典方法:")
    print(f"所有键: {list(register_func.keys())}")
    print(f"所有值: {list(register_func.values())}")
    print(f"键值对: {list(register_func.items())}")
    
    # 测试键存在性检查
    print("\n测试键存在性检查:")
    print(f"'test_add' 是否存在: {'test_add' in register_func}")
    print(f"'not_exist' 是否存在: {'not_exist' in register_func}")


    for k, v in register_func.items():
        print(f"key: {k}, value: {v}")
    # output:
    # key: add, value: <function add at 0x7fdedd53cb90>
    # key: multiply, value: <function multiply at 0x7fdedd540200>
    # key: matrix multiply, value: <function multiply at 0x7fdedd5403b0>
    # key: minus, value: <function minus at 0x7fdedd540320>


if __name__ == '__main__':
    test_reg()
