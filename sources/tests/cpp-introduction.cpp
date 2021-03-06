#include <boost/test/unit_test.hpp>

#include <iostream>
#include <map>

#include "utils/timer.hpp"
#include "a.hpp"

#define max(a, b) (((a) > (b)) ? (a) : (b))

template <class T>
T min(T a, T b)
{
    return (a < b) ? a : b;
}

void swap(int& x, int& y)
{
    int buf = x;
    x = y;
    y = buf;
}

void sort(int a[], int len)
{
    for (int k=0; k<len; ++k)
        for (int l=0; l<len-1; ++l)
            if (a[l] < a[l+1])
                swap(a[l], a[l+1]);
}

int f()
{
    static auto value = 10;
    return value--;
}

BOOST_AUTO_TEST_SUITE( cpp_introduction )

BOOST_AUTO_TEST_CASE( sort_test )
{
    const int len = 5;
    int a[5] = { 7, 4, 1, 2, max(3, 8) };
                // (((3) > (8)) ? (3) : (8))
    sort(a, len);
            
    for (int k=0; k<len-1; ++k)
        BOOST_CHECK_MESSAGE( a[k] > a[k+1], "k: " << k << "; a[k]: " << a[k] << "; a[k+1]: " << a[k+1] );
}

#undef max

BOOST_AUTO_TEST_CASE( int_pointer_test )
{
    int* ptr = NULL;
    int x = 5;
    ptr = &x;
    *ptr = 10;
    BOOST_CHECK_EQUAL( x, 10 );
}

struct ListItem
{
    ListItem* next;
    int content;
};

void add(ListItem*& anker, int value)
{
    ListItem* newitem = new ListItem;
    newitem->next = anker;
    newitem->content = value;
    anker = newitem;
}

int returnLastItem(ListItem* anker)
{
while(anker->next != NULL)
{
anker = anker->next;
}
return (anker->content);
}


void ausgabe(ListItem* anker)
{
while(anker->next != NULL)
{
std::cout << anker->content;
anker=anker->next;
}
}

BOOST_AUTO_TEST_CASE( add_pointer_test )
{
    ListItem* anker = NULL;

    add(anker, 10);
    add(anker, 11);

    /*
    ListItem* iterator = anker;
    while (iterator != NULL) {
        std::cout << "value: " << iterator->content << ".\n";
        iterator = iterator->next;
    }
    */
	BOOST_CHECK_EQUAL( returnLastItem(anker), 10 );
}

BOOST_AUTO_TEST_CASE( pointer_test )
{
    ListItem item;
    item.content = 10;

    ListItem* ptr = &item;
    (*ptr).content = 11;
    ptr->content = 11;

    ListItem* p = new ListItem;
    delete p;
}

BOOST_AUTO_TEST_CASE( string_test )
{
    std::string txt = "Hello ";
    txt += "World!";
    BOOST_CHECK_EQUAL( txt, "Hello World!" );
    BOOST_CHECK_EQUAL( txt[4], 'o' );
    txt[3] = 'L';
    BOOST_CHECK_EQUAL( txt, "HelLo World!" );
    BOOST_CHECK_EQUAL( txt.size(), 12U );
    //if (txt.c_str() == "HelLo World!")
    //    BOOST_CHECK( false );
}

class Base
{
    public:
        int x;
        int y;

        Base(int a = 15, int b = 5)
            : x(a), y(b)
        {
            //std::cout << "Hello World!\n";
        }

        Base(const Base& other)
            : x(other.x), y(other.y)
        { }
        
        ~Base()
        {
            //std::cout << "Goodbye!\n";
        }

        virtual int getSum() const;

    protected:
        int z;

    private:
        int f() const
        {
            return 0;
        }
};

//struct Name { int x };

int Base::getSum() const
{
    return x + y + f();
}

class Derived : public Base
{
    public:
        int w;

        Derived(int a)
            : Base(11, 11), w(a)
        { }

        int getSum() const override
        {
            return Base::getSum() + w + x;
        }
};

BOOST_AUTO_TEST_CASE( derived_test )
{
    Derived d(8);
    BOOST_CHECK_EQUAL( d.getSum(), 41 );

    Base* b = &d;
    BOOST_CHECK_EQUAL( b->getSum(), 41 );
}

template <class T = int>
class Pair
{
    public:
        T x;
        T y;

        Pair(T i, T j)
            : x(i), y(j)
        { }
};

BOOST_AUTO_TEST_CASE( template_test )
{
    Pair<> p(5, 7);
    BOOST_CHECK_EQUAL( p.x, 5 );
}

BOOST_AUTO_TEST_CASE( stl_lib )
{
    std::vector<int> arr = { 10, 11, 12, 16 };
    BOOST_CHECK_EQUAL( arr[2], 12 );
    BOOST_CHECK_EQUAL( arr.size(), 4U );
    BOOST_CHECK_EQUAL( arr.front(), 10 );
    BOOST_CHECK_EQUAL( arr.back(), 16 );
    arr.push_back(18);
    BOOST_CHECK_EQUAL( arr.size(), 5U );
    BOOST_CHECK_EQUAL( arr.back(), 18 );
}

BOOST_AUTO_TEST_CASE( function_pointers )
{
    std::vector<int (*) (int,int)> arr = { &min<int> };
    BOOST_CHECK_EQUAL( min(4, 6), 4 );
    BOOST_CHECK_EQUAL( min(4.0, 6.0), 4.0 );
    BOOST_CHECK_EQUAL( arr.front()(5, 6), 5 );
}

BOOST_AUTO_TEST_CASE( map_test )
{
    std::map<int, bool> m;
    m[1000] = true;
    m[100] = false;
    m[10] = true;
    m[1] = false;

    BOOST_CHECK( m[1000] );
    BOOST_CHECK( m[10] );

    BOOST_CHECK(m.find(50) == m.end());
    auto it = m.find(10);
    BOOST_REQUIRE(it != m.end());
    BOOST_CHECK_EQUAL(it->first, 10);
    BOOST_CHECK_EQUAL(it->second, true);
}

/*
BOOST_AUTO_TEST_CASE( class_test )
{
    std::cout << "begin...\n";
    Base base;
    Base other(base);
    BOOST_CHECK_EQUAL( base.x, 15 );
    BOOST_CHECK_EQUAL( base.getSum(), 10 );
    BOOST_CHECK_EQUAL( other.getSum(), 10 );
    //BOOST_CHECK_EQUAL( base.f(), 0 );
    //BOOST_CHECK_EQUAL( base.z, 0 );
    std::cout << "end.\n";
}
*/

/*
BOOST_AUTO_TEST_CASE( test_name )
{
    // type variablenname = initvalue;
    int x = 1;
    bool b = (true || false) && !false;
    double d = ((b) ? 4.0 : 1.0);

    d += 5.0;
    d = d + 5.0;
    b = (d == 0.0) && (d != 1.0);
    x %= 5;
    x = x % 5;

    // b = (a == d); => FEHLER

    if (x == 5) {
        d = 7.0;
    } else if (x == 6) {
        d = 4.0;
    } else {
        d = 6.0;
    }

    switch (x)
    {
        case 5:
        {
            d = 7.0;
            int w = 5;
            break;
        }
        case 6:
            d = 4.0;
            break;
        default:
            d = 6.0;
            break;
    }

    x = 21;
    while (x > 0) {
        int y = --x;
        if (y == 42)
            break;
        int z = x--;
    }

    do {
        x -= 1;
    } while (x > 0);

    const int len = 10;
    //len = 11; => FEHLER
    int a[len] = {0, 1, 3, 4, 5, 6, 7, 8, 9, 10};
    for (int k=0; k<10; ++k) {
        a[k] = k;
    }
    a[11] = 0;
}
*/

BOOST_AUTO_TEST_SUITE_END()









