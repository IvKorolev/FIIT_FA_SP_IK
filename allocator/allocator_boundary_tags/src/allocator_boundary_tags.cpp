#include <not_implemented.h>
#include "../include/allocator_boundary_tags.h"

#include <ranges>

using byte = unsigned char;

allocator_boundary_tags::~allocator_boundary_tags()
{
    if (_trusted_memory) {
        auto byte_ptr = reinterpret_cast<std::byte*>(_trusted_memory);
        auto mutex_ptr = reinterpret_cast<std::mutex*>(byte_ptr + sizeof(logger*) + sizeof(std::pmr::memory_resource*) + sizeof(allocator_with_fit_mode::fit_mode) + sizeof(size_t));
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
    // size_t total_size = allocator_metadata_size + space_size;
    //
    // // Выделяем память для аллокатора и блоков
    // _trusted_memory = ::operator new(total_size);  // Выделяем память с учетом выравнивания
    //
    // if (!_trusted_memory) {
    //     throw std::bad_alloc(); // Если память не выделена, генерируем исключение
    // }
    //
    // // Инициализация указателей на метаданные
    // auto byte_ptr = reinterpret_cast<std::byte*>(_trusted_memory);
    //
    // // Инициализация logger
    // if (logger != nullptr) {
    //     *reinterpret_cast<class logger**>(byte_ptr) = logger;
    // } else {
    //     *reinterpret_cast<class logger**>(byte_ptr) = nullptr;
    // }
    //
    // // Инициализация memory_resource
    // auto memory_resource_ptr = reinterpret_cast<std::pmr::memory_resource**>(byte_ptr + sizeof(class logger*));
    // *memory_resource_ptr = parent_allocator ? parent_allocator : std::pmr::get_default_resource();
    //
    // // Инициализация fit_mode
    // auto fit_mode_ptr = reinterpret_cast<allocator_with_fit_mode::fit_mode*>(byte_ptr + sizeof(class logger*) + sizeof(std::pmr::memory_resource*));
    // *fit_mode_ptr = allocate_fit_mode;
    //
    // // Инициализация дополнительной информации, если необходимо
    // auto size_ptr = reinterpret_cast<size_t*>(byte_ptr + sizeof(class logger*) + sizeof(std::pmr::memory_resource*) + sizeof(allocator_with_fit_mode::fit_mode));
    // *size_ptr = space_size;
    //
    // // Инициализация mutex
    // auto mutex_ptr = reinterpret_cast<std::mutex*>(byte_ptr + sizeof(class logger*) + sizeof(std::pmr::memory_resource*) + sizeof(allocator_with_fit_mode::fit_mode) + sizeof(size_t));
    // new (mutex_ptr) std::mutex(); // Вызываем конструктор mutex
    //
    // // Указатель на первый блок памяти (начало выделенной области)
    // void* start_block = byte_ptr + allocator_metadata_size;
    // *reinterpret_cast<void**>(byte_ptr + allocator_metadata_size - sizeof(void*)) = start_block;
    //
    // debug_with_guard("allocator_boundary_tags::allocator_boundary_tags"); // Логирование, если необходимо

    size_t total_size = allocator_metadata_size + space_size;
    //std::cout << total_size << std::endl;
    void *memory;

    if (parent_allocator == nullptr) {
        try
        {
            memory = ::operator new(total_size);
        } catch (const std::bad_alloc &ex) {
            throw;
        }
    }
    else {
        memory = parent_allocator->allocate(total_size);
    }
    _trusted_memory = memory;
    unsigned char* ptr = reinterpret_cast<unsigned char*>(_trusted_memory);

    *reinterpret_cast<class logger**> (memory) = logger;
    ptr += sizeof(class logger*);

    *reinterpret_cast<std::pmr::memory_resource**>(ptr) = parent_allocator;
    ptr += sizeof(std::pmr::memory_resource*);

    *reinterpret_cast<allocator_with_fit_mode::fit_mode*>(ptr) = allocate_fit_mode;
    ptr += sizeof(allocator_with_fit_mode::fit_mode);

    *reinterpret_cast<size_t*>(ptr) = space_size;
    ptr += sizeof(size_t);

    auto mut = reinterpret_cast<std::mutex*>(ptr);
    new (mut) :: std::mutex();
    ptr += sizeof(std::mutex);

    *reinterpret_cast<void**>(ptr) = nullptr;
    ptr += sizeof(void*);
    debug_with_guard("Constructed allocator_boudary_tags");
}

[[nodiscard]] void *allocator_boundary_tags::do_allocate_sm(
    size_t size)
{
    std::lock_guard lock(get_mutex());

    size_t total_memory_to_allocate = size + occupied_block_metadata_size;
    debug_with_guard("Started allocate process of " + std::to_string(total_memory_to_allocate) + " bytes");

    debug_with_guard("Memory state : \n" + get_info_in_string());

    void** first_block = get_first_block_ptr();
    byte* ptr;

    if (*first_block == nullptr) {
        ptr = (reinterpret_cast<byte*>(_trusted_memory)) + allocator_metadata_size;
        size_t size = get_size();
        if (size < total_memory_to_allocate)
        {
            error_with_guard("Try to allocate " +
                std::to_string(total_memory_to_allocate) + "from" + std::to_string(size));
            throw std::bad_alloc();
        }

        *first_block = ptr;
        return create_block_metadata(reinterpret_cast<void*>(ptr), total_memory_to_allocate - occupied_block_metadata_size, nullptr, nullptr);
    }

    allocator_with_fit_mode::fit_mode fit_mode = get_fit_mode();

    void* memory = nullptr;
    if (fit_mode == allocator_with_fit_mode::fit_mode::first_fit) {
        debug_with_guard("Allocating first block");
        memory = allocate_first_fit(*first_block, size);
        debug_with_guard("Done allocating first block");
    }
    else if (fit_mode == allocator_with_fit_mode::fit_mode::the_best_fit) {
        debug_with_guard("Allocating best block");
        memory = allocate_best_fit(*first_block, size);
        debug_with_guard("Done allocating best block");
    }
    else if (fit_mode == allocator_with_fit_mode::fit_mode::the_worst_fit) {
        debug_with_guard("Allocating worst block");
        memory = allocate_worst_fit(*first_block, size);
        debug_with_guard("Done allocating worst block");
    }

    if (memory != nullptr) {
        debug_with_guard("Succesfully allocated memory");
        return memory;
    }
    error_with_guard("Failed to allocate memory");
    throw std::bad_alloc();
}

void* allocator_boundary_tags::allocate_first_fit(void* left_elem, size_t size) {

    void* place_to_put = nullptr;
    void* that_block = nullptr;

    size_t whole_block_size = 0;
    bool at_start = false;

    void* next_block = get_next_existing_block(left_elem);
    void* block = left_elem;

    if (slide_block_for(_trusted_memory, allocator_metadata_size) != left_elem) {
        void* pointer_after_meta = slide_block_for(_trusted_memory, allocator_metadata_size);
        if ( reinterpret_cast<byte*>(left_elem) - reinterpret_cast<byte*>(pointer_after_meta)>= occupied_block_metadata_size + size) {
            whole_block_size = reinterpret_cast<byte*>(left_elem) - reinterpret_cast<byte*>(pointer_after_meta);
            std::cout << whole_block_size << std::endl;
            that_block = left_elem;
            place_to_put = pointer_after_meta;
            at_start = true;
        }
    }

    while (next_block != nullptr && !at_start) {
        size_t size_between_blocks = get_block_distance(block, next_block);
        if (size_between_blocks >= size + occupied_block_metadata_size) {
            whole_block_size = size_between_blocks;
            that_block = block;
            place_to_put = slide_block_for(block, occupied_block_metadata_size + get_block_data_size(block));
            break;
        }
        block = next_block;
        next_block = get_next_existing_block(block);
    }
    size_t size_between_end_and_block = ((reinterpret_cast<byte*>(block) + get_block_data_size(block) + occupied_block_metadata_size) - (reinterpret_cast<byte*>(_trusted_memory) + allocator_metadata_size));

    if (!at_start && place_to_put == nullptr && get_size() - size_between_end_and_block >= size + occupied_block_metadata_size) {
        whole_block_size = get_size() - size_between_end_and_block;
        that_block = block;
        place_to_put = slide_block_for(block, occupied_block_metadata_size + get_block_data_size(block));
    }

    if (at_start) {
        *get_first_block_ptr() = place_to_put;
    }
    if (place_to_put == nullptr) {
        return nullptr;
    }

    byte* next_block_ = reinterpret_cast<byte*>(get_next_existing_block(that_block));
    if (next_block_ != nullptr)
    {
        *reinterpret_cast<void**>(next_block_ + sizeof(void*) + sizeof(size_t)) = place_to_put;
    }

    byte* prev_block = reinterpret_cast<byte*>(that_block);
    if (at_start)
    {
        next_block = reinterpret_cast<byte*>(that_block);
        that_block = nullptr;
    }
    else if (prev_block != nullptr)
    {
        *reinterpret_cast<void**>(prev_block + 2 * sizeof(void*) + sizeof(size_t)) = place_to_put;
    }


    if (whole_block_size - size - occupied_block_metadata_size < occupied_block_metadata_size) {
        size = whole_block_size - occupied_block_metadata_size;
    }

    return create_block_metadata(place_to_put, size, that_block, reinterpret_cast<void*>(next_block));
}

void* allocator_boundary_tags::allocate_best_fit(void* left_elem, size_t size) {
    void* place_to_put = nullptr;
    void* that_block = nullptr;

    size_t whole_block_size = get_size() + 1;
    bool at_start = false;

    void* next_block = get_next_existing_block(left_elem);
    void* block = left_elem;

    while (next_block != nullptr) {
        size_t size_between_blocks = get_block_distance(block, next_block);
        if (size_between_blocks < whole_block_size && size_between_blocks >= size + occupied_block_metadata_size) {
            whole_block_size = size_between_blocks;
            that_block = block;
            place_to_put = slide_block_for(block, occupied_block_metadata_size + get_block_data_size(block));
        }
        block = next_block;
        next_block = get_next_existing_block(block);
    }
    size_t size_between_end_and_block = ((reinterpret_cast<byte*>(block) + get_block_data_size(block) + occupied_block_metadata_size) - (reinterpret_cast<byte*>(_trusted_memory) + allocator_metadata_size));

    if (get_size() - size_between_end_and_block >= size + occupied_block_metadata_size && get_size() - size_between_end_and_block < whole_block_size + occupied_block_metadata_size) {
        whole_block_size = get_size() - size_between_end_and_block;
        that_block = block;
        place_to_put = slide_block_for(block, occupied_block_metadata_size + get_block_data_size(block));
    }

    if (slide_block_for(_trusted_memory, allocator_metadata_size) != left_elem) {
        void* pointer_after_meta = slide_block_for(_trusted_memory, allocator_metadata_size);
        if (get_block_distance(pointer_after_meta, left_elem) >= occupied_block_metadata_size + size && get_block_distance(pointer_after_meta, left_elem) < whole_block_size && place_to_put == nullptr) {
            whole_block_size = get_block_distance(pointer_after_meta, left_elem);
            that_block = left_elem;
            place_to_put = pointer_after_meta;
            at_start = true;
        }
    }

    if (at_start)
    {
        *get_first_block_ptr() = place_to_put;
    }
    if (place_to_put == nullptr)
    {
        return nullptr;
    }
    byte* next_block_ = reinterpret_cast<byte*>(get_next_existing_block(that_block));
    if (next_block_ != nullptr)
    {
        *reinterpret_cast<void**>(next_block_ + sizeof(void*) + sizeof(size_t)) = place_to_put;
    }

    byte* prev_block = reinterpret_cast<byte*>(that_block);
    if (at_start)
    {
        next_block = reinterpret_cast<byte*>(that_block);
        that_block = nullptr;
    } else if (prev_block != nullptr)
    {
        *reinterpret_cast<void**>(prev_block + 2 * sizeof(void*) + sizeof(size_t)) = place_to_put;
    }


    return create_block_metadata(place_to_put, size, that_block, reinterpret_cast<void*>(next_block));

}

void* allocator_boundary_tags::allocate_worst_fit(void* left_elem, size_t size) {

    void* place_to_put = nullptr;
    void* that_block = nullptr;

    size_t whole_block_size = 0;
    bool at_start = false;

    void* next_block = get_next_existing_block(left_elem);
    void* block = left_elem;

    while (next_block != nullptr) {
        size_t size_between_blocks = get_block_distance(block, next_block);
        if (size_between_blocks >= whole_block_size && size_between_blocks >= size + occupied_block_metadata_size) {
            whole_block_size = size_between_blocks;
            that_block = block;
            place_to_put = slide_block_for(block, occupied_block_metadata_size + get_block_data_size(block));
        }
        block = next_block;
        next_block = get_next_existing_block(block);
    }
    size_t size_between_end_and_block = ((reinterpret_cast<byte*>(block) + get_block_data_size(block) + occupied_block_metadata_size) - (reinterpret_cast<byte*>(_trusted_memory) + allocator_metadata_size));
    if (get_size() - size_between_end_and_block >= size + occupied_block_metadata_size && get_size() - size_between_end_and_block > whole_block_size + occupied_block_metadata_size) {
        whole_block_size = get_size() - size_between_end_and_block;
        that_block = block;
        place_to_put = slide_block_for(block, occupied_block_metadata_size + get_block_data_size(block));
    }

    if (slide_block_for(_trusted_memory, allocator_metadata_size) != left_elem) {
        void* pointer_after_meta = slide_block_for(_trusted_memory, allocator_metadata_size);
        if (get_block_distance(pointer_after_meta, left_elem) >= occupied_block_metadata_size + size && get_block_distance(pointer_after_meta, left_elem) > whole_block_size && place_to_put == nullptr) {
            whole_block_size = get_block_distance(pointer_after_meta, left_elem);
            that_block = left_elem;
            place_to_put = pointer_after_meta;
            at_start = true;
        }
    }

    if (at_start)
    {
        *get_first_block_ptr() = place_to_put;
    }

    if (place_to_put == nullptr)
    {
        return nullptr;
    }
    byte* next_block_ = reinterpret_cast<byte*>(get_next_existing_block(that_block));

    if (next_block_ != nullptr)
    {
        *reinterpret_cast<void**>(next_block_ + sizeof(void*) + sizeof(size_t)) = place_to_put;
    }

    byte* prev_block = reinterpret_cast<byte*>(that_block);

    if (at_start)
    {
        next_block = reinterpret_cast<byte*>(that_block);
        that_block = nullptr;
    }
    else if (prev_block != nullptr)
    {
        *reinterpret_cast<void**>(prev_block + 2 * sizeof(void*) + sizeof(size_t)) = place_to_put;
    }


    return create_block_metadata(place_to_put, size, that_block, reinterpret_cast<void*>(next_block));

}


void* allocator_boundary_tags::create_block_metadata(void* block, size_t size, void* previous, void* next) noexcept {
    trace_with_guard("Creating new block metadata");
    byte* meta = reinterpret_cast<byte*>(block);

    *reinterpret_cast<void**>(meta) = _trusted_memory;
    meta += sizeof(void*);

    *reinterpret_cast<size_t*>(meta) = size;
    meta += sizeof(size_t);

    *reinterpret_cast<void**>(meta) = previous;
    meta += sizeof(void*);

    *reinterpret_cast<void**>(meta) = next;
    meta += sizeof(void*);
    trace_with_guard("Done creating new block metadata");
    return meta;
}

void allocator_boundary_tags::do_deallocate_sm(
    void *at)
{
    trace_with_guard("Starting to deallocate");
    std::lock_guard lock(get_mutex());

    if (at == nullptr) {
        return;
    }

    void* trusted = *reinterpret_cast<void**>(reinterpret_cast<byte*>(at) - occupied_block_metadata_size);
    if (trusted != _trusted_memory || trusted == nullptr)
    {
        error_with_guard("Tried to deallocate not allocator's property");
        throw std::logic_error("Not allocator's property");
        return;
    }

    void* block = reinterpret_cast<void*>(reinterpret_cast<byte*>(at) - occupied_block_metadata_size);

    debug_with_guard(get_dump(reinterpret_cast<int*>(at), get_block_data_size(block)));
    debug_with_guard(get_info_in_string());

    void* previous_block = get_previous_existing_block(block);
    void* next_block = get_next_existing_block(block);

    if (previous_block != nullptr && next_block != nullptr) {

        byte* mover = reinterpret_cast<byte*>(next_block);
        mover = mover + sizeof(void*) + sizeof(size_t);
        *reinterpret_cast<void**>(mover) = previous_block;

        mover = reinterpret_cast<byte*>(previous_block);
        mover = mover + 2 * sizeof(void*) + sizeof(size_t);
        *reinterpret_cast<void**>(mover) = next_block;
    }
    else if (previous_block != nullptr) {
        byte* mover = reinterpret_cast<byte*>(previous_block);
        mover = mover + 2 * sizeof(void*) + sizeof(size_t);
        *reinterpret_cast<void**>(mover) = nullptr;
    }
    else if (next_block != nullptr) {
        byte* mover = reinterpret_cast<byte*>(next_block);
        mover = mover + sizeof(void*) + sizeof(size_t);
        *reinterpret_cast<void**>(mover) = nullptr;
        *get_first_block_ptr() = next_block;
    }
    else {
        *get_first_block_ptr() = nullptr;
    }
    debug_with_guard(get_info_in_string());
    debug_with_guard("Deallocating done");
}

allocator_with_fit_mode::fit_mode allocator_boundary_tags::get_fit_mode() const
{
    auto byte_ptr = reinterpret_cast<std::byte*>(_trusted_memory);
    auto fit_mode_ptr = reinterpret_cast<allocator_with_fit_mode::fit_mode*>(byte_ptr + sizeof(logger*) + sizeof(std::pmr::memory_resource*));
    return *fit_mode_ptr;
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
    std::vector<allocator_test_utils::block_info> result;
    void* current_block = *get_first_block_ptr();

    // Если нет ни одного блока - весь размер свободен
    if (current_block == nullptr) {
        result.push_back({get_size(), false});
        return result;
    }

    // Проверяем пространство перед первым блоком
    byte* memory_start = reinterpret_cast<byte*>(_trusted_memory) + allocator_metadata_size;
    size_t space_before = reinterpret_cast<byte*>(current_block) - memory_start;

    if (space_before > 0) {
        result.push_back({space_before, false});
    }

    // Обрабатываем все блоки
    while (current_block != nullptr) {
        size_t block_size = get_block_data_size(current_block);
        result.push_back({block_size + occupied_block_metadata_size, true});

        void* next_block = get_next_existing_block(current_block);
        byte* current_block_end = reinterpret_cast<byte*>(current_block) + occupied_block_metadata_size + block_size;

        if (next_block != nullptr) {
            // Промежуток между текущим и следующим блоком
            size_t gap_size = reinterpret_cast<byte*>(next_block) - current_block_end;
            if (gap_size > 0) {
                result.push_back({gap_size, false});
            }
        } else {
            // Пространство после последнего блока
            byte* memory_end = memory_start + get_size();
            size_t remaining_space = memory_end - current_block_end;
            if (remaining_space > 0) {
                result.push_back({remaining_space, false});
            }
        }

        current_block = next_block;
    }

    // Объединяем соседние свободные блоки
    for (auto it = result.begin(); it != result.end(); ) {
        if (!it->is_block_occupied && it + 1 != result.end() && !(it + 1)->is_block_occupied) {
            it->block_size += (it + 1)->block_size;  // Слияние двух свободных блоков
            result.erase(it + 1);  // Удаляем второй блок
        } else {
            ++it;
        }
    }

    // Отладочный вывод для отслеживания состояния блоков
    for (const auto& block : result) {
        std::cout << (block.is_block_occupied ? "<occup>" : "<avail>")
                  << "<" << block.block_size << "> | ";
    }
    std::cout << std::endl;

    return result;
}


// std::vector<allocator_test_utils::block_info> allocator_boundary_tags::get_blocks_info() const
// {
//     std::vector<allocator_test_utils::block_info> result;
//     void* current_block = *get_first_block_ptr();
//
//     // Если нет ни одного блока - весь размер свободен
//     if (current_block == nullptr) {
//         result.push_back({get_size(), false});
//         return result;
//     }
//
//     // Проверяем пространство перед первым блоком
//     byte* memory_start = reinterpret_cast<byte*>(_trusted_memory) + allocator_metadata_size;
//     size_t space_before = reinterpret_cast<byte*>(current_block) - memory_start;
//
//     if (space_before > 0) {
//         result.push_back({space_before, false});
//     }
//
//     // Обрабатываем все блоки
//     while (current_block != nullptr) {
//         size_t block_size = get_block_data_size(current_block);
//         result.push_back({block_size, true});
//
//         void* next_block = get_next_existing_block(current_block);
//         byte* current_block_end = reinterpret_cast<byte*>(current_block) + occupied_block_metadata_size + block_size;
//
//         if (next_block != nullptr) {
//             // Промежуток между текущим и следующим блоком
//             size_t gap_size = reinterpret_cast<byte*>(next_block) - current_block_end;
//             if (gap_size > 0) {
//                 result.push_back({gap_size, false});
//             }
//         } else {
//             // Пространство после последнего блока
//             byte* memory_end = memory_start + get_size();
//             size_t remaining_space = memory_end - current_block_end;
//             if (remaining_space > 0) {
//                 result.push_back({remaining_space, false});
//             }
//         }
//
//         current_block = next_block;
//     }
//
//     // Объединяем соседние свободные блоки
//     for (auto it = result.begin(); it != result.end(); ) {
//         // Если оба соседних блока свободны, объединяем их
//         if (!it->is_block_occupied && it + 1 != result.end() && !(it + 1)->is_block_occupied) {
//             it->block_size += (it + 1)->block_size;  // Слияние двух свободных блоков
//             result.erase(it + 1);  // Удаляем второй блок
//         } else {
//             ++it;
//         }
//     }
//
//     return result;
// }

// std::vector<allocator_test_utils::block_info> allocator_boundary_tags::get_blocks_info() const
// {
//     std::vector<allocator_test_utils::block_info> result;
//     void* current_block = *get_first_block_ptr();
//
//     // Если нет ни одного блока - весь размер свободен
//     if (current_block == nullptr) {
//         result.push_back({get_size(), false});
//         return result;
//     }
//
//     // Проверяем пространство перед первым блоком
//     byte* memory_start = reinterpret_cast<byte*>(_trusted_memory) + allocator_metadata_size;
//     size_t space_before = reinterpret_cast<byte*>(current_block) - memory_start;
//
//     if (space_before > 0) {
//         result.push_back({space_before, false});
//     }
//
//     // Обрабатываем все блоки
//     while (current_block != nullptr) {
//         size_t block_size = get_block_data_size(current_block);
//         result.push_back({block_size, true});
//
//         void* next_block = get_next_existing_block(current_block);
//         byte* current_block_end = reinterpret_cast<byte*>(current_block) + occupied_block_metadata_size + block_size;
//
//         if (next_block != nullptr) {
//             // Промежуток между текущим и следующим блоком
//             size_t gap_size = reinterpret_cast<byte*>(next_block) - current_block_end;
//             if (gap_size > 0) {
//                 result.push_back({gap_size, false});
//             }
//         } else {
//             // Пространство после последнего блока
//             byte* memory_end = memory_start + get_size();
//             size_t remaining_space = memory_end - current_block_end;
//             if (remaining_space > 0) {
//                 result.push_back({remaining_space, false});
//             }
//         }
//
//         current_block = next_block;
//     }
//
//     // Объединяем соседние свободные блоки
//     for (auto it = result.begin(); it != result.end(); ) {
//         if (!it->is_block_occupied && it + 1 != result.end() && !(it + 1)->is_block_occupied) {
//             it->block_size += (it + 1)->block_size;
//             result.erase(it + 1);
//         } else {
//             ++it;
//         }
//     }
//
//     return result;
// }

// std::vector<allocator_test_utils::block_info> allocator_boundary_tags::get_blocks_info() const
// {
//     //trace_with_guard("started getting blocks info");
//     std::vector<allocator_test_utils::block_info> result;
//     size_t full_size_avail = 0;
//
//     void* first_block = *get_first_block_ptr();
//
//     if (first_block == nullptr) {
//         full_size_avail = get_size();
//         result.push_back({full_size_avail, false});
//         return result;
//     }
//
//     void* current_block = first_block;
//     void* next_block = get_next_existing_block(current_block);
//
//     if (get_previous_existing_block(current_block) == nullptr) {
//         size_t first_available_block_size = get_block_distance(slide_block_for(_trusted_memory, allocator_metadata_size), current_block);
//         if (first_available_block_size > 0) {
//             full_size_avail += first_available_block_size;
//             result.push_back({first_available_block_size, false});
//         }
//         result.push_back({get_block_data_size(current_block), true});
//     }
//     else {
//         throw std::logic_error("Immpossible situation: a block before the first block");
//     }
//
//     while (current_block != nullptr)
//     {
//         size_t size_between_two_blocks = 0;
//         bool t_ = false;
//         if (next_block == nullptr)
//         {
//             void* start = slide_block_for(current_block, occupied_block_metadata_size + get_block_data_size(current_block));
//             void* end = slide_block_for(_trusted_memory, allocator_metadata_size + get_size());
//
//             size_between_two_blocks = reinterpret_cast<byte*>(end) - reinterpret_cast<byte*>(start);
//
//             if (size_between_two_blocks > 0) {
//                 full_size_avail += size_between_two_blocks;
//                 result.push_back({size_between_two_blocks, false});
//             }
//             return result;
//         }
//         else
//         {
//             void* next_load_block = next_block;
//             void* end_for_current_block = slide_block_for(current_block, occupied_block_metadata_size + get_block_data_size(current_block));
//             size_between_two_blocks = reinterpret_cast<byte*>(next_block) - reinterpret_cast<byte*>(end_for_current_block);
//             if (size_between_two_blocks > 0) {
//                 full_size_avail += size_between_two_blocks;
//                 result.push_back({size_between_two_blocks, false});
//             }
//             result.push_back({get_block_data_size(next_block), true});
//         }
//         current_block = next_block;
//         next_block = get_next_existing_block(current_block);
//     }
//     //trace_with_guard("End getting info about blocks");
//     return result;
// }

inline void* allocator_boundary_tags::slide_block_for(void* block, size_t bytes) const
{
    return reinterpret_cast<void*>(reinterpret_cast<byte*>(block) + bytes);
}

void *allocator_boundary_tags::get_first_block() const noexcept
{
    byte* result = (reinterpret_cast<byte*>(_trusted_memory)) + allocator_metadata_size;
    return reinterpret_cast<void*>(result);
}

void **allocator_boundary_tags::get_first_block_ptr() const noexcept
{
    byte* result = reinterpret_cast<byte*>(_trusted_memory) + allocator_metadata_size - sizeof(void*);
    return reinterpret_cast<void**>(result);
}

void *allocator_boundary_tags::get_next_existing_block(void* left_block) const
{
    if (left_block == nullptr) {
        return nullptr;
    }
    byte* result = reinterpret_cast<byte*>(left_block);
    return *reinterpret_cast<void**>(result + sizeof(void*) + sizeof(void*) + sizeof(size_t));
}

void *allocator_boundary_tags::get_previous_existing_block(void *right_block) const
{
    if (right_block == nullptr) {
        return nullptr;
    }
    byte* result = reinterpret_cast<byte*>(right_block);
    return *reinterpret_cast<void**>(result + sizeof(void*) + sizeof(size_t));
}

size_t allocator_boundary_tags::get_block_distance(void* left_block, void* right_block) const
{
    if (left_block == right_block) {
        return 0;
    }
    return (reinterpret_cast<byte*>(right_block) - reinterpret_cast<byte*>(left_block) - get_block_data_size(left_block) - occupied_block_metadata_size);
}

size_t allocator_boundary_tags::get_block_data_size(void* block) const
{
    byte* result = reinterpret_cast<byte*>(block);
    return *reinterpret_cast<size_t*>(result + sizeof(void*));
}

inline logger *allocator_boundary_tags::get_logger() const
{
    if (_trusted_memory == nullptr)
    {
        return nullptr;
    }
    return *reinterpret_cast<logger**>(_trusted_memory);
}

std::mutex &allocator_boundary_tags::get_mutex() const noexcept
{
    auto byte_ptr = reinterpret_cast<std::byte*>(_trusted_memory);
    return *reinterpret_cast<std::mutex*>(byte_ptr + sizeof(logger*) + sizeof(std::pmr::memory_resource*) + sizeof(allocator_with_fit_mode::fit_mode) + sizeof(size_t));
}

size_t allocator_boundary_tags::get_size() const
{
    auto byte_ptr = reinterpret_cast<std::byte*>(_trusted_memory);
    return *reinterpret_cast<size_t*>(byte_ptr + sizeof(logger*) + sizeof(std::pmr::memory_resource*) + sizeof(fit_mode));
}

inline std::string allocator_boundary_tags::get_typename() const noexcept
{
    return "allocator_boundary_tags";
}

std::vector<allocator_test_utils::block_info> allocator_boundary_tags::get_blocks_info_inner() const
{
    // std::vector<allocator_test_utils::block_info> blocks_info;
    //
    // // Проверка, есть ли выделенная память
    // if (!_trusted_memory) {
    //     return blocks_info;
    // }
    //
    // // Инициализация итераторов для начала и конца области памяти
    // for (auto it = begin(); it != end(); ++it) {
    //     // Собираем информацию о каждом блоке
    //     allocator_test_utils::block_info info;
    //     info.block_size = it.size();    // Размер текущего блока
    //     info.is_block_occupied = it.occupied(); // Статус блока: занят или нет
    //
    //     // Добавляем информацию о блоке в вектор
    //     blocks_info.push_back(info);
    // }
    //
    // return blocks_info;
    throw not_implemented("allocator_red_black_tree::allocator_red_black_tree(allocator_red_black_tree &&) noexcept", "your code should be here...");
}

std::string allocator_boundary_tags::get_info_in_string() noexcept
{
    trace_with_guard("started loging info");
    std::vector<allocator_test_utils::block_info> vec = get_blocks_info();

    std::ostringstream str;
    for (auto& it : vec)
    {
        if (it.is_block_occupied)
        {
            str << "<occup>";
        } else
            str << "<avail>";

        str << "<" + std::to_string(it.block_size) + "> | ";
    }
    trace_with_guard("ended loging info");
    return str.str();
}

std::string allocator_boundary_tags::get_dump(int* at, size_t size)
{
    std::string result;
    for (size_t i = 0; i < size; ++i)
    {
        result += std::to_string(static_cast<int>(at[i])) + " ";
    }
    return result;
}

allocator_boundary_tags::allocator_boundary_tags(const allocator_boundary_tags &other)
{
    if (other._trusted_memory == nullptr) {
        _trusted_memory = nullptr; // Если в другом объекте память не выделена, то и в этом объекте она не выделяется
        return;
    }

    // Копирование метаданных из другого объекта
    auto byte_ptr_other = reinterpret_cast<std::byte*>(other._trusted_memory);
    size_t total_size = *reinterpret_cast<size_t*>(byte_ptr_other + sizeof(logger*) + sizeof(std::pmr::memory_resource*) + sizeof(allocator_with_fit_mode::fit_mode) + sizeof(size_t));

    // Выделяем новую память
    _trusted_memory = ::operator new(total_size, std::align_val_t(alignof(std::max_align_t)));

    if (!_trusted_memory) {
        throw std::bad_alloc(); // Если не удалось выделить память, генерируем исключение
    }

    // Копируем данные из other в текущий объект
    std::memcpy(_trusted_memory, other._trusted_memory, total_size);

    // Здесь мы можем добавить дополнительную логику, если нужно, для копирования специфичных частей данных
    // Например, логгеры и другие структуры
}

allocator_boundary_tags &allocator_boundary_tags::operator=(const allocator_boundary_tags &other)
{
    if (this != &other) {
        // Очистка текущего объекта, если память была выделена
        if (_trusted_memory) {
            ::operator delete(_trusted_memory);
            _trusted_memory = nullptr;
        }

        // Копирование данных из другого объекта
        if (other._trusted_memory != nullptr) {
            auto byte_ptr_other = reinterpret_cast<std::byte*>(other._trusted_memory);
            size_t total_size = *reinterpret_cast<size_t*>(byte_ptr_other + sizeof(logger*) + sizeof(std::pmr::memory_resource*) + sizeof(allocator_with_fit_mode::fit_mode) + sizeof(size_t));

            // Выделяем память для нового объекта
            _trusted_memory = ::operator new(total_size);

            if (!_trusted_memory) {
                throw std::bad_alloc(); // Если не удалось выделить память
            }

            // Копируем данные
            std::memcpy(_trusted_memory, other._trusted_memory, total_size);
        }
    }
    return *this;
}

bool allocator_boundary_tags::do_is_equal(const std::pmr::memory_resource &other) const noexcept
{
    // Попытка привести другой объект к типу allocator_boundary_tags
    const allocator_boundary_tags* other_alloc = dynamic_cast<const allocator_boundary_tags*>(&other);

    // Если приведение не удалось, возвращаем false
    if (!other_alloc) {
        return false;
    }

    // Теперь, когда мы уверены, что приведение прошло успешно, можно сравнивать данные
    auto byte_ptr_this = reinterpret_cast<std::byte*>(_trusted_memory);
    auto byte_ptr_other = reinterpret_cast<std::byte*>(other_alloc->_trusted_memory);

    // Сравниваем метаданные, включая fit_mode и другие компоненты
    auto fit_mode_this = *reinterpret_cast<allocator_with_fit_mode::fit_mode*>(byte_ptr_this + sizeof(logger*) + sizeof(std::pmr::memory_resource*));
    auto fit_mode_other = *reinterpret_cast<allocator_with_fit_mode::fit_mode*>(byte_ptr_other + sizeof(logger*) + sizeof(std::pmr::memory_resource*));

    if (fit_mode_this != fit_mode_other) {
        return false; // Если fit_mode различаются, аллокаторы не равны
    }

    // Можно добавить дополнительные проверки для других частей метаданных

    return true;
}
/*
allocator_boundary_tags::boundary_iterator allocator_boundary_tags::begin() const noexcept
{
    void* start_block = *reinterpret_cast<void **>(reinterpret_cast<byte*>(_trusted_memory) + allocator_metadata_size);
    return boundary_iterator(start_block);
}

allocator_boundary_tags::boundary_iterator allocator_boundary_tags::end() const noexcept
{
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
*/