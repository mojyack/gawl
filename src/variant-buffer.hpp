#pragma once
#define CUTIL_NS gawl
#include "util/event.hpp"
#include "util/variant.hpp"
#include "util/writers-reader-buffer.hpp"
#undef CUTIL_NS

namespace gawl::impl {
template <class... Ts>
class VariantBuffer : public WritersReaderBuffer<Variant<Ts...>> {
  private:
    using Super = WritersReaderBuffer<Variant<Ts...>>;

  public:
    using Item = Variant<Ts...>;

    template <class T>
    static constexpr auto index_of = Item::template index_of<T>;

    template <class T, class... Args>
    auto push(Args&&... args) -> void {
        Super::push(Item(Tag<T>(), std::forward<Args>(args)...));
    }
};

template <class... Ts>
class VariantEventBuffer : public VariantBuffer<Ts...> {
  private:
    using Super = VariantBuffer<Ts...>;

    Event reader_ready;

  public:
    template <class T, class... Args>
    auto push(Args&&... args) -> void {
        Super::template push<T>(std::forward<Args>(args)...);
        reader_ready.wakeup();
    }

    auto wait() -> void {
        reader_ready.wait();
        reader_ready.clear();
    }
};
} // namespace gawl::impl
