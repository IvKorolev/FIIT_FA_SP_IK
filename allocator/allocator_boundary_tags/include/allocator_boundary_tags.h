#ifndef MATH_PRACTICE_AND_OPERATING_SYSTEMS_ALLOCATOR_ALLOCATOR_BOUNDARY_TAGS_H
#define MATH_PRACTICE_AND_OPERATING_SYSTEMS_ALLOCATOR_ALLOCATOR_BOUNDARY_TAGS_H

#include <allocator_test_utils.h>
#include <allocator_with_fit_mode.h>
#include <pp_allocator.h>
#include <logger_guardant.h>
#include <logger.h>
#include <typename_holder.h>
#include <iterator>
#include <mutex>



class allocator_boundary_tags final :
    public smart_mem_resource,
    public allocator_test_utils,
    public allocator_with_fit_mode,
    private logger_guardant,
    private typename_holder
{

private:

    /**
     * TODO: You must improve it for alignment support
     */
    mutable std::mutex _allocator_mutex;
    static constexpr const size_t allocator_metadata_size = sizeof(logger*) + sizeof(memory_resource*) + sizeof(allocator_with_fit_mode::fit_mode) +
                                                            sizeof(size_t) + sizeof(void*);

    static constexpr const size_t occupied_block_metadata_size = sizeof(size_t) + sizeof(void*) + sizeof(void*) + sizeof(void*);

    static constexpr const size_t free_block_metadata_size = 0;
    logger *_logger;

    void *_trusted_memory;

public:
    
    ~allocator_boundary_tags() override;
    
    allocator_boundary_tags(allocator_boundary_tags const &other);
    
    allocator_boundary_tags &operator=(allocator_boundary_tags const &other);
    std::vector<allocator_test_utils::block_info> get_blocks_info_inner() const;

    allocator_boundary_tags(
        allocator_boundary_tags &&other) noexcept;
    
    allocator_boundary_tags &operator=(
        allocator_boundary_tags &&other) noexcept;

public:
    
    explicit allocator_boundary_tags(
            size_t space_size,
            std::pmr::memory_resource *parent_allocator = nullptr,
            logger *logger = nullptr,
            allocator_with_fit_mode::fit_mode allocate_fit_mode = allocator_with_fit_mode::fit_mode::first_fit);

public:
    
    [[nodiscard]] void *do_allocate_sm(
        size_t bytes) override;
    
    void do_deallocate_sm(
        void *at) override;

    bool do_is_equal(const std::pmr::memory_resource& other) const noexcept override;

public:
    
    inline void set_fit_mode(
        allocator_with_fit_mode::fit_mode mode) override;

public:
    
    std::vector<allocator_test_utils::block_info> get_blocks_info() const;
private:


/** TODO: Highly recommended for helper functions to return references */

    logger *get_logger() const override;
    std::string get_typename() const noexcept override;
    allocator_with_fit_mode::fit_mode& get_fit_mod() const noexcept;
    std::string print_blocks() const;
    void* get_first(size_t s) const noexcept;
    void* get_best(size_t s) const noexcept;
    void* get_worst(size_t s) const noexcept;
    static logger*& get_logger_ptr_ref(void* t);
    static std::pmr::memory_resource*& get_parent_allocator_ptr_ref(void* t);
    static allocator_with_fit_mode::fit_mode& get_static_fit_mode_ref(void* t);
    static size_t& get_user_space_size_ref(void* t);
    static void*& get_first_occupied_block_ptr_ref(void* t);
    size_t& get_occupied_user_size_ref(void* bm);
    void*& get_occupied_prev_ptr_ref(void* bm);
    void*& get_occupied_next_ptr_ref(void* bm);
    static void* get_pool_start(void* t);
    static void* get_pool_end(void* t);
    static void* get_user_ptr_from_meta(void* bm);
    static void* get_meta_ptr_from_user(void* up);
    static size_t& get_block_user_size_ref(void* bm);
    static void*& get_block_prev_ptr_ref(void* bm);
    static void*& get_block_next_ptr_ref(void* bm);
    static void*& get_block_allocator_ptr_ref(void* bm);

    std::mutex& get_mutex() const noexcept;
    static size_t get_next_free_size(void* os, void* t) noexcept;
    size_t get_free_size() const noexcept;

    class boundary_iterator
    {
        void* _occupied_ptr;
        bool _occupied;
        void* _trusted_memory;

    public:

        using iterator_category = std::bidirectional_iterator_tag;
        using value_type = void*;
        using reference = void*&;
        using pointer = void**;
        using difference_type = ptrdiff_t;

        bool operator==(const boundary_iterator&) const noexcept;

        bool operator!=(const boundary_iterator&) const noexcept;

        boundary_iterator& operator++() & noexcept;

        boundary_iterator& operator--() & noexcept;

        boundary_iterator operator++(int n);

        boundary_iterator operator--(int n);

        size_t size() const noexcept;

        bool occupied() const noexcept;

        void* operator*() const noexcept;

        void* get_ptr() const noexcept;

        boundary_iterator();

        boundary_iterator(void* trusted);
    };

    friend class boundary_iterator;

    boundary_iterator begin() const noexcept;

    boundary_iterator end() const noexcept;
};

#endif //MATH_PRACTICE_AND_OPERATING_SYSTEMS_ALLOCATOR_ALLOCATOR_BOUNDARY_TAGS_H