#ifndef LOWLEVEL_H
#define LOWLEVEL_H

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;

// Опасное приведение типов - просто один тип назывется другим
// Единственная проверка - совпадение размеров
template <typename OutputClass, typename InputClass>
union horrible_union{
    OutputClass out;
    InputClass in;
};

template <typename OutputClass, typename InputClass>
inline OutputClass horrible_cast(const InputClass input){
    horrible_union<OutputClass, InputClass> u;
    // Cause a compile-time error if in, out and u are not the same size.
    // If the compile fails here, it means the compiler has peculiar
    // unions which would prevent the cast from working.
    typedef int ERROR_CantUseHorrible_cast[sizeof(InputClass)==sizeof(u)
        && sizeof(InputClass)==sizeof(OutputClass) ? 1 : -1];
    u.in = input;
    return u.out;
}

// Предствление переменной, как имеющей другой тип
#define _cast(type, var) ( *(type*)(&var) )


// Представляет любую переменную как массив байт
// нумерация начинается с наименее значащего байта
template<class Base>
class Complex
{
public:
    Complex (const Base& a =0)
        : base (a) {}
    Complex (const volatile Base& a)
        : base (a) {}

    template<typename... Args>
    Complex (const uint8_t& byte0, const Args&... bytes)
        { init (0, byte0, bytes...); }
    template<typename... Args>
    Complex (const volatile uint8_t& byte0, const volatile Args&... bytes)
        { init (0, byte0, bytes...); }

    operator Base& ()
        { return base; }
    operator volatile Base& () volatile
        { return base; }

    // Доступ к N-тому байту. 0 байт - наименее значащий
    uint8_t& operator[] (uint8_t byteNumber)
        { return *((uint8_t*)this + byteNumber); }

    Base base;

private:
    void init (uint8_t n) {}

    template< typename... Args >
    void init (uint8_t n, uint8_t byte, Args... bytes)
    {
        operator[] (n) = byte;
        init (n+1, bytes...);
    }
};

Complex<uint16_t> swap (const uint16_t& a)
{
    return {Complex<uint16_t>(a)[1], Complex<uint16_t>(a)[0]};
}

// Подбирает беззнаковый тип нужного размера (в байтах)
template <uint8_t size> struct TypeSelect;
    template <> struct TypeSelect<1> { typedef uint8_t Result; };
    template <> struct TypeSelect<2> { typedef uint16_t Result; };
    template <> struct TypeSelect<4> { typedef uint32_t Result; };

// Для имеющиейся структуры с битовыми полями Bit
// предосталяет доступ как к целому
// также сохраняя доступ к элементам через оператор ->
template< typename Bit >
class Bitfield
{
private:
    typedef typename TypeSelect< sizeof(Bit) >::Result Base;
public:
    explicit Bitfield (const Base& a = 0)
        : base (a) {}
    explicit Bitfield (const volatile Base& a)
        : base (a) {}
    Bitfield (const Bit& a)
    {
        operator= (a);
    }
    void operator = (const Base& a) volatile
    {
        base = a;
    }
    void operator = (const Bitfield& a) volatile
    {
        base = a.base;
    }
    void operator = (const Bit& a) volatile
    {
        base = horrible_cast<Base>(a);
    }
    operator volatile Base () volatile const
    {
        return base;
    }

    Bit* operator-> ()
    {
        return &bit;
    }
    const Bit* operator-> () const
    {
        return &bit;
    }


    union
    {
        Base base;
        Bit bit;
    };
};

// Если размер Bit не соответсвует никакому базовому типу,
// то предлагается использовать BitfieldDummy, который не выполняет никаких функций, но
// предоставляет доступ к элементам Bit через -> для единообразия
template< typename Bit >
class BitfieldDummy
{
public:
    Bit* operator-> ()
    {
        return &bit;
    }
    Bit bit;
};

#endif // LOWLEVEL_H
