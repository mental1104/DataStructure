import sys
import os
import ctypes

# 尝试加载 libbst.so
try:
    bst_lib = ctypes.CDLL("../../lib/libbst.so")
    print(f"✅ Successfully loaded: 'libbst.so'")
except OSError as e:
    print(f"❌ Failed to load libbst.so: {e}")
    sys.exit(1)

# 假设 C++ 代码里有 `void hello();`
try:
    bst_lib.hello()
except AttributeError:
    print("❌ Function `hello` not found in libbst.so")
