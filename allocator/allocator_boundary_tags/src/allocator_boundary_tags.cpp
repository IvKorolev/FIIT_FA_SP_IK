#include <not_implemented.h>
#include "../include/allocator_boundary_tags.h"

using byte = unsigned char;

std::mutex &allocator_boundary_tags::get_mutex() const noexcept
{
    auto byte_ptr = reinterpret_cast<std::byte*>(_trusted_memory);
    return *reinterpret_cast<std::mutex*>(byte_ptr + sizeof(logger*) + sizeof(std::pmr::memory_resource*) + sizeof(fit_mode) + sizeof(unsigned char));
}

allocator_boundary_tags::~allocator_boundary_tags()
{
    if (_trusted_memory) {
        auto byte_ptr = reinterpret_cast<std::byte*>(_trusted_memory);
        auto mutex_ptr = reinterpret_cast<std::mutex*>(byte_ptr + sizeof(logger*) + sizeof(memory_resource*) + sizeof(allocator_with_fit_mode::fit_mode) + sizeof(size_t));
        mutex_ptr->~mutex();
        ::operator delete(_trusted_memory);
    }
    _trusted_memory = nullptr;
    debug_with_guard("allocator_boundary_tags::destructor");
}

allocator_boundary_tags::allocator_boundary_tags(
    allocator_boundary_tags &&other) noexcept : _trusted_memory(other._trusted_memory)
{
    other._trusted_memory = nullptr;
    debug_with_guard("allocator_boundary_tags::move constructor");
}

allocator_boundary_tags &allocator_boundary_tags::operator=(
    allocator_boundary_tags &&other) noexcept
{
    if (this != &other) {
        this->_trusted_memory = other._trusted_memory;
        other._trusted_memory = nullptr;
    }
    debug_with_guard("allocator_boundary_tags::operator=");
    return *this;
}

/** If parent_allocator* == nullptr you should use std::pmr::get_default_resource()
 */
allocator_boundary_tags::allocator_boundary_tags(
        size_t space_size,
        std::pmr::memory_resource *parent_allocator,
        logger *logger,
        allocator_with_fit_mode::fit_mode allocate_fit_mode)
{
    throw not_implemented("allocator_boundary_tags::allocator_boundary_tags(size_t,std::pmr::memory_resource *,logger *,allocator_with_fit_mode::fit_mode)", "your code should be here...");
}

[[nodiscard]] void *allocator_boundary_tags::do_allocate_sm(
    size_t size)
{
    throw not_implemented("[[nodiscard]] void *allocator_boundary_tags::do_allocate_sm(size_t)", "your code should be here...");
}

void allocator_boundary_tags::do_deallocate_sm(
    void *at)
{
    throw not_implemented("void allocator_boundary_tags::do_deallocate_sm(void *)", "your code should be here...");
}

inline void allocator_boundary_tags::set_fit_mode(
    allocator_with_fit_mode::fit_mode mode)
{
    trace_with_guard("allocator_buddies_system::set_fit_mode");
    auto byte_ptr = reinterpret_cast<std::byte*>(_trusted_memory);
    std::lock_guard lock(get_mutex());

    auto fit_mode_ptr = reinterpret_cast<allocator_with_fit_mode::fit_mode*>(byte_ptr + sizeof(logger*) + sizeof(memory_resource*));
    *fit_mode_ptr = mode;
}


std::vector<allocator_test_utils::block_info> allocator_boundary_tags::get_blocks_info() const
{
    std::vector<allocator_test_utils::block_info> blocks_info;
    if (!_trusted_memory) return blocks_info;

    auto byte_ptr = reinterpret_cast<std::byte*>(_trusted_memory);
    std::cout << "Begin: " << *begin() << ", End: " << *end() << std::endl;

    for (auto it = begin(); it != end(); ++it)
    {
        if (*it == nullptr) break; // Проверка на корректность указателя
        blocks_info.push_back({it.size(), it.occupied()});
    }

    return blocks_info;
}

inline logger *allocator_boundary_tags::get_logger() const
{
    if (_trusted_memory == nullptr)
    {
        return nullptr;
    }
    return *reinterpret_cast<logger**>(_trusted_memory);
}

inline std::string allocator_boundary_tags::get_typename() const noexcept
{
    return "allocator_boundary_tags";
}


allocator_boundary_tags::boundary_iterator allocator_boundary_tags::begin() const noexcept
{
    void* start_block = *reinterpret_cast<void **>(reinterpret_cast<byte*>(_trusted_memory) + allocator_metadata_size);
    return boundary_iterator(start_block);
}

allocator_boundary_tags::boundary_iterator allocator_boundary_tags::end() const noexcept
{
}

std::vector<allocator_test_utils::block_info> allocator_boundary_tags::get_blocks_info_inner() const
{
    std::vector<allocator_test_utils::block_info> blocks_info;

    // Проверка, есть ли выделенная память
    if (!_trusted_memory) {
        return blocks_info;
    }

    // Инициализация итераторов для начала и конца области памяти
    for (auto it = begin(); it != end(); ++it) {
        // Собираем информацию о каждом блоке
        allocator_test_utils::block_info info;
        info.block_size = it.size();    // Размер текущего блока
        info.is_block_occupied = it.occupied(); // Статус блока: занят или нет

        // Добавляем информацию о блоке в вектор
        blocks_info.push_back(info);
    }

    return blocks_info;
}

allocator_boundary_tags::allocator_boundary_tags(const allocator_boundary_tags &other)
{
    throw not_implemented("allocator_boundary_tags::allocator_boundary_tags(const allocator_boundary_tags &other)", "your code should be here...");
}

allocator_boundary_tags &allocator_boundary_tags::operator=(const allocator_boundary_tags &other)
{
    throw not_implemented("allocator_boundary_tags &allocator_boundary_tags::operator=(const allocator_boundary_tags &other)", "your code should be here...");
}

bool allocator_boundary_tags::do_is_equal(const std::pmr::memory_resource &other) const noexcept
{
    throw not_implemented("bool allocator_boundary_tags::do_is_equal(const std::pmr::memory_resource &other) const noexcept", "your code should be here...");
}

bool allocator_boundary_tags::boundary_iterator::operator==(
        const allocator_boundary_tags::boundary_iterator &other) const noexcept
{
    return _occupied_ptr == other._occupied_ptr;
}

bool allocator_boundary_tags::boundary_iterator::operator!=(
        const allocator_boundary_tags::boundary_iterator & other) const noexcept
{
    return _occupied_ptr != other._occupied_ptr;
}

allocator_boundary_tags::boundary_iterator &allocator_boundary_tags::boundary_iterator::operator++() & noexcept
{
    throw not_implemented("allocator_boundary_tags::boundary_iterator &allocator_boundary_tags::boundary_iterator::operator++() & noexcept", "your code should be here...");
}

allocator_boundary_tags::boundary_iterator &allocator_boundary_tags::boundary_iterator::operator--() & noexcept
{
    throw not_implemented("allocator_boundary_tags::boundary_iterator &allocator_boundary_tags::boundary_iterator::operator--() & noexcept", "your code should be here...");
}

allocator_boundary_tags::boundary_iterator allocator_boundary_tags::boundary_iterator::operator++(int n)
{
    auto temporary = *this;
    ++(*this);
    return temporary;
}

allocator_boundary_tags::boundary_iterator allocator_boundary_tags::boundary_iterator::operator--(int n)
{
    auto temporary = *this;
    --(*this);
    return temporary;
}

size_t allocator_boundary_tags::boundary_iterator::size() const noexcept
{
    throw not_implemented("size_t allocator_boundary_tags::boundary_iterator::size() const noexcept", "your code should be here...");
}

bool allocator_boundary_tags::boundary_iterator::occupied() const noexcept
{
    return _occupied;
}

void* allocator_boundary_tags::boundary_iterator::operator*() const noexcept
{
    return get_ptr();
}

allocator_boundary_tags::boundary_iterator::boundary_iterator() : _occupied_ptr(nullptr), _occupied(false), _trusted_memory(nullptr)
{

}

allocator_boundary_tags::boundary_iterator::boundary_iterator(void *trusted) : _occupied_ptr(trusted), _occupied(true), _trusted_memory(trusted)
{

}

void *allocator_boundary_tags::boundary_iterator::get_ptr() const noexcept
{
    return _occupied_ptr;
}
