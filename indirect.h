#include <memory>
#include <type_traits>

struct in_place_t
{
};

constexpr in_place_t in_place;

class bad_indirect_cast : public std::bad_cast
{
};

namespace detail
{

    class control_block
    {
    public:
        virtual ~control_block() = default;
        virtual std::unique_ptr<control_block> copy() const = 0;
        virtual std::unique_ptr<control_block> move() = 0;
        virtual void* ptr() = 0;
    };

    template <typename T>
    class direct_control_block final : public control_block
    {
    private:
        T t;

    public:
        template <typename... Ts>
        explicit direct_control_block(Ts&&... ts) :
            t(std::forward<Ts>(ts)...)
        {
        }

        std::unique_ptr<control_block> copy() const override
        {
            return std::make_unique<direct_control_block>(t);
        }

        std::unique_ptr<control_block> move() override
        {
            return std::make_unique<direct_control_block>(std::move(t));
        }

        void* ptr() override
        {
            return &t;
        }
    };

    template <typename T>
    using dcb_t = direct_control_block<std::remove_cv_t<std::remove_reference_t<T>>>;

} // namespace detail

template <typename T>
class indirect
{
    template <typename>
    friend class indirect;

private:
    struct data
    {
        T* ptr;
        std::unique_ptr<detail::control_block> cb;

        data(std::unique_ptr<detail::control_block>&& cb_) :
            ptr(static_cast<T*>(cb_->ptr())),
            cb(std::move(cb_))
        {
        }

        data& operator=(std::unique_ptr<detail::control_block>&& cb_)
        {
            ptr = static_cast<T*>(cb_->ptr());
            cb = std::move(cb_);
            return *this;
        }
    };

    data m;

public:
    template <typename T_ = T, std::enable_if_t<std::is_default_constructible<T_>::value, int> = 0>
    indirect() :
        m(std::make_unique<detail::dcb_t<T>>())
    {
    }

    template <typename... Ts>
    explicit indirect(in_place_t, Ts&&... ts) :
        m(std::make_unique<detail::dcb_t<T>>(std::forward<Ts>(ts)...))
    {
    }

    indirect(indirect const& other) :
        m(other.m.cb->copy())
    {
    }

    indirect(indirect&& other) :
        m(other.m.cb->move())
    {
    }

    indirect& operator=(indirect const& other)
    {
        m = other.m.cb->copy();
        return *this;
    }

    indirect& operator=(indirect&& other)
    {
        m = other.m.cb->move();
        return *this;
    }

    template <typename U, std::enable_if_t<std::is_base_of<T, U>::value, int> = 0>
    indirect(indirect<U> const& other) :
        m(other.m.cb->copy())
    {
    }

    template <typename U, std::enable_if_t<std::is_base_of<T, U>::value, int> = 0>
    indirect(indirect<U>&& other) :
        m(other.m.cb->move())
    {
    }

    template <typename U, std::enable_if_t<std::is_base_of<T, U>::value, int> = 0>
    indirect& operator=(indirect<U> const& other)
    {
        m = other.m.cb->copy();
        return *this;
    }

    template <typename U, std::enable_if_t<std::is_base_of<T, U>::value, int> = 0>
    indirect& operator=(indirect<U>&& other)
    {
        m = other.m.cb->move();
        return *this;
    }

    template <typename U, std::enable_if_t<std::is_base_of<T, U>::value, int> = 0>
    indirect(U&& other) :
        m(std::make_unique<detail::dcb_t<U>>(std::forward<U>(other)))
    {
    }

    template <typename U, std::enable_if_t<std::is_base_of<T, U>::value, int> = 0>
    indirect& operator=(U&& other)
    {
        m = std::make_unique<detail::dcb_t<U>>(std::forward<U>(other));
        return *this;
    }

    void swap(indirect& other) noexcept
    {
        using std::swap;
        swap(m.ptr, other.m.ptr);
        swap(m.cb, other.m.cb);
    }

    T const* operator->() const noexcept
    {
        return m.ptr;
    }

    T* operator->() noexcept
    {
        return m.ptr;
    }

    T const& operator*() const noexcept
    {
        return *m.ptr;
    }

    T& operator*() noexcept
    {
        return *m.ptr;
    }

private:
    template <typename T, typename U>
    friend indirect<T> static_indirect_cast(indirect<U> const& i);
    template <typename T, typename U>
    friend indirect<T> static_indirect_cast(indirect<U>&& i);
    template <typename T, typename U>
    friend indirect<T> dynamic_indirect_cast(indirect<U> const& i);
    template <typename T, typename U>
    friend indirect<T> dynamic_indirect_cast(indirect<U>&& i);

    explicit indirect(std::unique_ptr<detail::control_block>&& cb) noexcept :
        m(std::move(cb))
    {
    }

    template <typename U>
    void try_static_cast() const
    {
        static_cast<U*>(m.ptr);
    }

    template <typename U>
    void try_dynamic_cast() const
    {
        if (!dynamic_cast<U*>(m.ptr))
        {
            throw bad_indirect_cast();
        }
    }
};

template <typename T, typename... Ts>
indirect<T> make_indirect(Ts&&... ts)
{
    return indirect<T>(in_place, std::forward<Ts>(ts)...);
}

template <typename T, typename U>
indirect<T> static_indirect_cast(indirect<U> const& i)
{
    i.try_static_cast<T>();
    return indirect<T>(i.m.cb->copy());
}

template <typename T, typename U>
indirect<T> static_indirect_cast(indirect<U>&& i)
{
    i.try_static_cast<T>();
    return indirect<T>(i.m.cb->move());
}

template <typename T, typename U>
indirect<T> dynamic_indirect_cast(indirect<U> const& i)
{
    i.try_dynamic_cast<T>();
    return static_indirect_cast<T>(i);
}

template <typename T, typename U>
indirect<T> dynamic_indirect_cast(indirect<U>&& i)
{
    i.try_dynamic_cast<T>();
    return static_indirect_cast<T>(std::move(i));
}

template <typename T>
void swap(indirect<T>& i1, indirect<T>& i2) noexcept
{
    i1.swap(i2);
}
