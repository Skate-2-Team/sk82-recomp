import idaapi
import idautils
import idc

def get_function_sizes(addresses_within_functions):
    result = []
    seen_functions = set()
    
    for addr in addresses_within_functions:
        func = idaapi.get_func(addr)
        if func:
            if func.start_ea not in seen_functions:
                size = func.end_ea - func.start_ea

                result.append({
                    "address": func.start_ea,
                    "size": size
                })

                seen_functions.add(func.start_ea)
        else:
            print(f"Warning: No function found containing address 0x{addr:X}")
    
    return result

def print_functions_table(funcs):
    print("CLEARING \n\n")

    print("functions = [")
    for func in funcs:
        print(f"    {{ address = 0x{func['address']:X}, size = 0x{func['size']:X} }},")
    print("]")

def main():
    addresses_within_functions = [
        # FUNCTIONS FROM "JMPTABLEPARSER.PY" GO HERE.
    ]
    
    functions = get_function_sizes(addresses_within_functions)
    print_functions_table(functions)

if __name__ == '__main__':
    main()