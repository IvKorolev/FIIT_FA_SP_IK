#include <not_implemented.h>
#include <cstddef>
#include "../include/allocator_buddies_system.h"
#include <sstream>

using byte = unsigned char;

allocator_buddies_system::~allocator_buddies_system()
{
    if (_trusted_memory) {
        auto byte_ptr = reinterpret_cast<std::byte*>(_trusted_memory);
        auto mutex_ptr = reinterpret_cast<std::mutex*>(byte_ptr + sizeof(logger*) + sizeof(allocator_dbg_helper*) + sizeof(fit_mode) + sizeof(unsigned char));
        mutex_ptr->~mutex();
        ::operator delete(_trusted_memory);
    }
    _trusted_memory = nullptr;
    debug_with_guard("allocator_buddies_system::destructor");
}

allocator_buddies_system::allocator_buddies_system(
    allocator_buddies_system &&other) noexcept : _trusted_memory(other._trusted_memory)
{
    other._trusted_memory = nullptr;
    debug_with_guard("allocator_buddies_system::move constructor");
}

allocator_buddies_system &allocator_buddies_system::operator=(
    allocator_buddies_system &&other) noexcept
{
    if (this != &other) {
        this->_trusted_memory = other._trusted_memory;
        other._trusted_memory = nullptr;
    }
    debug_with_guard("allocator_buddies_system::operator=");
    return *this;
}

allocator_buddies_system::allocator_buddies_system(
        size_t space_size,
        std::pmr::memory_resource *parent_allocator,
        logger *logger,
        allocator_with_fit_mode::fit_mode allocate_fit_mode)
{
    if (space_size < min_k) {
        throw std::logic_error("size must be greater than meta_data of an occupied block");
    }

    size_t real_size = pow_2(space_size) + allocator_metadata_size;
    if (parent_allocator == nullptr)
    {
        try
        {
            _trusted_memory = ::operator new(real_size);
        } catch (std::bad_alloc& ex)
        {
            error_with_guard("Bad allocation memory");
            throw;
        }
    } else
    {
        try
        {
            _trusted_memory = parent_allocator->allocate(real_size, 1);
        }
        catch (std::bad_alloc& ex)
        {
            error_with_guard("Bad allocation memory from parent allocator");
            throw;
        }
    }

    fill_allocator_fields(space_size, parent_allocator, logger, allocate_fit_mode);
    debug_with_guard("Built allocator: " + get_typename());
}

void allocator_buddies_system::fill_allocator_fields(size_t space_size,
        std::pmr::memory_resource *parent_allocator,
        logger *logger,
        allocator_with_fit_mode::fit_mode allocate_fit_mode)
{
    void* memory =_trusted_memory;
    *reinterpret_cast<class logger**>(memory) = logger;
    memory = slide(memory, sizeof(class logger*));

    *reinterpret_cast<std::pmr::memory_resource**>(memory) = parent_allocator;
    memory = slide(memory, sizeof(std::pmr::memory_resource*));

    *reinterpret_cast<allocator_with_fit_mode::fit_mode*>(memory) = allocate_fit_mode;
    memory = slide(memory, sizeof(allocator_with_fit_mode::fit_mode));

    *reinterpret_cast<byte*>(memory) = space_size;
    memory = slide(memory, sizeof(byte));

    auto mut = reinterpret_cast<std::mutex*>(memory);
    new (mut) :: std::mutex();
    memory = slide(memory, sizeof(std::mutex));

    block_metadata* first_block = reinterpret_cast<block_metadata*>(memory);

    (*first_block).occupied = false;
    first_block->size = space_size;  // Записываем показатель степени, а не размер
    std::cout << "Initial block size: " << (1 << first_block->size) << " bytes\n";
}

std::string allocator_buddies_system::get_info_in_string(const std::vector<allocator_test_utils::block_info>& vec) noexcept
{
    std::ostringstream str;
    for (auto& it : vec)
    {
        if (it.is_block_occupied)
        {
            str << "<occup>";
        } else
            str << "<avail>";

        str << " <" + std::to_string(it.block_size) + "> | ";
    }
    return str.str();
}

std::mutex &allocator_buddies_system::get_mutex() const noexcept
{
    auto byte_ptr = reinterpret_cast<std::byte*>(_trusted_memory);

    return *reinterpret_cast<std::mutex*>(byte_ptr + sizeof(logger*) + sizeof(std::pmr::memory_resource*) + sizeof(fit_mode) + sizeof(unsigned char));
}

[[nodiscard]] void *allocator_buddies_system::do_allocate_sm(
    size_t size)
{
    std::lock_guard lock(get_mutex());

    size_t free_space;
    size_t real_size = size + occupied_block_metadata_size;

    information_with_guard("Allocator before allocated: current condition of blocks: " + get_info_in_string(get_blocks_info()));
    information_with_guard("Free memory to allocate: " + std::to_string(free_space));
    trace_with_guard(get_typename() + " allocating " + std::to_string(real_size) + " bytes ");

    void* free_block;

    switch (get_fit_mod())
    {
        case allocator_with_fit_mode::fit_mode::first_fit:
            free_block = get_first(real_size);
        break;
        case allocator_with_fit_mode::fit_mode::the_best_fit:
            free_block = get_best(real_size);
        break;
        case allocator_with_fit_mode::fit_mode::the_worst_fit:
            free_block = get_worst(real_size);
        break;
        default:
            throw std::logic_error("Not existing case");
    }

    if (free_block == nullptr)
    {
        error_with_guard("Bad allocation by " + get_typename() + " of " + std::to_string(real_size) + " bytes");
        throw std::bad_alloc();
    }

    while (get_size_block(free_block) >= (real_size << 1))
    {
        auto brother = reinterpret_cast<block_metadata*>(free_block);
        --(brother->size);

        auto sister = reinterpret_cast<block_metadata*>(get_twin(free_block));
        sister->occupied = false;
        sister->size = brother->size;
    }

    if (get_size_block(free_block) != real_size)
    {
        warning_with_guard(get_typename() + "changed allocating block size to " +
                           std::to_string(get_size_block(free_block)));
    }

    auto first_twin = reinterpret_cast<block_metadata*>(free_block);
    first_twin->occupied = true;

    trace_with_guard(get_typename() + " allocated " + std::to_string(real_size) + " bytes ");

    information_with_guard("Condition of blocks after allocation: " + get_info_in_string(get_blocks_info()));
    information_with_guard("Available free space: " + std::to_string(free_space));

    return slide(free_block, occupied_block_metadata_size);
}


void allocator_buddies_system::do_deallocate_sm(void *at)
{
    std::lock_guard lock(get_mutex());
    trace_with_guard("Started deallocate");

    size_t size_avaliable;
    information_with_guard("Blocks condition before deallocate: \n" +
            get_info_in_string(get_blocks_info()));

    information_with_guard("Available free space: " + std::to_string(size_avaliable));
    void* current_block = reinterpret_cast<std::byte*>(at) - occupied_block_metadata_size;
    // if (*reinterpret_cast<void**>(reinterpret_cast<std::byte*>(at) - sizeof(void*)) != _trusted_memory)
    // {
    //     error_with_guard("Tried to deallocate not allocator's property");
    //     throw std::logic_error("Tried to deallocate not allocator's property");
    // }
    size_t current_block_size = get_size_block(current_block) - occupied_block_metadata_size;

    debug_with_guard("condition of block before deallocate: " + get_dump(reinterpret_cast<char*>(at), current_block_size));

    reinterpret_cast<block_metadata*>(current_block)->occupied = false;

    void* twin = get_twin(current_block);

    // merge twins until meet using block
    while (get_size_block(current_block) < get_size_full() &&
           get_size_block(current_block) == get_size_block(twin) && !is_occupied(twin))
    {
        void* left_twin = current_block < twin ? current_block : twin;

        auto current_meta = reinterpret_cast<block_metadata*>(left_twin);
        ++current_meta->size;

        current_block = left_twin;
        twin = get_twin(current_block);
    }

    trace_with_guard("Ended deallocate");

    information_with_guard("Blocks condition after deallocate: \n" + get_info_in_string(get_blocks_info()));

    information_with_guard("Available free space: " + std::to_string(size_avaliable));
}

allocator_buddies_system::allocator_buddies_system(const allocator_buddies_system &other)
{
    if (other._trusted_memory) {
        size_t real_size = __detail::nearest_greater_k_of_2(
            reinterpret_cast<const unsigned char*>(other._trusted_memory)[sizeof(logger*) + sizeof(allocator_dbg_helper*) + sizeof(fit_mode)]);
        _trusted_memory = ::operator new(real_size);
        std::memcpy(_trusted_memory, other._trusted_memory, real_size);
    } else {
        _trusted_memory = nullptr;
    }
    debug_with_guard("allocator_buddies_system::copy constructor");
}

allocator_buddies_system &allocator_buddies_system::operator=(const allocator_buddies_system &other)
{
    if (this != &other) {
        // Освобождаем текущую память
        if (_trusted_memory) {
            auto byte_ptr = reinterpret_cast<std::byte*>(_trusted_memory);
            auto mutex_ptr = reinterpret_cast<std::mutex*>(byte_ptr + sizeof(logger*) + sizeof(allocator_dbg_helper*) + sizeof(fit_mode) + sizeof(unsigned char));
            mutex_ptr->~mutex();
            ::operator delete(_trusted_memory);
        }

        _trusted_memory = nullptr;

        // Копируем данные из другого объекта
        if (other._trusted_memory) {
            size_t real_size = __detail::nearest_greater_k_of_2(
                reinterpret_cast<const unsigned char*>(other._trusted_memory)[sizeof(logger*) + sizeof(allocator_dbg_helper*) + sizeof(fit_mode)]);
            _trusted_memory = ::operator new(real_size);
            std::memcpy(_trusted_memory, other._trusted_memory, real_size);

            // Инициализируем мьютекс
            auto byte_ptr = reinterpret_cast<std::byte*>(_trusted_memory);
            auto mutex_ptr = reinterpret_cast<std::mutex*>(byte_ptr + sizeof(logger*) + sizeof(allocator_dbg_helper*) + sizeof(fit_mode) + sizeof(unsigned char));
            new (mutex_ptr) ::std::mutex();
        }
    }

    debug_with_guard("allocator_buddies_system::copy assignment operator");
    return *this;
}

bool allocator_buddies_system::do_is_equal(const std::pmr::memory_resource &other) const noexcept
{
    if (this == &other)
    {
        return true;
    }
    if (typeid(other) != typeid(allocator_buddies_system))
    {
        return false;
    }
    auto other_allocator = static_cast<const allocator_buddies_system*>(&other);
    return _trusted_memory == other_allocator->_trusted_memory;
}

inline void allocator_buddies_system::set_fit_mode(
    allocator_with_fit_mode::fit_mode mode)
{
    trace_with_guard("allocator_buddies_system::set_fit_mode");
    auto byte_ptr = reinterpret_cast<std::byte*>(_trusted_memory);
    // auto mutex_ptr = reinterpret_cast<std::mutex*>(byte_ptr + sizeof(logger*) + sizeof(allocator_dbg_helper*) + sizeof(fit_mode) + sizeof(unsigned char));
    // std::lock_guard lock(*mutex_ptr);
    std::lock_guard lock(get_mutex());

    auto fit_mode_ptr = reinterpret_cast<allocator_with_fit_mode::fit_mode*>(byte_ptr + sizeof(logger*) + sizeof(allocator_dbg_helper*));
    *fit_mode_ptr = mode;
}


std::vector<allocator_test_utils::block_info> allocator_buddies_system::get_blocks_info() const noexcept
{
    std::vector<allocator_test_utils::block_info> blocks_info;
    if (!_trusted_memory) return blocks_info;

    auto byte_ptr = reinterpret_cast<std::byte*>(_trusted_memory);
    // auto mutex_ptr = reinterpret_cast<std::mutex*>(byte_ptr + sizeof(logger*) + sizeof(allocator_dbg_helper*) + sizeof(fit_mode) + sizeof(unsigned char));
    // std::lock_guard lock(*mutex_ptr);
    //std::lock_guard lock(get_mutex());
    std::cout << "Begin: " << *begin() << ", End: " << *end() << std::endl;

    for (auto it = begin(); it != end(); ++it)
    {
        if (*it == nullptr) break; // Проверка на корректность указателя
        blocks_info.push_back({it.size(), it.occupied()});
    }

    return blocks_info;
}

inline logger *allocator_buddies_system::get_logger() const
{
    if (_trusted_memory == nullptr)
    {
        return nullptr;
    }
    return *reinterpret_cast<logger**>(_trusted_memory);
}

inline std::string allocator_buddies_system::get_typename() const
{
    return "allocator_buddies_system";
}

allocator_with_fit_mode::fit_mode &allocator_buddies_system::get_fit_mod() const noexcept
{
    auto byte_ptr = reinterpret_cast<std::byte*>(_trusted_memory);

    return *reinterpret_cast<fit_mode*>(byte_ptr + sizeof(logger*) + sizeof(allocator_dbg_helper*));
}

void *allocator_buddies_system::get_first(size_t size) const noexcept
{
    for(auto it = begin(), sent = end(); it != sent; ++it)
    {
        if (*it == nullptr) {
            // Проверка на корректность указателя
            continue;
        }

        if (!it.occupied() && it.size() >= size)
        {
            return *it;
        }
    }

    return nullptr;
}

void *allocator_buddies_system::get_best(size_t size) const noexcept
{
    buddy_iterator res;

    for(auto it = begin(), sent = end(); it != sent; ++it)
    {
        if (*it == nullptr) {
            // Проверка на корректность указателя
            continue;
        }

        if (!it.occupied() && it.size() >= size && (it.size() < res.size() || *res == nullptr))
        {
            res = it;
        }
    }

    return *res;
}

void *allocator_buddies_system::get_worst(size_t size) const noexcept
{
    buddy_iterator res;

    for(auto it = begin(), sent = end(); it != sent; ++it)
    {

        if (*it == nullptr) {
            // Проверка на корректность указателя
            continue;
        }

        if (!it.occupied() && it.size() >= size && (it.size() > res.size() || *res == nullptr))
        {
            res = it;
        }
    }

    return *res;
}

std::vector<allocator_test_utils::block_info> allocator_buddies_system::get_blocks_info_inner() const
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

size_t allocator_buddies_system::pow_2(size_t power_of_2) noexcept
{
    constexpr const size_t number = 1;
    return number << power_of_2;
}

inline size_t allocator_buddies_system::get_size_block(void* current_block) const noexcept
{
    auto metadata = reinterpret_cast<block_metadata*>(current_block);
    return static_cast<size_t>(1) << metadata->size;
}

allocator_buddies_system::buddy_iterator allocator_buddies_system::begin() const noexcept
{
    void *start_block = reinterpret_cast<void *>(reinterpret_cast<byte*>(_trusted_memory) + allocator_metadata_size);
    return buddy_iterator(start_block);
}

void* allocator_buddies_system::get_twin(void* current_block) noexcept
{
    // size_t zero_point = reinterpret_cast<byte*>(current_block) -
    //         (reinterpret_cast<byte*>(_trusted_memory) + allocator_metadata_size);
    // size_t delta = zero_point ^ (get_size_block(current_block));
    //
    // return slide(_trusted_memory, allocator_metadata_size + delta);
    size_t block_size = get_size_block(current_block);
    size_t offset = reinterpret_cast<byte*>(current_block) -
                   (reinterpret_cast<byte*>(_trusted_memory) + allocator_metadata_size);
    return reinterpret_cast<byte*>(_trusted_memory) + allocator_metadata_size + (offset ^ block_size);
}

bool allocator_buddies_system::is_occupied(void* current_block) noexcept
{
    return reinterpret_cast<block_metadata*>(current_block)->occupied;
}

allocator_buddies_system::buddy_iterator allocator_buddies_system::end() const noexcept
{
    byte* end_ptr = reinterpret_cast<byte*>(_trusted_memory) +
                    allocator_metadata_size +
                    (static_cast<size_t>(1) << *reinterpret_cast<byte*>(
                        reinterpret_cast<byte*>(_trusted_memory) +
                        sizeof(logger*) +
                        sizeof(std::pmr::memory_resource*) +
                        sizeof(fit_mode)));
    return buddy_iterator(end_ptr);
}

bool allocator_buddies_system::buddy_iterator::operator==(const allocator_buddies_system::buddy_iterator &other) const noexcept
{
    return _block == other._block;
}

bool allocator_buddies_system::buddy_iterator::operator!=(const allocator_buddies_system::buddy_iterator &other) const noexcept
{
    return !(*this == other);
}

allocator_buddies_system::buddy_iterator &allocator_buddies_system::buddy_iterator::operator++() & noexcept
{
    auto metadata = reinterpret_cast<block_metadata*>(_block);
    size_t block_size = static_cast<size_t>(1) << metadata->size;

    // Проверка на выход за пределы памяти
    if (reinterpret_cast<std::byte*>(_block) + block_size >= reinterpret_cast<std::byte*>(_block) + allocator_buddies_system::allocator_metadata_size + allocator_buddies_system::pow_2(metadata->size))
    {
        _block = nullptr; // Устанавливаем итератор в конец
    }
    else
    {
        _block = reinterpret_cast<byte*>(_block) + block_size;
    }
    return *this;
}

allocator_buddies_system::buddy_iterator allocator_buddies_system::buddy_iterator::operator++(int n)
{
    auto temporary = *this;
    ++(*this);
    return temporary;
}

inline size_t allocator_buddies_system::get_size_full() const noexcept
{
    void* ptr = slide(_trusted_memory, (sizeof(logger*) + sizeof(std::pmr::memory_resource*) + sizeof(fit_mode)));
    return pow_2(*reinterpret_cast<unsigned char*>(ptr));
}

size_t allocator_buddies_system::buddy_iterator::size() const noexcept
{
    auto metadata = reinterpret_cast<block_metadata*>(_block);
    return static_cast<size_t>(1) << metadata->size;  // 2^kч
}

bool allocator_buddies_system::buddy_iterator::occupied() const noexcept
{
    auto metadata = reinterpret_cast<block_metadata*>(_block);
    return metadata->occupied;
}

void *allocator_buddies_system::buddy_iterator::operator*() const noexcept
{
    return _block;
}

allocator_buddies_system::buddy_iterator::buddy_iterator(void *start) : _block(start)
{
}

allocator_buddies_system::buddy_iterator::buddy_iterator() : _block(nullptr)
{
}

inline void* allocator_buddies_system::slide(void* mem, size_t siz) const noexcept
{
    return reinterpret_cast<void*>(reinterpret_cast<byte*>(mem) + siz);
}