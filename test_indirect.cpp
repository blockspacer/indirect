#include <catch.hpp>
#include <indirect.h>

class base
{
public:
    virtual ~base() = default;
    virtual int get_value() const = 0;
    virtual void set_value(int) = 0;
};

class derived : public base
{
private:
    int value;

public:
    static size_t object_count;

public:
    derived() : value() { ++object_count; }
    derived(derived const& other) : value(other.value) { ++object_count; }
    derived(derived&& other) : value(other.value) { other.value = 0; ++object_count; }
    derived(int v) : value(v) { ++object_count; }
    ~derived() { --object_count; }
    int get_value() const override { return value; }
    void set_value(int i) override { value = i; }
};

size_t derived::object_count = 0u;

class derived_other : public base
{
private:
    int value = 0;

public:
    int get_value() const override { return value; }
    void set_value(int i) override { value = i; }
};

SCENARIO("`indirect` can be default constructed", "[construct][default]")
{
    GIVEN("a default constructed `indirect<derived>`")
    {
        indirect<derived> d;

        REQUIRE(derived::object_count == 1);
        REQUIRE(d->get_value() == 0);
    }
}

SCENARIO("`indirect` can be constructed in-place", "[construct][in_place]")
{
    GIVEN("an `indirect<derived>` constructed in-place")
    {
        indirect<derived> d{in_place, 42};

        REQUIRE(derived::object_count == 1);
        REQUIRE(d->get_value() == 42);
    }
}

SCENARIO("`indirect` can be copy constructed", "[construct][copy]")
{
    GIVEN("an `indirect<derived>`")
    {
        indirect<derived> d1{in_place, 42};

        WHEN("it is copied into another `indirect<derived>` on construction")
        {
            indirect<derived> d2{d1};

            REQUIRE(derived::object_count == 2);
            REQUIRE(d1->get_value() == 42);
            REQUIRE(d2->get_value() == 42);
        }

        WHEN("it is copied into an `indirect<base>` on construction")
        {
            indirect<base> b2{d1};

            REQUIRE(derived::object_count == 2);
            REQUIRE(d1->get_value() == 42);
            REQUIRE(b2->get_value() == 42);
        }
    }

    GIVEN("an `indirect<base>`")
    {
        indirect<base> b1{indirect<derived>(in_place, 42)};

        WHEN("it is copied into another `indirect<base>` on construction")
        {
            indirect<base> b2{b1};

            REQUIRE(derived::object_count == 2);
            REQUIRE(b1->get_value() == 42);
            REQUIRE(b2->get_value() == 42);
        }
    }
}

SCENARIO("`indirect` can be copy assigned", "[assign][copy]")
{
    GIVEN("an `indirect<derived>`")
    {
        indirect<derived> d1{in_place, 42};

        WHEN("it is copied into an existing `indirect<derived>`")
        {
            indirect<derived> d2;
            d2 = d1;

            REQUIRE(derived::object_count == 2);
            REQUIRE(d1->get_value() == 42);
            REQUIRE(d2->get_value() == 42);
        }

        WHEN("it is copied into an existing `indirect<base>`")
        {
            indirect<base> b2{indirect<derived>{}};
            b2 = d1;

            REQUIRE(derived::object_count == 2);
            REQUIRE(d1->get_value() == 42);
            REQUIRE(b2->get_value() == 42);
        }
    }

    GIVEN("an `indirect<base>`")
    {
        indirect<base> b1{indirect<derived>(in_place, 42)};

        WHEN("it is copied into an existing `indirect<base>`")
        {
            indirect<base> b2{indirect<derived>{}};
            b2 = b1;

            REQUIRE(derived::object_count == 2);
            REQUIRE(b1->get_value() == 42);
            REQUIRE(b2->get_value() == 42);
        }
    }
}

SCENARIO("`indirect` can be move constructed", "[construct][move]")
{
    GIVEN("an `indirect<derived>`")
    {
        indirect<derived> d1{in_place, 42};

        WHEN("it is moved into another `indirect<derived>` on construction")
        {
            indirect<derived> d2{std::move(d1)};

            REQUIRE(derived::object_count == 2);
            REQUIRE(d1->get_value() == 0);
            REQUIRE(d2->get_value() == 42);
        }

        WHEN("it is moved into an `indirect<base>` on construction")
        {
            indirect<base> b2{std::move(d1)};

            REQUIRE(derived::object_count == 2);
            REQUIRE(d1->get_value() == 0);
            REQUIRE(b2->get_value() == 42);
        }
    }

    GIVEN("an `indirect<base>`")
    {
        indirect<base> b1{indirect<derived>(in_place, 42)};

        WHEN("it is moved into another `indirect<base>` on construction")
        {
            indirect<base> b2{std::move(b1)};

            REQUIRE(derived::object_count == 2);
            REQUIRE(b1->get_value() == 0);
            REQUIRE(b2->get_value() == 42);
        }
    }
}

SCENARIO("`indirect` can be move assigned", "[assign][move]")
{
    GIVEN("an `indirect<derived>`")
    {
        indirect<derived> d1{in_place, 42};

        WHEN("it is moved into another existing `indirect<derived>`")
        {
            indirect<derived> d2;
            d2 = std::move(d1);

            REQUIRE(derived::object_count == 2);
            REQUIRE(d1->get_value() == 0);
            REQUIRE(d2->get_value() == 42);
        }

        WHEN("it is moved into an existing `indirect<base>`")
        {
            indirect<base> b2{indirect<derived>{}};
            b2 = std::move(d1);

            REQUIRE(derived::object_count == 2);
            REQUIRE(d1->get_value() == 0);
            REQUIRE(b2->get_value() == 42);
        }
    }

    GIVEN("an `indirect<base>`")
    {
        indirect<base> b1{indirect<derived>(in_place, 42)};

        WHEN("it is moved into another `indirect<base>` on construction")
        {
            indirect<base> b2{indirect<derived>{}};
            b2 = std::move(b1);

            REQUIRE(derived::object_count == 2);
            REQUIRE(b1->get_value() == 0);
            REQUIRE(b2->get_value() == 42);
        }
    }
}

SCENARIO("`indirect` can be constructed from values of type `T`", "[construct][copy]")
{
    GIVEN("an `indirect<derived>` constructed from a `derived const&`")
    {
        derived d1{42};
        indirect<derived> d2 = d1;

        REQUIRE(derived::object_count == 2);
        REQUIRE(d1.get_value() == 42);
        REQUIRE(d2->get_value() == 42);

        WHEN("it is assigned from another `derived`")
        {
            d2 = derived{};

            REQUIRE(derived::object_count == 2);
            REQUIRE(d1.get_value() == 42);
            REQUIRE(d2->get_value() == 0);
        }
    }
}

    //GIVEN("an `indirect<base>` constructed from a `derived`")
    //{
        //indirect<base> b{derived{42}};

        //REQUIRE(derived::object_count == 1);
        //REQUIRE(d1->get_value() == 42);

        //WHEN("it is assigned from another `derived`")
        //{
            //d1 = derived{};

            //REQUIRE(derived::object_count == 1);
            //REQUIRE(b1->get_value() == 0);
        //}
    //}
//}

//SCENARIO("`indirect` can be assigned from values of type `T const&`", "[assign][copy]")
//{
    //GIVEN("an `indirect<derived>`")
    //{
        //indirect<derived> d1{in_place, 42};

        //WHEN("it is copied into an existing `indirect<derived>`")
        //{
            //indirect<derived> d2;
            //d2 = d1;

            //REQUIRE(derived::object_count == 2);
            //REQUIRE(d1->get_value() == 42);
            //REQUIRE(d2->get_value() == 42);
        //}
    //}

    //GIVEN("an `indirect<base>`")
    //{
        //indirect<base> b1{indirect<derived>(in_place, 42)};

        //WHEN("it is copied into an existing `indirect<base>`")
        //{
            //indirect<base> b2{indirect<derived>{}};
            //b2 = b1;

            //REQUIRE(derived::object_count == 2);
            //REQUIRE(b1->get_value() == 42);
            //REQUIRE(b2->get_value() == 42);
        //}
    //}
//}

//SCENARIO("`indirect` can be constructed from values of type `T&&`", "[construct][move]")
//{
    //GIVEN("an `indirect<derived>`")
    //{
        //indirect<derived> d1{in_place, 42};

        //WHEN("it is moved into another `indirect<derived>` on construction")
        //{
            //indirect<derived> d2{std::move(d1)};

            //REQUIRE(derived::object_count == 2);
            //REQUIRE(d1->get_value() == 0);
            //REQUIRE(d2->get_value() == 42);
        //}
    //}

    //GIVEN("an `indirect<base>`")
    //{
        //indirect<base> b1{indirect<derived>(in_place, 42)};

        //WHEN("it is moved into another `indirect<base>` on construction")
        //{
            //indirect<base> b2{std::move(b1)};

            //REQUIRE(derived::object_count == 2);
            //REQUIRE(b1->get_value() == 0);
            //REQUIRE(b2->get_value() == 42);
        //}
    //}
//}

//SCENARIO("`indirect` can be assigned from values of type `T&&`", "[assign][move]")
//{
    //GIVEN("an `indirect<derived>`")
    //{
        //indirect<derived> d1{in_place, 42};

        //WHEN("it is moved into another existing `indirect<derived>`")
        //{
            //indirect<derived> d2;
            //d2 = std::move(d1);

            //REQUIRE(derived::object_count == 2);
            //REQUIRE(d1->get_value() == 0);
            //REQUIRE(d2->get_value() == 42);
        //}
    //}

    //GIVEN("an `indirect<base>`")
    //{
        //indirect<base> b1{indirect<derived>(in_place, 42)};

        //WHEN("it is moved into another `indirect<base>` on construction")
        //{
            //indirect<base> b2{indirect<derived>{}};
            //b2 = std::move(b1);

            //REQUIRE(derived::object_count == 2);
            //REQUIRE(b1->get_value() == 0);
            //REQUIRE(b2->get_value() == 42);
        //}
    //}
//}

SCENARIO("`indirect` can be cast", "[cast]")
{
    GIVEN("an `indirect<base>` constructed from an `indirect<derived>`")
    {
        indirect<base> b1 = make_indirect<derived>(42);

        WHEN("it is static cast copied to `indirect<derived>`")
        {
            auto d2 = static_indirect_cast<derived>(b1);

            REQUIRE(derived::object_count == 2);
            REQUIRE(b1->get_value() == 42);
            REQUIRE(d2->get_value() == 42);
        }

        WHEN("it is static cast moved to `indirect<derived>`")
        {
            auto d2 = static_indirect_cast<derived>(std::move(b1));

            REQUIRE(derived::object_count == 2);
            REQUIRE(b1->get_value() == 0);
            REQUIRE(d2->get_value() == 42);
        }

        WHEN("it is dynamic cast copied to `indirect<derived>`")
        {
            auto d2 = dynamic_indirect_cast<derived>(b1);

            REQUIRE(derived::object_count == 2);
            REQUIRE(b1->get_value() == 42);
            REQUIRE(d2->get_value() == 42);
        }

        WHEN("it is dynamic cast copied to `indirect<derived_other>`")
        {
            REQUIRE_THROWS(auto d2 = dynamic_indirect_cast<derived_other>(b1));

            REQUIRE(derived::object_count == 1);
            REQUIRE(b1->get_value() == 42);
        }

        WHEN("it is dynamic cast moved back to `indirect<derived_other>`")
        {
            REQUIRE_THROWS(auto d2 = dynamic_indirect_cast<derived_other>(std::move(b1)));

            REQUIRE(derived::object_count == 1);
            REQUIRE(b1->get_value() == 42);
        }
    }

    GIVEN("an `indirect<derived const>` constructed from an `indirect<derived>`")
    {
        indirect<derived const> b1 = make_indirect<derived>(42);

        WHEN("it is const cast copied to `indirect<derived>`")
        {
            indirect<derived> d2 = b1;

            REQUIRE(derived::object_count == 2);
            REQUIRE(b1->get_value() == 42);
            REQUIRE(d2->get_value() == 42);
        }

        WHEN("it is const cast moved to `indirect<derived>`")
        {
            indirect<derived> d2 = std::move(b1);

            REQUIRE(derived::object_count == 2);
            REQUIRE(b1->get_value() == 0);
            REQUIRE(d2->get_value() == 42);
        }
    }
}
