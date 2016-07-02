
#include <type_traits>
#include <iostream>

template <typename Type>
struct is_string : std::integral_constant<bool, false> {};

template <>
struct is_string<std::string> : std::integral_constant<bool, true> {};

template <bool isIntegral, bool isString>
struct printer_impl
{
    template <typename U>
    void print(U value) {
        (void)value;	
        std::cout << "Error" << std::endl;
    }
};

template <>
struct printer_impl<false, true>
{
    template <typename U>
    void print(U value) {
        std::cout << value.c_str() << std::endl;
    }
};

template <>
struct printer_impl<true, false>
{
    template <typename U>
    void print(U value) {
        std::cout << value << std::endl;
    }
};

template <typename Type,
          bool isIntegral = std::is_integral<Type>::value,
          bool isString = is_string<Type>::value>
void print(Type _value)
{
    printer_impl<isIntegral, isString> printer;
    printer.print(_value);
}

class foo {};

int main()
{
	print(1);
	std::string str = "112";
	print(str);
    foo foo_;
    print(foo_);
    return 0;
}
