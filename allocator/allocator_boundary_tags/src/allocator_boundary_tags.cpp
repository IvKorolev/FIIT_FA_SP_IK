#include <not_implemented.h>
#include "../include/allocator_boundary_tags.h"
#include <new>
#include <stdexcept>
#include <utility>
#include <vector>
#include <sstream>
#include <iomanip>
#include <memory>
#include <cstddef>
#include <mutex>
#include <string>
#include <numeric>

namespace
{
    std::string pointer_to_string_local(const void* ptr)
    {
        std::ostringstream oss;
        oss << ptr;
        return oss.str();
    }
}

inline logger*& allocator_boundary_tags::get_logger_ptr_ref(void* t) {
    if (!t)
    {
        throw std::logic_error("get_logger_ptr_ref(null)");
    }
    return *reinterpret_cast<logger**>(t);
}

inline std::pmr::memory_resource*& allocator_boundary_tags::get_parent_allocator_ptr_ref(void* t) {
    if (!t)
    {
        throw std::logic_error("get_parent_allocator_ptr_ref(null)");
    }
    size_t o = sizeof(logger*);
    return *reinterpret_cast<std::pmr::memory_resource**>(reinterpret_cast<std::byte*>(t) + o);
}

inline allocator_with_fit_mode::fit_mode& allocator_boundary_tags::get_static_fit_mode_ref(void* t) {
    if (!t)
    {
        throw std::logic_error("get_static_fit_mode_ref(null)");
    }
    size_t o = sizeof(logger*) + sizeof(std::pmr::memory_resource*);
    return *reinterpret_cast<allocator_with_fit_mode::fit_mode*>(reinterpret_cast<std::byte*>(t) + o);
}

inline size_t& allocator_boundary_tags::get_user_space_size_ref(void* t) {
    if (!t)
    {
        throw std::logic_error("get_user_space_size_ref(null)");
    }
    size_t o = sizeof(logger*) + sizeof(std::pmr::memory_resource*) + sizeof(allocator_with_fit_mode::fit_mode);
    return *reinterpret_cast<size_t*>(reinterpret_cast<std::byte*>(t) + o);
}

inline void*& allocator_boundary_tags::get_first_occupied_block_ptr_ref(void* t) {
    if (!t)
    {
        throw std::logic_error("get_first_occupied_block_ptr_ref(null)");
    }
    size_t o = sizeof(logger*) + sizeof(std::pmr::memory_resource*) + sizeof(allocator_with_fit_mode::fit_mode) + sizeof(size_t);
    return *reinterpret_cast<void**>(reinterpret_cast<std::byte*>(t) + o);
}

inline size_t& allocator_boundary_tags::get_occupied_user_size_ref(void* bm) {
     if (!bm)
     {
         throw std::logic_error("get_occupied_user_size_ref(null)");
     }
     return *reinterpret_cast<size_t*>(reinterpret_cast<std::byte*>(bm));
}

inline void*& allocator_boundary_tags::get_occupied_prev_ptr_ref(void* bm) {
    if (!bm)
    {
        throw std::logic_error("get_occupied_prev_ptr_ref(null)");
    }
    size_t o = sizeof(size_t);
    return *reinterpret_cast<void**>(reinterpret_cast<std::byte*>(bm) + o);
}

inline void*& allocator_boundary_tags::get_occupied_next_ptr_ref(void* bm) {
    if (!bm)
    {
        throw std::logic_error("get_occupied_next_ptr_ref(null)");
    }
    size_t o = sizeof(size_t) + sizeof(void*);
    return *reinterpret_cast<void**>(reinterpret_cast<std::byte*>(bm) + o);
}

inline void* allocator_boundary_tags::get_pool_start(void* t) {
    if (!t)
    {
        throw std::logic_error("get_pool_start(null)");
    }
    return reinterpret_cast<std::byte*>(t) + allocator_metadata_size;
}

inline void* allocator_boundary_tags::get_pool_end(void* t) {
    if (!t)
    {
        throw std::logic_error("get_pool_end(null)");
    }
    return reinterpret_cast<std::byte*>(get_pool_start(t)) + get_user_space_size_ref(t);

}

inline size_t& allocator_boundary_tags::get_block_user_size_ref(void* bm) {
    if (!bm)
    {
        throw std::logic_error("get_block_user_size_ref(null)");
    }
    return *reinterpret_cast<size_t*>(reinterpret_cast<std::byte*>(bm));
}

inline void*& allocator_boundary_tags::get_block_prev_ptr_ref(void* bm) {
    if (!bm)
    {
        throw std::logic_error("get_block_prev_ptr_ref(null)");
    }
    size_t o = sizeof(size_t);
    return *reinterpret_cast<void**>(reinterpret_cast<std::byte*>(bm) + o);
}

inline void*& allocator_boundary_tags::get_block_next_ptr_ref(void* bm) {
    if (!bm)
    {
        throw std::logic_error("get_block_next_ptr_ref(null)");
    }
    size_t o = sizeof(size_t) + sizeof(void*);
    return *reinterpret_cast<void**>(reinterpret_cast<std::byte*>(bm) + o);
}

inline void*& allocator_boundary_tags::get_block_allocator_ptr_ref(void* bm) {
    if (!bm)
    {
        throw std::logic_error("get_block_allocator_ptr_ref(null)");
    }
    size_t o = sizeof(size_t) + sizeof(void*) + sizeof(void*);
    return *reinterpret_cast<void**>(reinterpret_cast<std::byte*>(bm) + o);
}

inline void* allocator_boundary_tags::get_user_ptr_from_meta(void* bm) {
    if (!bm)
    {
        throw std::logic_error("get_user_ptr_from_meta(null)");
    }
    return reinterpret_cast<std::byte*>(bm) + occupied_block_metadata_size;
}

inline void* allocator_boundary_tags::get_meta_ptr_from_user(void* up) {
    if (!up)
    {
        throw std::logic_error("get_meta_ptr_from_user(null)");
    }
    return reinterpret_cast<std::byte*>(up) - occupied_block_metadata_size;
}

inline allocator_with_fit_mode::fit_mode& allocator_boundary_tags::get_fit_mod() const noexcept
{
    try {
        if (!_trusted_memory) {
            static fit_mode dummy = fit_mode::first_fit;
            return dummy;
        }
        return get_static_fit_mode_ref(_trusted_memory);
    }
    catch(...) {
        static fit_mode dummy = fit_mode::first_fit;
        return dummy;
    }
}

inline std::mutex& allocator_boundary_tags::get_mutex() const noexcept
{
    return const_cast<std::mutex&>(_allocator_mutex);
}

inline size_t allocator_boundary_tags::get_next_free_size(void* prev_occupied_meta, void* t) noexcept
{
    if (!t)
    {
        return 0;
    }

    void* gap_start = nullptr;
    void* next_occupied_meta = nullptr;

    try {
        if (prev_occupied_meta == t) {
            gap_start = get_pool_start(t);
        } else {
            if (!prev_occupied_meta)
            {
                return 0;
            }
            size_t prev_user_size = get_block_user_size_ref(prev_occupied_meta);
            gap_start = reinterpret_cast<std::byte*>(prev_occupied_meta) + occupied_block_metadata_size + prev_user_size;
        }
        if (prev_occupied_meta == t) {
            next_occupied_meta = get_first_occupied_block_ptr_ref(t);
        } else {
            if (!prev_occupied_meta)
            {
                return 0;
            }
            next_occupied_meta = get_block_next_ptr_ref(prev_occupied_meta);
        }

        void* gap_end = (next_occupied_meta == nullptr) ? get_pool_end(t) : next_occupied_meta;
        if (reinterpret_cast<std::byte*>(gap_start) >= reinterpret_cast<std::byte*>(gap_end)) {
            return 0;
        }
        return reinterpret_cast<std::byte*>(gap_end) - reinterpret_cast<std::byte*>(gap_start);

    } catch (const std::exception& e) {
        return 0;
    }
}

inline void* allocator_boundary_tags::get_first(size_t s) const noexcept
{
    logger* log = get_logger();
    if (log)
    {
        log->trace("get_first started. Required size: " + std::to_string(s));
    }

    if (!_trusted_memory) {
         if (log)
         {
             log->trace("get_first finished: Not initialized.");
         }
        return nullptr;
    }

    void* current_occupied = nullptr;
    void* prev_occupied = nullptr;

    try {
        prev_occupied = _trusted_memory;
        current_occupied = get_first_occupied_block_ptr_ref(_trusted_memory);

        while (true) {
            size_t gap_size = get_next_free_size(prev_occupied, _trusted_memory);
            if (gap_size >= s) {
                 if (log)
                 {
                     log->trace("get_first finished: Found gap after " + pointer_to_string_local(prev_occupied) + " with size " + std::to_string(gap_size));
                 }
                return prev_occupied;
            }
            if (current_occupied == nullptr) {
                 if (log)
                 {
                     log->trace("get_first finished: No suitable gap found.");
                 }
                break;
            }
            prev_occupied = current_occupied;
            current_occupied = get_block_next_ptr_ref(current_occupied);
        }
    } catch (const std::exception& e) {
         if (log)
         {
             log->error("get_first exception: " + std::string(e.what()));
         }
         return nullptr;
    } catch (...) {
         if (log)
         {
             log->error("get_first unknown exception.");
         }
         return nullptr;
    }
    if (log)
    {
        log->trace("get_first finished: No suitable gap found.");
    }
    return nullptr;
}

inline void* allocator_boundary_tags::get_best(size_t s) const noexcept
{
    logger* log = get_logger();
    if (log)
    {
        log->trace("get_best started. Required size: " + std::to_string(s));
    }

    if (!_trusted_memory) {
         if (log)
         {
             log->trace("get_best finished: Not initialized.");
         }
        return nullptr;
    }

    void* best_prev_block = nullptr;
    size_t min_suitable_gap = static_cast<size_t>(-1);

    void* current_occupied = nullptr;
    void* prev_occupied = nullptr;

    try {
        prev_occupied = _trusted_memory;
        current_occupied = get_first_occupied_block_ptr_ref(_trusted_memory);

        while (true) {
            size_t gap_size = get_next_free_size(prev_occupied, _trusted_memory);

            if (gap_size >= s) {
                if (!best_prev_block || gap_size < min_suitable_gap) {
                    min_suitable_gap = gap_size;
                    best_prev_block = prev_occupied;
                }
            }

            if (current_occupied == nullptr) {
                break;
            }
            prev_occupied = current_occupied;
            current_occupied = get_block_next_ptr_ref(current_occupied);
        }
    } catch (const std::exception& e) {
        if (log)
        {
            log->error("get_best exception: " + std::string(e.what()));
        }
        return nullptr;
    } catch (...) {
        if (log)
        {
            log->error("get_best unknown exception.");
        }
        return nullptr;
    }

     if (log)
     {
         log->trace("get_best finished. Best gap found after: " + (best_prev_block ? pointer_to_string_local(best_prev_block) : "None"));
     }
    return best_prev_block;
}

inline void* allocator_boundary_tags::get_worst(size_t s) const noexcept
{
    logger* log = get_logger();
    if (log)
    {
        log->trace("get_worst started. Required size: " + std::to_string(s));
    }

    if (!_trusted_memory) {
        if (log)
        {
            log->trace("get_worst finished: Not initialized.");
        }
        return nullptr;
    }

    void* worst_prev_block = nullptr;
    size_t max_suitable_gap = 0;

    void* current_occupied = nullptr;
    void* prev_occupied = nullptr;

    try {
        prev_occupied = _trusted_memory;
        current_occupied = get_first_occupied_block_ptr_ref(_trusted_memory);

        while (true) {
            size_t gap_size = get_next_free_size(prev_occupied, _trusted_memory);

            if (gap_size >= s) {
                 if (!worst_prev_block || gap_size > max_suitable_gap) {
                    max_suitable_gap = gap_size;
                    worst_prev_block = prev_occupied;
                }
            }

            if (current_occupied == nullptr) {
                break;
            }
            prev_occupied = current_occupied;
            current_occupied = get_block_next_ptr_ref(current_occupied);
        }
    } catch (const std::exception& e) {
        if (log)
        {
            log->error("get_worst exception: " + std::string(e.what()));
        }
        return nullptr;
    } catch (...) {
        if (log)
        {
            log->error("get_worst unknown exception.");
        }
        return nullptr;
    }

    if (log)
    {
        log->trace("get_worst finished. Worst gap found after: " + (worst_prev_block ? pointer_to_string_local(worst_prev_block) : "None"));
    }
    return worst_prev_block;
}

inline size_t allocator_boundary_tags::get_free_size() const noexcept
{
    logger* log = get_logger();
    if (log)
    {
        log->trace("get_free_size started.");
    }

    if (!_trusted_memory)
    {
        return 0;
    }

    size_t total_free_area = 0;
    void* current_occupied = nullptr;
    void* prev_occupied = nullptr;

    try {
        prev_occupied = _trusted_memory;
        current_occupied = get_first_occupied_block_ptr_ref(_trusted_memory);

        while (true) {
            total_free_area += get_next_free_size(prev_occupied, _trusted_memory);

            if (current_occupied == nullptr) {
                break;
            }
            prev_occupied = current_occupied;
            current_occupied = get_block_next_ptr_ref(current_occupied);
        }
    } catch (const std::exception& e) {
        return 0;
    }

    if (log)
    {
        log->trace("get_free_size finished. Total free: " + std::to_string(total_free_area));
    }
    return total_free_area;
}

std::string allocator_boundary_tags::print_blocks() const
{
    logger* log = get_logger();
    if (log)
    {
        log->trace("print_blocks started.");
    }

    if (!_trusted_memory)
    {
        return "[]";
    }

    std::stringstream ss;
    ss << "[";
    bool first_block = true;

    void* current_occupied_meta = nullptr;
    void* last_block_processed_end = nullptr;
    void* pool_start = nullptr;
    void* pool_end = nullptr;

    try {
        pool_start = get_pool_start(_trusted_memory);
        pool_end = get_pool_end(_trusted_memory);
        current_occupied_meta = get_first_occupied_block_ptr_ref(_trusted_memory);
        last_block_processed_end = pool_start;

        while (true) {
            void* free_block_start = last_block_processed_end;
            void* free_block_end = (current_occupied_meta == nullptr) ? pool_end : current_occupied_meta;

            if (reinterpret_cast<std::byte*>(free_block_start) < reinterpret_cast<std::byte*>(free_block_end)) {
                size_t free_size = reinterpret_cast<std::byte*>(free_block_end) - reinterpret_cast<std::byte*>(free_block_start);
                if (!first_block)
                {
                    ss << "|";
                }
                ss << "avail " << free_size;
                first_block = false;
            }
            if (current_occupied_meta == nullptr)
            {
                break;
            }
            size_t user_size = get_block_user_size_ref(current_occupied_meta);
            if (!first_block)
            {
                ss << "|";
            }
            ss << "occup " << user_size;
            first_block = false;
            last_block_processed_end = reinterpret_cast<std::byte*>(current_occupied_meta) + occupied_block_metadata_size + user_size;
            current_occupied_meta = get_block_next_ptr_ref(current_occupied_meta);
        }
    } catch (const std::exception& e) {
        if (log)
        {
            log->error("Exception during print_blocks: " + std::string(e.what()));
        }
    } catch (...) {
        if (log)
        {
            log->error("Unknown exception during print_blocks.");
        }
    }

    ss << "]";
    return ss.str();
}

logger* allocator_boundary_tags::get_logger() const
{
    if (!_trusted_memory) return nullptr;
    try {
        return get_logger_ptr_ref(_trusted_memory);
    } catch(...) {
        return nullptr;
    }
}


inline std::string allocator_boundary_tags::get_typename() const noexcept
{
    return "allocator_boundary_tags";
}

inline void allocator_boundary_tags::set_fit_mode(allocator_with_fit_mode::fit_mode m)
{
    debug_with_guard("set_fit_mode started. Mode: " + std::to_string(static_cast<int>(m)));
    std::lock_guard lock(get_mutex());
    trace_with_guard("set_fit_mode: Lock acquired.");
    try {
        if (!_trusted_memory) {
             error_with_guard("set_fit_mode failed: Allocator not initialized.");
             debug_with_guard("set_fit_mode finished (error: not initialized).");
             return;
        }
        allocator_with_fit_mode::fit_mode& current_mode = get_static_fit_mode_ref(_trusted_memory);
        current_mode = m;
        information_with_guard("Fit mode successfully set to: " + std::to_string(static_cast<int>(m)));
        trace_with_guard("set_fit_mode: Releasing lock.");
    } catch (const std::exception& e) {
        error_with_guard(std::string("set_fit_mode failed with exception: ") + e.what());
    } catch (...) {
        error_with_guard("set_fit_mode failed with unknown exception.");
    }
    debug_with_guard("set_fit_mode finished.");
}

allocator_boundary_tags::~allocator_boundary_tags()
{
    logger* log = get_logger();
    if (log) log->debug("Destructor started for allocator at " + pointer_to_string_local(this) + " managing memory " + pointer_to_string_local(_trusted_memory));

    if (!_trusted_memory) {
        if (log) log->debug("Destructor finished (was not initialized or already moved).");
        return;
    }
    void* memory_to_free = _trusted_memory;
    std::pmr::memory_resource* parent_alloc = nullptr;
    size_t total_allocated_size = 0;
    bool error_getting_metadata = false;

    try {
        parent_alloc = get_parent_allocator_ptr_ref(memory_to_free);
        size_t user_size = get_user_space_size_ref(memory_to_free);
        total_allocated_size = user_size + allocator_metadata_size;
        if (log)
        {
            log->trace("Destructor: Parent allocator: " + pointer_to_string_local(parent_alloc) + ", Total size to free: " + std::to_string(total_allocated_size));
        }
    } catch(const std::exception& e) {
         if (log)
         {
             log->error("Destructor: Error getting metadata: " + std::string(e.what()));
         }
         error_getting_metadata = true;
    } catch (...) {
         if (log)
         {
             log->error("Destructor: Unknown error getting metadata.");
         }
         error_getting_metadata = true;
    }
    if (!error_getting_metadata) {
        try {
            if (parent_alloc) {
                if (log) log->trace("Destructor: Deallocating using parent allocator.");
                parent_alloc->deallocate(memory_to_free, total_allocated_size, alignof(std::max_align_t));
            } else {
                 if (log)
                 {
                     log->trace("Destructor: Deallocating using global operator delete.");
                 }
                ::operator delete(memory_to_free, total_allocated_size, std::align_val_t(alignof(std::max_align_t)));
            }
            if (log)
            {
                log->trace("Destructor: Deallocation successful.");
            }
        } catch (const std::exception& e) {
            if (log)
            {
                log->critical("Destructor: CRITICAL ERROR during deallocation: " + std::string(e.what()));
            }
        } catch (...) {
             if (log)
             {
                 log->critical("Destructor: CRITICAL UNKNOWN ERROR during deallocation.");
             }
        }
    } else {
         if (log)
         {
             log->critical("Destructor: Could not deallocate memory due to earlier metadata read error.");
         }
    }
    _trusted_memory = nullptr;
    if (log)
    {
        log->debug("Destructor finished.");
    }
}

allocator_boundary_tags::allocator_boundary_tags(allocator_boundary_tags &&other) noexcept
    : logger_guardant(std::move(other)),
      typename_holder(std::move(other)),
      _trusted_memory(std::exchange(other._trusted_memory, nullptr))
{
    logger* log = get_logger();
    if (log) {
        log->debug("Move constructor: Moved allocator state from " + pointer_to_string_local(&other) + " to " + pointer_to_string_local(this));
    }
}

allocator_boundary_tags &allocator_boundary_tags::operator=(allocator_boundary_tags &&other) noexcept
{
    logger* log_this_before = get_logger();
    if (log_this_before)
    {
        log_this_before->debug("Move assignment: Assigning from " + pointer_to_string_local(&other) + " to " + pointer_to_string_local(this));
    }

    if (this != &other) {
        if (_trusted_memory) {
             if (log_this_before)
             {
                 log_this_before->trace("Move assignment: Destroying current content.");
             }
             this->~allocator_boundary_tags();
             if (_trusted_memory) {
                 this->~allocator_boundary_tags();
                 _trusted_memory = nullptr;
             }
        }
        logger_guardant::operator=(std::move(other));
         typename_holder::operator=(std::move(other));
        _trusted_memory = std::exchange(other._trusted_memory, nullptr);

        logger* log_this_after = get_logger();
        if (log_this_after)
        {
            log_this_after->trace("Move assignment: State moved. Current memory: " + pointer_to_string_local(_trusted_memory));
        }
    } else {
         if (log_this_before)
         {
             log_this_before->debug("Move assignment: Self-assignment detected, no action taken.");
         }
    }
    return *this;
}

allocator_boundary_tags::allocator_boundary_tags(
    size_t space_size,
    std::pmr::memory_resource *parent_allocator,
    logger* logger_instance,
    allocator_with_fit_mode::fit_mode allocate_fit_mode)
: logger_guardant(),
  typename_holder(),
 _trusted_memory(nullptr)
{
    if (logger_instance)
    {
        logger_instance->debug("Constructor started. Requested space: " + std::to_string(space_size));
    }
    if (space_size < occupied_block_metadata_size) {
        std::string msg = "Requested space size " + std::to_string(space_size) + " is too small (less than occupied block metadata " + std::to_string(occupied_block_metadata_size) + ")";
        if (logger_instance)
        {
            logger_instance->error("Constructor exception: " + msg);
        }
        throw std::logic_error(msg);
    }

    size_t total_size_needed = space_size + allocator_metadata_size;
    if (logger_instance)
    {
        logger_instance->trace("Constructor: Total memory needed (user + allocator meta): " + std::to_string(total_size_needed));
    }
    std::pmr::memory_resource *alloc_to_use = parent_allocator ? parent_allocator : std::pmr::get_default_resource();
    if (logger_instance)
    {
        logger_instance->trace("Constructor: Using memory resource at " + pointer_to_string_local(alloc_to_use));
    }
    void* allocated_memory = nullptr;
    try {
        if (logger_instance)
        {
            logger_instance->trace("Constructor: Attempting allocation...");
        }
        allocated_memory = alloc_to_use->allocate(total_size_needed, alignof(std::max_align_t));
        if (logger_instance)
        {
            logger_instance->trace("Constructor: Allocation successful. Allocated memory at: " + pointer_to_string_local(allocated_memory));
        }
    } catch (const std::bad_alloc& e) {
        if (logger_instance)
        {
            logger_instance->error(std::string("Constructor exception: Failed to allocate memory pool - ") + e.what());
        }
        throw;
    } catch (const std::exception& e) {
        if (logger_instance)
        {
            logger_instance->error(std::string("Constructor exception: Unexpected error during pool allocation - ") + e.what());
        }
        throw;
    } catch (...) {
        if (logger_instance)
        {
            logger_instance->error("Constructor exception: Unknown error during pool allocation.");
        }
        throw;
    }
    if (!allocated_memory) {
         std::string msg = "Memory allocation returned nullptr unexpectedly.";
         if (logger_instance)
         {
             logger_instance->critical("Constructor critical error: " + msg);
         }
         throw std::runtime_error(msg);
    }
    _trusted_memory = allocated_memory;
    try {
        if (logger_instance)
        {
            logger_instance->trace("Constructor: Writing allocator metadata...");
        }
        get_logger_ptr_ref(_trusted_memory) = logger_instance;
        get_parent_allocator_ptr_ref(_trusted_memory) = alloc_to_use;
        get_static_fit_mode_ref(_trusted_memory) = allocate_fit_mode;
        get_user_space_size_ref(_trusted_memory) = space_size;
        get_first_occupied_block_ptr_ref(_trusted_memory) = nullptr;
        if (logger_instance)
        {
            logger_instance->trace("Constructor: Allocator metadata written successfully.");
        }
    } catch (const std::exception& e) {
        std::string msg = std::string("Failed to write allocator metadata after allocation: ") + e.what();
        if (logger_instance)
        {
            logger_instance->critical("Constructor critical error: " + msg);
        }
        try {
             alloc_to_use->deallocate(_trusted_memory, total_size_needed, alignof(std::max_align_t));
        } catch (...) {
             if (logger_instance)
             {
                 logger_instance->critical("Constructor critical error: Failed to deallocate memory during error handling!");
             }
        }
        _trusted_memory = nullptr;
        throw std::runtime_error(msg);
    } catch (...) {
         std::string msg = "Unknown error writing allocator metadata.";
         if (logger_instance)
         {
             logger_instance->critical("Constructor critical error: " + msg);
         }
         try {
             alloc_to_use->deallocate(_trusted_memory, total_size_needed, alignof(std::max_align_t));
         } catch(...) {}
         _trusted_memory = nullptr;
         throw std::runtime_error(msg);
    }
    debug_with_guard("Constructor finished successfully.");
    information_with_guard("Initial available memory: " + std::to_string(get_free_size()) + " bytes.");
    debug_with_guard("Initial memory state: " + print_blocks());
}

[[nodiscard]] void* allocator_boundary_tags::do_allocate_sm(size_t bytes)
{
    debug_with_guard("do_allocate_sm started. Requesting user size: " + std::to_string(bytes) + " bytes.");

    if (!_trusted_memory) {
        error_with_guard("Allocation failed: Allocator not initialized.");
        throw std::bad_alloc();
    }

    std::lock_guard lock(get_mutex());
    trace_with_guard("do_allocate_sm: Lock acquired.");

    size_t value_size = bytes;
    if (value_size == 0) {
        warning_with_guard("Allocation request for 0 bytes was redefined to allocate user size 0 bytes.");
    }

    size_t requested_total_size = value_size + occupied_block_metadata_size;
    trace_with_guard("Total size needed (user=" + std::to_string(value_size) + " + meta=" + std::to_string(occupied_block_metadata_size) + "): " + std::to_string(requested_total_size));

    void* prev_occupied = nullptr;
    allocator_with_fit_mode::fit_mode current_mode = get_fit_mod();
    trace_with_guard("Finding gap using mode: " + std::to_string(static_cast<int>(current_mode)));

    void* user_ptr = nullptr;

    try {
        switch (current_mode) {
             case fit_mode::first_fit: prev_occupied = get_first(requested_total_size); break;
             case fit_mode::the_best_fit: prev_occupied = get_best(requested_total_size); break;
             case fit_mode::the_worst_fit: prev_occupied = get_worst(requested_total_size); break;
            default:
                error_with_guard("Internal error: Unknown fit mode encountered: " + std::to_string(static_cast<int>(current_mode)));
                throw std::logic_error("Unknown fit mode");
        }

        if (prev_occupied == nullptr) {
            std::string msg = "No suitable free gap found for required total size " + std::to_string(requested_total_size) + " bytes.";
            error_with_guard("Allocation failed: " + msg);
            throw std::bad_alloc();
        }
        trace_with_guard("Found suitable gap after block/meta: " + pointer_to_string_local(prev_occupied));
        size_t free_block_size = get_next_free_size(prev_occupied, _trusted_memory);
        trace_with_guard("Available gap size: " + std::to_string(free_block_size));

        size_t actual_total_size_to_use = requested_total_size;
        size_t user_size_to_write = value_size;
        if (free_block_size < requested_total_size + occupied_block_metadata_size) {
             warning_with_guard("Original logic applied: Gap size " + std::to_string(free_block_size) + " is less than (requested_total=" + std::to_string(requested_total_size) + " + meta=" + std::to_string(occupied_block_metadata_size) + "). Adjusting allocation size down to fit the gap.");
             actual_total_size_to_use = free_block_size;
             if (actual_total_size_to_use < occupied_block_metadata_size) {
                 error_with_guard("Adjusted total size ("+ std::to_string(actual_total_size_to_use) +") is too small even for metadata ("+ std::to_string(occupied_block_metadata_size) +"). Cannot allocate.");
                 throw std::bad_alloc();
             }
             user_size_to_write = actual_total_size_to_use - occupied_block_metadata_size;
             trace_with_guard("Adjusted user size to write: " + std::to_string(user_size_to_write));
        }
        void* new_block_meta_start = nullptr;
        if (prev_occupied == _trusted_memory) {
            new_block_meta_start = get_pool_start(_trusted_memory);
        } else {
            size_t prev_occupied_user_size = get_occupied_user_size_ref(prev_occupied);
            new_block_meta_start = reinterpret_cast<std::byte*>(prev_occupied) + occupied_block_metadata_size + prev_occupied_user_size;
        }
        trace_with_guard("New block meta will start at: " + pointer_to_string_local(new_block_meta_start));
        trace_with_guard("Writing new block metadata: user_size=" + std::to_string(user_size_to_write));
        get_block_user_size_ref(new_block_meta_start) = user_size_to_write;
        get_block_prev_ptr_ref(new_block_meta_start) = prev_occupied;
        void* next_block_meta = (prev_occupied == _trusted_memory)
                               ? get_first_occupied_block_ptr_ref(_trusted_memory)
                               : get_block_next_ptr_ref(prev_occupied);
        get_block_next_ptr_ref(new_block_meta_start) = next_block_meta;
        get_block_allocator_ptr_ref(new_block_meta_start) = _trusted_memory;
        trace_with_guard("Metadata written. Prev=" + pointer_to_string_local(prev_occupied) + ", Next=" + pointer_to_string_local(next_block_meta));
        trace_with_guard("Updating linked list pointers...");
        if (prev_occupied == _trusted_memory) {
            get_first_occupied_block_ptr_ref(_trusted_memory) = new_block_meta_start;
        } else {
            get_block_next_ptr_ref(prev_occupied) = new_block_meta_start;
        }
        if (next_block_meta != nullptr) {
            get_block_prev_ptr_ref(next_block_meta) = new_block_meta_start;
        }
        trace_with_guard("List updated.");
        user_ptr = get_user_ptr_from_meta(new_block_meta_start);
        trace_with_guard("Calculated user pointer: " + pointer_to_string_local(user_ptr)); // Добавим trace
        size_t current_free_size = 0;
        std::string current_state_string = "[Error getting state]";
        try {
            current_free_size = get_free_size();
            trace_with_guard("get_free_size() successful.");
        } catch(...) { trace_with_guard("get_free_size() failed!"); }
        try {
            current_state_string = print_blocks();
            trace_with_guard("print_blocks() successful.");
        } catch(...) { trace_with_guard("print_blocks() failed!"); }

        information_with_guard("Available memory after alloc: " + std::to_string(current_free_size) + " bytes.");
        debug_with_guard("Memory state after alloc: " + current_state_string);
        trace_with_guard("do_allocate_sm: Releasing lock.");
        debug_with_guard("do_allocate_sm finished: Returning user pointer " + pointer_to_string_local(user_ptr));
        return user_ptr;

    } catch (const std::bad_alloc& e) {
        error_with_guard(std::string("Caught bad_alloc during allocation process: ") + e.what());
        trace_with_guard("do_allocate_sm: Releasing lock (exception).");
        debug_with_guard("do_allocate_sm finished (exception).");
        throw;
    } catch (const std::logic_error& e) {
         error_with_guard(std::string("Caught logic_error during allocation process: ") + e.what());
         trace_with_guard("do_allocate_sm: Releasing lock (exception).");
         debug_with_guard("do_allocate_sm finished (exception).");
         throw std::bad_alloc();
    } catch (const std::exception& e) {
        error_with_guard(std::string("Caught std::exception during allocation process: ") + e.what());
        trace_with_guard("do_allocate_sm: Releasing lock (exception).");
        debug_with_guard("do_allocate_sm finished (exception).");
        throw std::bad_alloc();
    } catch (...) {
        error_with_guard("Caught unknown exception during allocation process.");
        trace_with_guard("do_allocate_sm: Releasing lock (exception).");
        debug_with_guard("do_allocate_sm finished (exception).");
        throw std::bad_alloc();
    }
}

void allocator_boundary_tags::do_deallocate_sm(void *at)
{
    debug_with_guard("do_deallocate_sm started for user pointer " + pointer_to_string_local(at));

    if (at == nullptr) {
        warning_with_guard("Attempt to deallocate nullptr. Ignoring.");
        debug_with_guard("do_deallocate_sm finished (nullptr ignored).");
        return;
    }
    if (!_trusted_memory) {
        error_with_guard("Deallocation failed: Allocator not initialized.");
        throw std::logic_error("Deallocation failed: Allocator not initialized.");
    }

    std::lock_guard lock(get_mutex());
    trace_with_guard("do_deallocate_sm: Lock acquired.");

    void* block_meta_ptr = nullptr;

    try {
        trace_with_guard("Starting pointer validation.");
        void* pool_start = get_pool_start(_trusted_memory);
        void* pool_end = get_pool_end(_trusted_memory);
        void* user_pool_start = get_user_ptr_from_meta(pool_start);
        if (reinterpret_cast<std::byte*>(at) < reinterpret_cast<std::byte*>(user_pool_start) ||
            reinterpret_cast<std::byte*>(at) >= reinterpret_cast<std::byte*>(pool_end)) {
             throw std::logic_error("Pointer " + pointer_to_string_local(at) + " out of managed pool range");
        }
        trace_with_guard("Pointer is within pool range.");

        block_meta_ptr = get_meta_ptr_from_user(at);
        trace_with_guard("Calculated block meta pointer: " + pointer_to_string_local(block_meta_ptr));
        void* parent_ptr = get_block_allocator_ptr_ref(block_meta_ptr);
        if (parent_ptr != _trusted_memory) {
             throw std::logic_error("Block parent mismatch");
        }
        trace_with_guard("Block parent pointer matches.");
        trace_with_guard("Checking if block is in the occupied list...");
        bool found = false;
        void* curr = get_first_occupied_block_ptr_ref(_trusted_memory);
         int sanity_check = 0; const int max_iterations = 1000000;
         while(curr) {
              if (sanity_check++ > max_iterations) {
                  error_with_guard("Potential infinite loop detected in occupied list during deallocation check!");
                  throw std::runtime_error("List corruption (potential cycle) detected");
              }
             if(curr == block_meta_ptr) { found = true; break; }
             curr = get_block_next_ptr_ref(curr);
         }
        if (!found) {
            throw std::logic_error("Block not found in occupied list (already deallocated?)");
        }
        trace_with_guard("Block found in occupied list. Validation successful.");
        trace_with_guard("Unlinking block " + pointer_to_string_local(block_meta_ptr) + "...");
        void* prev_meta = get_block_prev_ptr_ref(block_meta_ptr);
        void* next_meta = get_block_next_ptr_ref(block_meta_ptr);
        if (prev_meta == _trusted_memory) {
            get_first_occupied_block_ptr_ref(_trusted_memory) = next_meta;
        } else {
            get_block_next_ptr_ref(prev_meta) = next_meta;
        }
        if (next_meta != nullptr) {
            get_block_prev_ptr_ref(next_meta) = prev_meta;
        }
        trace_with_guard("Block successfully unlinked.");
        size_t current_free_size = 0;
        std::string current_state_string = "[Error getting state]";
         try {
             current_free_size = get_free_size();
             trace_with_guard("get_free_size() successful.");
         } catch(...) { trace_with_guard("get_free_size() failed!"); }
         try {
             current_state_string = print_blocks();
             trace_with_guard("print_blocks() successful.");
         } catch(...) { trace_with_guard("print_blocks() failed!"); }

        information_with_guard("Available memory after dealloc: " + std::to_string(current_free_size) + " bytes.");
        debug_with_guard("Memory state after dealloc: " + current_state_string);
        trace_with_guard("do_deallocate_sm: Releasing lock.");
        debug_with_guard("do_deallocate_sm finished successfully.");
    } catch (const std::logic_error& e) {
         std::string msg = std::string("Logic error during deallocation: ") + e.what();
         error_with_guard(msg);
         trace_with_guard("do_deallocate_sm: Releasing lock (exception).");
         debug_with_guard("do_deallocate_sm finished (exception).");
         throw;
    } catch (const std::runtime_error& e) {
         std::string msg = std::string("Runtime error during deallocation: ") + e.what();
         error_with_guard(msg);
         critical_with_guard("Allocator state potentially corrupted: " + msg);
         trace_with_guard("do_deallocate_sm: Releasing lock (runtime_error exception).");
         debug_with_guard("do_deallocate_sm finished (runtime_error).");
         throw;
    } catch (const std::exception& e) {
        std::string msg = std::string("Unexpected std::exception during deallocation: ") + e.what();
        error_with_guard(msg);
        critical_with_guard("Allocator state potentially corrupted: " + msg);
        trace_with_guard("do_deallocate_sm: Releasing lock (std::exception).");
        debug_with_guard("do_deallocate_sm finished (std::exception).");
    } catch (...) {
        error_with_guard("Unknown error during deallocation.");
        critical_with_guard("Allocator state potentially corrupted after unknown error.");
        trace_with_guard("do_deallocate_sm: Releasing lock (unknown exception).");
        debug_with_guard("do_deallocate_sm finished (unknown exception).");
    }
}

std::vector<allocator_test_utils::block_info> allocator_boundary_tags::get_blocks_info() const
{
    logger* log = get_logger();
    if (log)
    {
        log->debug("get_blocks_info started.");
    }
    std::lock_guard lock(get_mutex());
    if (log)
    {
        log->trace("get_blocks_info: Lock acquired.");
    }

    std::vector<allocator_test_utils::block_info> result;
    try {
        result = get_blocks_info_inner();
    } catch (const std::exception& e) {
        if (log)
        {
            log->error(std::string("get_blocks_info: Exception caught from inner method: ") + e.what());
        }
        result.clear();
    } catch (...) {
        if (log)
        {
            log->error("get_blocks_info: Unknown exception caught from inner method.");
        }
        result.clear();
    }
    if (log)
    {
        log->trace("get_blocks_info: Releasing lock.");
    }
    if (log)
    {
        log->debug("get_blocks_info finished. Returning " + std::to_string(result.size()) + " block(s).");
    }
    return result;
}

std::vector<allocator_test_utils::block_info> allocator_boundary_tags::get_blocks_info_inner() const
{
    logger* log = get_logger();

    if (log)
    {
        log->trace("get_blocks_info_inner started.");
    }

    std::vector<allocator_test_utils::block_info> res;

    if (!_trusted_memory) {
        if (log)
        {
            log->trace("get_blocks_info_inner finished (allocator not initialized).");
        }
        return res;
    }

    void* pool_start = nullptr;
    void* pool_end = nullptr;
    void* current_occupied_meta = nullptr;
    void* last_block_processed_end = nullptr;

    try {
        pool_start = get_pool_start(_trusted_memory);
        pool_end = get_pool_end(_trusted_memory);
        current_occupied_meta = get_first_occupied_block_ptr_ref(_trusted_memory);
        last_block_processed_end = pool_start;

        if (log)
        {
            log->trace("[INFO_INNER] Iterating blocks. Pool: [" + pointer_to_string_local(pool_start) + ", " + pointer_to_string_local(pool_end) + "). First occupied: " + pointer_to_string_local(current_occupied_meta));
        }

        while (true) {
            void* free_block_start = last_block_processed_end;
            void* free_block_end = (current_occupied_meta == nullptr) ? pool_end : current_occupied_meta;

            if (reinterpret_cast<std::byte*>(free_block_start) < reinterpret_cast<std::byte*>(free_block_end)) {
                size_t free_size = reinterpret_cast<std::byte*>(free_block_end) - reinterpret_cast<std::byte*>(free_block_start);
                if (log)
                {
                    log->trace("[INFO_INNER] Found Free Block. Start: " + pointer_to_string_local(free_block_start) + ", Size: " + std::to_string(free_size));
                }
                res.push_back({ .block_size = free_size, .is_block_occupied = false });
            }
            if (current_occupied_meta == nullptr) {
                 if (log)
                 {
                     log->trace("[INFO_INNER] Reached end of occupied block list.");
                 }
                 break;
            }
            size_t user_size = get_block_user_size_ref(current_occupied_meta);
            size_t total_block_size = user_size + occupied_block_metadata_size;

            if (log)
            {
                log->trace("[INFO_INNER] Found Occupied Block at " + pointer_to_string_local(current_occupied_meta) +
                        ", user_size=" + std::to_string(user_size) +
                        ", total_size=" + std::to_string(total_block_size));
            }

            res.push_back({ .block_size = total_block_size, .is_block_occupied = true });
            last_block_processed_end = reinterpret_cast<std::byte*>(current_occupied_meta) + total_block_size;
            current_occupied_meta = get_block_next_ptr_ref(current_occupied_meta); // Move to next occupied block
        }

    } catch (const std::exception& e) {
        if (log)
        {
            log->error("[INFO_INNER] Exception during block info gathering: " + std::string(e.what()));
        }
        res.clear();
    } catch (...) {
        if (log)
        {
            log->error("[INFO_INNER] Unknown exception during block info gathering.");
        }
        res.clear();
    }

    if (log)
    {
        log->trace("get_blocks_info_inner finished. Reporting " + std::to_string(res.size()) + " block(s).");
    }
    return res;
}

allocator_boundary_tags::allocator_boundary_tags(const allocator_boundary_tags &other):
    allocator_with_fit_mode(),
    _trusted_memory(nullptr)
{
    logger* other_log = other.get_logger();
    if (other_log) other_log->debug("Copy constructor started from other allocator at " + pointer_to_string_local(&other));

    if (!other._trusted_memory) {
        if (other_log) other_log->warning("Copy constructor: Source allocator is not initialized (moved from?). Creating an empty allocator.");
        return;
    }

    void* new_allocated_memory = nullptr;
    size_t total_size_needed = 0;
    std::pmr::memory_resource* alloc_to_use = nullptr;

    try {
        if (other_log) other_log->trace("Copy constructor: Reading metadata from source.");
        logger* source_logger = get_logger_ptr_ref(other._trusted_memory);
        std::pmr::memory_resource* source_parent = get_parent_allocator_ptr_ref(other._trusted_memory);
        allocator_with_fit_mode::fit_mode source_fit_mode = get_static_fit_mode_ref(other._trusted_memory);
        size_t source_user_space_size = get_user_space_size_ref(other._trusted_memory);
        total_size_needed = source_user_space_size + allocator_metadata_size;
        alloc_to_use = source_parent ? source_parent : std::pmr::get_default_resource();

        if (other_log) {
            other_log->trace("Copy constructor: Source config - Size=" + std::to_string(source_user_space_size) +
                             ", Mode=" + std::to_string(static_cast<int>(source_fit_mode)) +
                             ", Logger=" + pointer_to_string_local(source_logger) +
                             ", Parent=" + pointer_to_string_local(alloc_to_use));
        }

        if (other_log) other_log->trace("Copy constructor: Allocating new memory block of size " + std::to_string(total_size_needed));
        new_allocated_memory = alloc_to_use->allocate(total_size_needed, alignof(std::max_align_t));
        if (!new_allocated_memory) {
            throw std::bad_alloc();
        }
         _trusted_memory = new_allocated_memory;
         if (other_log) other_log->trace("Copy constructor: New memory allocated at " + pointer_to_string_local(_trusted_memory));


        if (other_log) other_log->trace("Copy constructor: Writing metadata to new block.");
        get_logger_ptr_ref(_trusted_memory) = source_logger;
        get_parent_allocator_ptr_ref(_trusted_memory) = alloc_to_use;
        get_static_fit_mode_ref(_trusted_memory) = source_fit_mode;
        get_user_space_size_ref(_trusted_memory) = source_user_space_size;
        get_first_occupied_block_ptr_ref(_trusted_memory) = nullptr;

        trace_with_guard("Copy constructor: Metadata initialized.");

        trace_with_guard("Copy constructor: Starting block copy loop.");
        void* current_old_meta = get_first_occupied_block_ptr_ref(other._trusted_memory);
        void* last_new_meta = _trusted_memory;
        void* pool_start_other = get_pool_start(other._trusted_memory);
        void* pool_start_this = get_pool_start(this->_trusted_memory);

        while (current_old_meta != nullptr) {
             trace_with_guard("Copy constructor: Processing source block at " + pointer_to_string_local(current_old_meta));

            std::ptrdiff_t offset = reinterpret_cast<std::byte*>(current_old_meta) - reinterpret_cast<std::byte*>(pool_start_other);
            void* current_new_meta = reinterpret_cast<std::byte*>(pool_start_this) + offset;

            size_t old_user_size = get_block_user_size_ref(current_old_meta);
            trace_with_guard("Copy constructor: Block user size = " + std::to_string(old_user_size));

            trace_with_guard("Copy constructor: Writing metadata for new block at " + pointer_to_string_local(current_new_meta));
            get_block_user_size_ref(current_new_meta) = old_user_size;
            get_block_prev_ptr_ref(current_new_meta) = last_new_meta;
            get_block_next_ptr_ref(current_new_meta) = nullptr;
            get_block_allocator_ptr_ref(current_new_meta) = this->_trusted_memory;

            if (last_new_meta == _trusted_memory) {
                 trace_with_guard("Copy constructor: Linking first new block.");
                get_first_occupied_block_ptr_ref(_trusted_memory) = current_new_meta;
            } else {
                 trace_with_guard("Copy constructor: Linking subsequent new block.");
                get_block_next_ptr_ref(last_new_meta) = current_new_meta;
            }

            void* src_user_ptr = get_user_ptr_from_meta(current_old_meta);
            void* dest_user_ptr = get_user_ptr_from_meta(current_new_meta);
            trace_with_guard("Copy constructor: Copying " + std::to_string(old_user_size) + " user bytes from " + pointer_to_string_local(src_user_ptr) + " to " + pointer_to_string_local(dest_user_ptr));
            if (old_user_size > 0) {
                std::memcpy(dest_user_ptr, src_user_ptr, old_user_size);
            }

            last_new_meta = current_new_meta;
            current_old_meta = get_block_next_ptr_ref(current_old_meta);
        }
        trace_with_guard("Copy constructor: Block copy loop finished.");

        information_with_guard("Available memory after copy: " + std::to_string(get_free_size()) + " bytes.");
        debug_with_guard("Memory state after copy: " + print_blocks());
        debug_with_guard("Copy constructor finished successfully.");

    } catch (const std::bad_alloc& e) {
        logger* log_to_use = _trusted_memory ? get_logger() : other_log;
        if (log_to_use) log_to_use->error("Copy constructor failed: Memory allocation error - " + std::string(e.what()));

        if (new_allocated_memory) {
             if (log_to_use) log_to_use->trace("Copy constructor: Deallocating partially allocated memory due to exception.");
             try {
                 alloc_to_use->deallocate(new_allocated_memory, total_size_needed, alignof(std::max_align_t));
             } catch (...) {
                 if (log_to_use)
                 {
                     log_to_use->critical("Copy constructor: CRITICAL - Failed to deallocate memory during exception handling!");
                 }
             }
        }
         _trusted_memory = nullptr;
        throw;
    } catch (const std::exception& e) {
        logger* log_to_use = _trusted_memory ? get_logger() : other_log;
        if (log_to_use) log_to_use->error("Copy constructor failed: Exception caught - " + std::string(e.what()));
        if (new_allocated_memory) {
             if (log_to_use) log_to_use->trace("Copy constructor: Deallocating partially allocated memory due to exception.");
            try {
                 alloc_to_use->deallocate(new_allocated_memory, total_size_needed, alignof(std::max_align_t));
             } catch(...) {
                 if (log_to_use) log_to_use->critical("Copy constructor: CRITICAL - Failed to deallocate memory during exception handling!");
             }
        }
         _trusted_memory = nullptr;
         throw std::runtime_error(std::string("Allocator copy construction failed: ") + e.what());
    } catch (...) {
        logger* log_to_use = _trusted_memory ? get_logger() : other_log;
        if (log_to_use) log_to_use->error("Copy constructor failed: Unknown exception caught.");
         if (new_allocated_memory) {
              if (log_to_use) log_to_use->trace("Copy constructor: Deallocating partially allocated memory due to exception.");
              try {
                  alloc_to_use->deallocate(new_allocated_memory, total_size_needed, alignof(std::max_align_t));
              } catch(...) {
                  if (log_to_use)
                  {
                      log_to_use->critical("Copy constructor: CRITICAL - Failed to deallocate memory during exception handling!");
                  }
              }
         }
         _trusted_memory = nullptr;
        throw std::runtime_error("Allocator copy construction failed due to unknown exception.");
    }
}

allocator_boundary_tags &allocator_boundary_tags::operator=(const allocator_boundary_tags &other)
{
    if (this == &other)
    {
        return *this;
    }
    debug_with_guard("Copy assignment operator called (not supported).");
    throw not_implemented("allocator_boundary_tags &allocator_boundary_tags::operator=(const)", "Copying not supported");
}

bool allocator_boundary_tags::do_is_equal(const std::pmr::memory_resource &other) const noexcept
{
    logger* log = get_logger();
    if (log)
    {
        log->trace("do_is_equal started.");
    }
    const auto* other_cast = dynamic_cast<const allocator_boundary_tags*>(&other);
    bool result = (other_cast != nullptr && this == other_cast);
    if (log)
    {
        log->trace("do_is_equal finished. Result: " + std::string(result ? "true" : "false"));
    }
    return result;
}

allocator_boundary_tags::boundary_iterator::boundary_iterator()
: _trusted_memory(nullptr), _occupied_ptr(nullptr), _occupied(false)
{
}

allocator_boundary_tags::boundary_iterator::boundary_iterator(void* trusted)
: _trusted_memory(trusted), _occupied_ptr(trusted), _occupied(true)
{
    if (!_trusted_memory) {
        _occupied_ptr = nullptr;
        _occupied = false;
        return;
    }

    void* first_occupied = nullptr;
    void* pool_start = nullptr;
    try {
        first_occupied = allocator_boundary_tags::get_first_occupied_block_ptr_ref(trusted);
        pool_start = allocator_boundary_tags::get_pool_start(trusted);
        if (first_occupied == pool_start) {
            _occupied = true;
            _occupied_ptr = first_occupied;
        } else {
            _occupied = false;
            _occupied_ptr = trusted;
        }
    } catch (...) {
         _occupied_ptr = nullptr;
         _occupied = false;
    }
}


bool allocator_boundary_tags::boundary_iterator::operator==(
    const allocator_boundary_tags::boundary_iterator &other) const noexcept
{
    bool result = (_occupied_ptr == other._occupied_ptr) &&
                  (_occupied == other._occupied || _occupied_ptr == nullptr);
    return result;
}

bool allocator_boundary_tags::boundary_iterator::operator!=(
    const allocator_boundary_tags::boundary_iterator & other) const noexcept
{
    return !(*this == other);
}


allocator_boundary_tags::boundary_iterator& allocator_boundary_tags::boundary_iterator::operator++() & noexcept
{
    if (_occupied_ptr == nullptr) {
        return *this;
    }

    try {
        if (!_occupied) {

            _occupied = true;
            if (_occupied_ptr == _trusted_memory) {
                _occupied_ptr = allocator_boundary_tags::get_first_occupied_block_ptr_ref(_trusted_memory);
            } else {
                _occupied_ptr = allocator_boundary_tags::get_block_next_ptr_ref(_occupied_ptr);
            }
            if (_occupied_ptr == nullptr) {
                 _occupied = false;
            } else {
            }

        } else {
            _occupied = false;
            void* next_occupied_meta = allocator_boundary_tags::get_block_next_ptr_ref(_occupied_ptr);
            if (next_occupied_meta == nullptr) {
                size_t current_user_size = allocator_boundary_tags::get_block_user_size_ref(_occupied_ptr);
                void* end_of_current_occupied = reinterpret_cast<std::byte*>(_occupied_ptr) + allocator_boundary_tags::occupied_block_metadata_size + current_user_size;
                void* pool_end_ptr = allocator_boundary_tags::get_pool_end(_trusted_memory);
                if (reinterpret_cast<std::byte*>(end_of_current_occupied) >= reinterpret_cast<std::byte*>(pool_end_ptr)) {
                    _occupied_ptr = nullptr;
                    _occupied = false;
                } else {
                }
            } else {
            }
        }
    } catch(...) {
         _occupied_ptr = nullptr;
         _occupied = false;
    }
    return *this;
}

allocator_boundary_tags::boundary_iterator& allocator_boundary_tags::boundary_iterator::operator--() & noexcept
{
    if (_occupied_ptr == nullptr && !_occupied) {
         return *this;
    }

    try {
        if (!_occupied) {
            if (_occupied_ptr == _trusted_memory) {
                 _occupied_ptr = nullptr;
                 _occupied = false;
            } else {
                _occupied = true;
            }
        } else{
            void* prev_meta = allocator_boundary_tags::get_block_prev_ptr_ref(_occupied_ptr);
            _occupied = false;
            _occupied_ptr = prev_meta;
        }
    } catch (...) {
         _occupied_ptr = nullptr;
         _occupied = false;
    }
    return *this;
}


allocator_boundary_tags::boundary_iterator allocator_boundary_tags::boundary_iterator::operator++(int)
{
    auto tmp = *this;
    ++(*this);
    return tmp;
}

allocator_boundary_tags::boundary_iterator allocator_boundary_tags::boundary_iterator::operator--(int)
{
    auto tmp = *this;
    --(*this);
    return tmp;
}

size_t allocator_boundary_tags::boundary_iterator::size() const noexcept
{
    if (_occupied_ptr == nullptr)
    {
        return 0;
    }
    if (_occupied) {
        return allocator_boundary_tags::get_block_user_size_ref(_occupied_ptr) + allocator_boundary_tags::occupied_block_metadata_size;
    } else {
        return allocator_boundary_tags::get_next_free_size(_occupied_ptr, _trusted_memory);
    }

}

bool allocator_boundary_tags::boundary_iterator::occupied() const noexcept
{
    return _occupied_ptr != nullptr && _occupied;
}

void* allocator_boundary_tags::boundary_iterator::operator*() const noexcept
{
    if (_occupied_ptr == nullptr) return nullptr;

    try {
        if (_occupied) {
            return allocator_boundary_tags::get_user_ptr_from_meta(_occupied_ptr);
        } else {
            if (_occupied_ptr == _trusted_memory) {
                return allocator_boundary_tags::get_pool_start(_trusted_memory);
            } else {
                size_t prev_user_size = allocator_boundary_tags::get_block_user_size_ref(_occupied_ptr);
                return reinterpret_cast<std::byte*>(_occupied_ptr) + allocator_boundary_tags::occupied_block_metadata_size + prev_user_size;
            }
        }
    } catch(...)
    {
        return nullptr;
    }
}

void* allocator_boundary_tags::boundary_iterator::get_ptr() const noexcept
{
    return _occupied_ptr;
}
allocator_boundary_tags::boundary_iterator allocator_boundary_tags::begin() const noexcept
{
    logger* log = get_logger();
    if (log)
    {
        log->trace("begin() called.");
    }
    return boundary_iterator(_trusted_memory);
}

allocator_boundary_tags::boundary_iterator allocator_boundary_tags::end() const noexcept
{
    logger* log = get_logger();
    if (log)
    {
        log->trace("end() called.");
    }
    return boundary_iterator();
}