#pragma once

#include <vector>

#if defined(TEST)
#  include <fmt/format.h>
#  define MY_ASSERT(cond, msg) if (!(cond)) throw std::runtime_error((msg));
#else
#  define MY_ASSERT(cond, msg) assert(cond);
#endif

namespace wcc {

// ChunkStorage is not intended for outside
template <typename Chunk>
class ChunkStorage
{
public:
    using size_type  = Chunk::size_type;

    ChunkStorage(uint32_t N) : new_curr_(0), return_curr_(0), chunks_(N) {
        for (auto it = chunks_.begin(); it != chunks_.end(); ++it) {
            it->reserve(Chunk::chunk_size);
        }
    }

    ~ChunkStorage() {}

    size_t num_chunks_allocated() const { return chunks_.size(); }

    Chunk& new_chunk() {
        MY_ASSERT(new_curr_ <= chunks_.size(), (fmt::format("new_curr:{} <= N{}", new_curr_, chunks_.size())))
        if (new_curr_ == chunks_.size()) new_curr_ = 0;

        // Note the fact that every unused chunk must be space reserved!
        // When a chunk at position new_curr_ is used, its capacity must be zero
        // since it simply is pure empty (not even reserve any space)
        // (space swapped by the user using it).
        if (chunks_[new_curr_].capacity() == 0) [[unlikely]] { // being used
            //throw std::runtime_error("No free chunks left in storage");
            // When no chunks, acquire by new alloc.
            // TODO: recheck logic if using return_chunk!
            chunks_.emplace_back();
            new_curr_ = chunks_.size() - 1;
            chunks_[new_curr_].reserve(Chunk::chunk_size);
        }
        return chunks_[new_curr_++]; // for moving
    }

    void return_chunk(Chunk& chunk) {
        if (return_curr_ == chunks_.size()) return_curr_ = 0;
        MY_ASSERT(chunks_[return_curr_].capacity() == 0,
          (fmt::format("chunks_[return_curr_({})].capaticy()=={}, expected 0", return_curr_, chunks_[return_curr_].capacity())));  // current place is no used by any chunk
        chunk.clear();  // leaves the capacity() of the chunk unchanged
        chunks_[return_curr_++] = std::move(chunk);
    }

private:
    size_type new_curr_;
    size_type return_curr_;
    std::vector<Chunk> chunks_;
};


/**
 * AppendOnlyVec: only appending, never delete, chunked vector.
 *
 * Memory block never be reallocated or moved, only added chunk by chunk,
 * where chunk is size-fixed (and specified) at compiler time.
 */

template <class Vec, bool IsConst>
class VecIterator {
    using vec_ref_type = std::conditional_t<IsConst, Vec const&, Vec&>;
public:
    using iterator_category = std::random_access_iterator_tag;
    using value_type        = Vec::value_type;
    using difference_type   = Vec::difference_type;
    using size_type         = Vec::size_type;
    using reference         = std::conditional_t<IsConst, typename Vec::const_reference, typename Vec::reference>;
    using pointer           = std::conditional_t<IsConst, typename Vec::const_pointer, typename Vec::pointer>;

    // Constructors
    VecIterator(vec_ref_type vec, size_type i)  : vec_(vec), idx_(i) {}   // not intended for user to use
    VecIterator(const VecIterator& other)    : vec_(other.vec_), idx_(other.idx_) {}

   // Assignments
    VecIterator& operator=(const VecIterator& other) { vec_ = other.vec_; idx_ = other.idx_; return *this; }

    // pointer like operators
    pointer  operator->() const { return &vec_[idx_]; }
    reference operator*() const { return vec_[idx_]; }
    reference operator[](difference_type off) const { return vec_[idx_ + off]; }

    // Increment / Decrement
    VecIterator& operator++()    { ++idx_; return *this; }
    VecIterator  operator++(int) { return VecIterator(idx_++); }
    VecIterator& operator--()    { --idx_; return *this; }
    VecIterator  operator--(int) { return VecIterator(idx_--); }

    // Arithmetic
    VecIterator& operator+=(difference_type off) { idx_ += off; return *this; }
    VecIterator& operator-=(difference_type off) { idx_ -= off; return *this; }

    friend VecIterator operator+(VecIterator left, difference_type off) { left.idx_ += off; return left; }
    friend VecIterator operator+(difference_type off, VecIterator right) { right.idx_ += off; return right; }
    friend VecIterator operator-(VecIterator left, difference_type off) { left.idx_ -= off; return left; }

    // Difference
    friend difference_type operator-(const VecIterator& left, const VecIterator& right) { return left.idx_ - right.idx_; }

    // Comparison operators
    friend bool operator==(const VecIterator& l, const VecIterator& r) { return l.idx_ == r.idx_; }
    friend auto operator<=>(const VecIterator& l, const VecIterator& r) { return l.idx <=> r.idx_; }

private:
    size_type idx_;
    vec_ref_type vec_;
};

// ChunkSize is capacity of each chunk.
// chunks_.size() is number of chunks.
template <typename T, size_t ChunkSize>
class AppendOnlyVec {
public:
    struct Chunk : std::vector<T> {
        constexpr static uint32_t chunk_size = ChunkSize;

        Chunk() = default;  // { this->reserve(ChunkSize); }
        // Move ctor and assignment will be default and use that of std::vector;
        // anyway we make our intention explicit!
        Chunk(Chunk&&) = default;
        Chunk(Chunk const&) = delete;
        Chunk& operator=(Chunk&&) = default;
        Chunk& operator=(Chunk const&) = delete;
    };

    using value_type             = Chunk::value_type;
    using size_type              = Chunk::size_type;
    using difference_type        = Chunk::difference_type;
    using reference              = Chunk::reference;
    using const_reference        = Chunk::const_reference;
    using pointer                = Chunk::pointer;
    using const_pointer          = Chunk::const_pointer;
    using iterator               = VecIterator<AppendOnlyVec, false>;
    using const_iterator         = VecIterator<AppendOnlyVec, true>;
    using reverse_iterator       = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;
    using allocator_type         = typename Chunk::allocator_type;

    AppendOnlyVec() {
        static_assert((ChunkSize & (ChunkSize-1)) == 0, "ChunkSize must be power of 2");
        if (!IsConfigured) [[unlikely]] {
            throw std::logic_error("Construct AppendOnlyVec before calling AppendOnlyVec::config()");
        }
    }

    static void config(size_t n_chunks) {
        if (!IsConfigured) {
            NumChunks = n_chunks;
            StoragePtr = std::make_unique<ChunkStorage<Chunk>>(NumChunks);
            IsConfigured = true;
        } else {
            throw std::logic_error("Calling AppendOnlyVec::config multiple times!");
        }
    }

    static size_t num_chunks_allocated() {
        if (IsConfigured) [[likely]] {
            return StoragePtr->num_chunks_allocated();
        }
        throw std::logic_error("Must call AppendOnlyVec::config first!");
    }

    AppendOnlyVec(AppendOnlyVec const&) = delete;  // copy ctor not allowed

    AppendOnlyVec(AppendOnlyVec&& o) {
        this->chunks_.swap(o.chunks_);
        last_chunk_idx_ = o.last_chunk_idx_;
        o.last_chunk_idx_ = 0; // b/c this is ctor
    }

    ~AppendOnlyVec() {
        for (auto& c : chunks_) {
            c.clear();
            StoragePtr->return_chunk(c);
        }
    }

    AppendOnlyVec& operator = (AppendOnlyVec const&) = delete;

    AppendOnlyVec& operator = (AppendOnlyVec&& o) {
        clear();
        this->chunks_.swap(o.chunks_);
        last_chunk_idx_ = o.last_chunk_idx_;
        o.last_chunk_idx_ = 0; // b/c clear()
        return *this;
    }

    size_type size() const {
        if (chunks_.size() == 0) return 0;
        return last_chunk_idx_ * ChunkSize + chunks_[last_chunk_idx_].size();
    }

    bool empty() const { return !size(); }

    size_type capacity() const { return ChunkSize * chunks_.capacity(); }

    void reserve(size_type num_chunks) {
        chunks_.reserve(num_chunks);
    }

    void clear() {
        for (auto& c : chunks_) c.clear();
        last_chunk_idx_ = 0;
    }

    T& operator[](size_type i) {
        assert(i < size());
        auto chunk_i = i >> shift_n;  // divided by ChunkSize
        return chunks_[chunk_i][i & (ChunkSize-1)]; // modularized by ChunkSize
    }

    T const& operator[](size_type i) const {
        return (*const_cast<AppendOnlyVec*>(this))[i];
    }

    T&       front()       { return (*this)[0]; }
    T const& front() const { return (*this)[0]; }

    T&       back()        { return (*this)[size()-1]; }
    T const& back()  const { return (*this)[size()-1]; }

    iterator begin()  { return make_iter(0); }
    iterator end()    { return make_iter(size()); }

    const_iterator begin() const { return make_iter(0); }
    const_iterator end()   const { return make_iter(size()); }

    const_iterator cbegin() const { return make_iter(0); }
    const_iterator cend()   const { return make_iter(size()); }

    iterator rbegin()  { return make_iter(size()-1); }
    iterator rend()    { return make_iter(0-1); }  // Note -1 == unsigned(0)-1

    const_iterator rbegin() const { return make_iter(size()-1); }
    const_iterator rend()   const { return make_iter(0-1); }

    const_iterator crbegin() const { return make_iter(size()-1); }
    const_iterator crend()   const { return make_iter(0-1); }

    void push_back(T const& t) {
        if (chunks_.size() == 0) new_chunk();
        if (chunks_[last_chunk_idx_].size() == ChunkSize) ++last_chunk_idx_;
        if (chunks_.size() == last_chunk_idx_) new_chunk();
        chunks_[last_chunk_idx_].push_back(t);
    }

    template <typename... Args>
    reference emplace_back(Args&&... args) {
        if (chunks_.size() == 0) new_chunk();                                 // there nothing, add a new chunk
        if (chunks_[last_chunk_idx_].size() == ChunkSize) ++last_chunk_idx_; // current chunk full, move to next
        if (chunks_.size() == last_chunk_idx_) new_chunk();                   // there's no next chunk, add a new one
        chunks_[last_chunk_idx_].emplace_back(std::forward<Args>(args)...);
        return chunks_[last_chunk_idx_].back();
    }

private:
    void new_chunk() {
        chunks_.emplace_back(std::move(StoragePtr->new_chunk()));
    }

    auto make_iter(size_type i) const { return const_iterator(*this, i); }
    auto make_iter(size_type i)       { return iterator(*this, i); }

    constexpr static size_type shift_n = []()constexpr {
        size_type i = 0;
        for (auto v = ChunkSize; v; ++i, v >>= 1);
        return i-1;
    }();

    size_type last_chunk_idx_ = 0; // depends on if chunks_.size() <> 0
    std::vector<Chunk> chunks_;

    inline static std::unique_ptr<ChunkStorage<Chunk>> StoragePtr;
    inline static bool IsConfigured = false;
protected:
    inline static uint32_t NumChunks = 1024;
};

}  // namespace wcc
