import re

# Load linker error text
with open("linker_errors.txt", "r") as f:
    data = f.read()

# Extract __imp__ symbols
symbol_pattern = re.compile(r'__imp__(\w+)')
symbols = sorted(set(symbol_pattern.findall(data)))

# Buffers for output
function_stubs = []
hook_macros = []

# Generate each stub and hook macro
for name in symbols:
    stub_name = f"Import_{name}"
    function_stubs.append(f"void {stub_name}()\n{{\n    Log::Stub(\"{name}\", \"Called.\");\n}}\n")
    hook_macros.append(f"GUEST_FUNCTION_HOOK(__imp__{name}, Hooks::{stub_name})")

# Output the stub functions
for stub in function_stubs:
    print(stub)

# Output the hooks at the bottom
print("\n// Hook bindings")
for hook in hook_macros:
    print(hook)
