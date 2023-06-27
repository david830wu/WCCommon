#pragma once

namespace wcc {

template <typename T> class delegate_base;

template <typename RET, typename... PARAMS>
class delegate_base<RET(PARAMS...)> {
  protected:
    using stub_type = RET (*)(void *this_ptr, PARAMS...);

    struct InvocationElement {
        void *object = nullptr;
        stub_type stub = nullptr;
    };
};

template <typename T> class delegate;
template <typename T> class multicast_delegate;

template <typename RET, typename... PARAMS>
class delegate<RET(PARAMS...)> final : private delegate_base<RET(PARAMS...)> {
public:
    delegate() = default;

    bool is_null() const { return invocation.stub == nullptr; }
    bool operator==(void *ptr) const {
        return (ptr == nullptr) && this->is_null();
    } // operator ==
    bool operator!=(void *ptr) const {
        return (ptr != nullptr) || (!this->is_null());
    } // operator !=

    template <typename LAMBDA>
    delegate(const LAMBDA &lambda) {
        assign((void *)(&lambda), lambda_stub<LAMBDA>);
    } // delegate

    template <typename LAMBDA> // template instantiation is not needed, will be deduced (inferred):
    delegate &operator=(const LAMBDA &instance) {
        assign((void *)(&instance), lambda_stub<LAMBDA>);
        return *this;
    } // operator =

    bool operator==(const delegate &another) const { return invocation == another.invocation; }
    bool operator!=(const delegate &another) const { return invocation != another.invocation; }

    bool operator==(const multicast_delegate<RET(PARAMS...)> &another) const { return another == (*this); }
    bool operator!=(const multicast_delegate<RET(PARAMS...)> &another) const { return another != (*this); }

    template <class T, RET (T::*TMethod)(PARAMS...)>
    static delegate create(T *instance) {
        return delegate(instance, method_stub<T, TMethod>);
    } // create

    template <class T, RET (T::*TMethod)(PARAMS...) const>
    static delegate create(T const *instance) {
        return delegate(const_cast<T *>(instance), const_method_stub<T, TMethod>);
    } // create

    template <RET (*TMethod)(PARAMS...)>
    static delegate create() {
        return delegate(nullptr, function_stub<TMethod>);
    } // create

    template <typename LAMBDA>
    static delegate create(const LAMBDA &instance) {
        return delegate((void *)(&instance), lambda_stub<LAMBDA>);
    } // create

    RET operator()(PARAMS... arg) const {
        return (*invocation.stub)(invocation.object, arg...);
    } // operator()

private:
    delegate(void *anObject, typename delegate_base<RET(PARAMS...)>::stub_type aStub) {
        invocation.object = anObject;
        invocation.stub = aStub;
    } // delegate

    void assign(void *anObject, typename delegate_base<RET(PARAMS...)>::stub_type aStub) {
        this->invocation.object = anObject;
        this->invocation.stub = aStub;
    } // assign

    template <class T, RET (T::*TMethod)(PARAMS...)>
    static RET method_stub(void *this_ptr, PARAMS... params) {
        T *p = static_cast<T *>(this_ptr);
        return (p->*TMethod)(params...);
    } // method_stub

    template <class T, RET (T::*TMethod)(PARAMS...) const>
    static RET const_method_stub(void *this_ptr, PARAMS... params) {
        T *const p = static_cast<T *>(this_ptr);
        return (p->*TMethod)(params...);
    } // const_method_stub

    template <RET (*TMethod)(PARAMS...)>
    static RET function_stub(void *this_ptr, PARAMS... params) {
        return (TMethod)(params...);
    } // function_stub

    template <typename LAMBDA>
    static RET lambda_stub(void *this_ptr, PARAMS... arg) {
        LAMBDA *p = static_cast<LAMBDA *>(this_ptr);
        return (p->operator())(arg...);
    } // lambda_stub

    friend class multicast_delegate<RET(PARAMS...)>;

    typename delegate_base<RET(PARAMS...)>::InvocationElement invocation;

}; // class delegate


template <typename RET, typename... PARAMS>
class multicast_delegate<RET(PARAMS...)> final : private delegate_base<RET(PARAMS...)> {
public:
    multicast_delegate() = default;
    ~multicast_delegate() {
        for (auto &element : invocationList)
            delete element;
        invocationList.clear();
    } //~multicast_delegate

    bool is_null() const { return invocationList.size() < 1; }
    bool operator==(void *ptr) const {
        return (ptr == nullptr) && this->is_null();
    } // operator ==
    bool operator!=(void *ptr) const {
        return (ptr != nullptr) || (!this->is_null());
    } // operator !=

    size_t size() const { return invocationList.size(); }

    multicast_delegate &operator=(const multicast_delegate &) = delete;
    multicast_delegate(const multicast_delegate &) = delete;

    bool operator==(const multicast_delegate &another) const {
        if (invocationList.size() != another.invocationList.size()) return false;
        auto anotherIt = another.invocationList.begin();
        for (auto it = invocationList.begin(); it != invocationList.end(); ++it)
            if (**it != **anotherIt) return false;
        return true;
    } //==
    bool operator!=(const multicast_delegate &another) const { return !(*this == another); }

    bool operator==(const delegate<RET(PARAMS...)> &another) const {
        if (is_null() && another.is_null()) return true;
        if (another.is_null() || (size() != 1)) return false;
        return (another.invocation == **invocationList.begin());
    } //==
    bool operator!=(const delegate<RET(PARAMS...)> &another) const { return !(*this == another); }

    multicast_delegate &operator+=(const multicast_delegate &another) {
        for (auto &item : another.invocationList) // clone, not copy; flattens hierarchy:
            this->invocationList.push_back(new typename delegate_base<RET(PARAMS...)>::InvocationElement(item->object, item->stub));
        return *this;
    } // operator +=

    template <typename LAMBDA> // template instantiation is not neededm, will be deduced/inferred:
    multicast_delegate &operator+=(const LAMBDA &lambda) {
        delegate<RET(PARAMS...)> d = delegate<RET(PARAMS...)>::template create<LAMBDA>(lambda);
        return *this += d;
    } // operator +=

    multicast_delegate &operator+=(const delegate<RET(PARAMS...)> &another) {
        if (another.is_null()) return *this;
        this->invocationList.push_back(new typename delegate_base<RET(PARAMS...)>::InvocationElement(another.invocation.object, another.invocation.stub));
        return *this;
    } // operator +=

    // will work even if RET is void, return values are ignored:
    // (for handling return values, see operator(..., handler))
    void operator()(PARAMS... arg) const {
        for (auto &item : invocationList)
            (*(item->stub))(item->object, arg...);
    } // operator()

    template <typename HANDLER>
    void operator()(PARAMS... arg, HANDLER handler) const {
        size_t index = 0;
        for (auto &item : invocationList) {
            RET value = (*(item->stub))(item->object, arg...);
            handler(index, &value);
            ++index;
        } // loop
    }     // operator()

    void operator()(PARAMS... arg, delegate<void(size_t, RET *)> handler) const {
        operator()<decltype(handler)>(arg..., handler);
    } // operator()
    void operator()(PARAMS... arg, std::function<void(size_t, RET *)> handler) const {
        operator()<decltype(handler)>(arg..., handler);
    } // operator()

private:
    std::list<typename delegate_base<RET(PARAMS...)>::InvocationElement *> invocationList;

}; // class multicast_delegate

} // namespace wcc