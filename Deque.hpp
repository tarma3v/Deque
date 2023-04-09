//
// Created by shatar on 02.03.2022.
//

#ifndef DEQUE_DEQUE_H
#define DEQUE_DEQUE_H

#include <vector>
#include <iterator>

template<bool is_const, typename T1, typename T2>
struct conditional_s {
    using type = T1;
};

template<typename T1, typename T2>
struct conditional_s<false, T1, T2> {
    using type = T2;
};

template<bool is_const, typename T1, typename T2>
using conditional = typename conditional_s<is_const, T1, T2>::type;

template<typename T>
class Deque {
public:

    using value_type = T;
    using size_type = size_t;
    using reference = value_type&;
    using const_reference = const value_type&;

    Deque() noexcept {
        pointers = new T*[1];
        pointers[0] = reinterpret_cast<T*>(new char[sizeof(T) * ArrSize]);
        head_pointer = tail_pointer = 0;
        head_pos = tail_pos = 0;
        deque_size = 1;
    }

    Deque(const Deque& other) noexcept {
        try {
            pointers = new T*[other.deque_size];
            for (size_t i = 0; i < other.deque_size; ++i) {
                pointers[i] = reinterpret_cast<T*>(new char[sizeof(T) * ArrSize]);
            }
        } catch(...) {}
        size_t pos = other.head_pos;
        size_t ptr = other.head_pointer;
        for (size_t i = 0; i < other.size(); ++i, increase(pos, ptr)) {
            pointers[ptr][pos] = other[i];
        }
        head_pos = other.head_pos;
        head_pointer = other.head_pointer;
        tail_pos = other.tail_pos;
        tail_pointer = other.tail_pointer;
        deque_size = other.deque_size;
    }

    Deque(size_t number, const T& elem = T()) noexcept {
        deque_size = number / ArrSize + 1;
        head_pointer = 0;
        tail_pointer = number / ArrSize;
        head_pos = 0;
        tail_pos = number % ArrSize;
        try {
            pointers = new T *[deque_size];
        } catch(...) {}
        for (size_t i = 0; i < deque_size; ++i) {
            try {
                pointers[i] = reinterpret_cast<T *>
                (new char[sizeof(T) * ArrSize]);
            } catch(...) {}
        }
        for (size_t i = 0, j = 0; i != tail_pointer ||
                                    j != tail_pos; increase(j, i)) {
            try {
                new(pointers[i] + j) T(elem);
            } catch (...) {}
        }
    }

    ~Deque() {
        for (size_t i = 0; i < deque_size; ++i) {
            (*this)[i].~T();
        }
        for (size_t i = 0; i < deque_size; ++i) {
            delete[] reinterpret_cast<char*>(pointers[i]);
        }
        delete[] pointers;
    }

    void swap(Deque& other) noexcept {
        std::swap(other.pointers, pointers);
        std::swap(other.deque_size, deque_size);
        std::swap(other.head_pos, head_pos);
        std::swap(other.head_pointer, head_pointer);
        std::swap(other.tail_pointer, tail_pointer);
        std::swap(other.tail_pos, tail_pos);
    }

    Deque& operator=(const Deque& other) noexcept {
        Deque tmp(other);
        swap(tmp);
        return *this;
    }

    T& operator[](size_t index) noexcept {
        return pointers[head_pointer + (head_pos + index) / ArrSize]
        [(head_pos + index) % ArrSize];
    }

    const T& operator[](size_t index) const noexcept {
        return pointers[head_pointer + (head_pos + index) / ArrSize]
        [(head_pos + index) % ArrSize];
    }

    T& at(size_t index) {
        if (index >= size()) {
            throw std::out_of_range("out of range");
        }
        return (*this)[index];
    }

    const T& at(size_t index) const {
        if (index >= size()) {
            throw std::out_of_range("out of range");
        }
        return (*this)[index];
    }

    size_t size() const noexcept {
        return (tail_pointer - head_pointer) * ArrSize + tail_pos - head_pos;
    }

    void push_back(const T& new_val) {
        if (tail_pointer == deque_size - 1 && tail_pos == ArrSize - 1) {
            try {
                allocate();
            } catch(...) {
                throw;
            }
        }
        new(pointers[tail_pointer] + tail_pos) T(new_val);
        increase(tail_pos, tail_pointer);
    }

    void push_front(const T& new_val) {
        if (head_pointer == 0 && head_pos == 0) {
            try {
                allocate();
            } catch(...) {
                throw;
            }
        }
        decrease(head_pos, head_pointer);
        new(pointers[head_pointer] + head_pos) T(new_val);
    }

    void pop_back() {
        (pointers[tail_pointer] + tail_pos) -> ~T();
        try {
            decrease(tail_pos, tail_pointer);
        }
        catch(const std::out_of_range& err) {}
    }

    void pop_front() noexcept {
        (pointers[head_pointer] + head_pos) -> ~T();
        increase(head_pos, head_pointer);
    }

    template<bool is_const>
    struct common_iterator {
        using value_type = conditional<is_const, const T, T>;
        using pointer = value_type*;
        using reference = conditional<is_const, const T&, T&>;
        using iterator_category = std::random_access_iterator_tag;
        using difference_type = int;

        common_iterator(const common_iterator& other) = default;

        common_iterator& operator=(const common_iterator& other) noexcept {
            common_iterator tmp(other);
            std::swap(deque_pointer, tmp.deque_pointer);
            std::swap(position, tmp.position);
            return *this;
        }

        common_iterator (T*& deque_pointer, size_t position) :
                        deque_pointer(&deque_pointer), position(position) {}

        common_iterator& operator++() noexcept {
            position = (position + 1) % ArrSize;
            if (position == 0) {
                ++deque_pointer;
            }
            return *this;
        }

        common_iterator& operator--() noexcept {
            if (position > 0) {
                --position;
            } else {
                --deque_pointer;
                position = ArrSize - 1;
            }
            return *this;
        }

        common_iterator operator++(int) noexcept {
            common_iterator tmp = *this;
            ++(*this);
            return tmp;
        }

        common_iterator operator--(int) noexcept {
            common_iterator tmp = *this;
            --(*this);
            return tmp;
        }

        common_iterator& operator+=(difference_type number) noexcept {
            deque_pointer += (position + number) / ArrSize;
            position = (position + number) % ArrSize;
            return *this;
        }

        common_iterator& operator-=(difference_type number) noexcept {
            if (position < number) {
                deque_pointer -= (number - position + ArrSize - 1) / ArrSize;
            }
            position = (position + ArrSize - number % ArrSize) % ArrSize;
            return *this;
        }

        common_iterator operator+(difference_type number) const noexcept {
            common_iterator tmp = *this;
            tmp += number;
            return tmp;
        }

        common_iterator operator-(difference_type number) const noexcept {
            common_iterator tmp = *this;
            tmp -= number;
            return tmp;
        }

        operator common_iterator<true>() noexcept {
            return common_iterator<true>(deque_pointer, position);
        }

        bool operator<(const common_iterator& other) const noexcept {
            if (deque_pointer < other.deque_pointer) {
                return true;
            } else if (deque_pointer > other.deque_pointer) {
                return false;
            }
            return position < other.position;
        }

        bool operator>(const common_iterator& other) const noexcept {
            return other < *this;
        }

        bool operator<=(const common_iterator& other) const noexcept {
            return !(other < *this);
        }

        bool operator>=(const common_iterator& other) const noexcept {
            return !(*this < other);
        }

        bool operator==(const common_iterator& other) const noexcept {
            return (*this <= other) && (other <= *this);
        }

        bool operator!=(const common_iterator& other) const noexcept {
            return !(*this == other);
        }

        difference_type operator-(const common_iterator& other) const noexcept {
            if (*this < other) {
                return (other - *this) * (-1);
            }
            if (position >= other.position) {
                return position - other.position +
                    (deque_pointer - other.deque_pointer) * ArrSize;
            } else {
                return position + ArrSize - other.position
                + (deque_pointer - other.deque_pointer - 1) * ArrSize;
            }
        }

        reference operator*() const noexcept {
            return *(*(deque_pointer) + position);
        }

        pointer operator->() const noexcept {
            return &((*deque_pointer)[position]);
        }

    private:
        T** deque_pointer;
        difference_type position;
        const size_t ArrSize = 32;
    };

    using iterator = common_iterator<false>;
    using const_iterator = common_iterator<true>;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    void insert (iterator insert_it, const T& new_val) {
        auto dump = *this;
        try {
            T tmp = new_val;
            for (; insert_it != end(); ++insert_it) {
                std::swap(*insert_it, tmp);
            }
            push_back(tmp);
        } catch(...) {
            swap(dump);
            throw;
        }
    }

    void erase(iterator it) {
        auto dump = *this;
        try {
            for (; it + 1 != end(); ++it) {
                std::swap(*it, *(it + 1));
            }
            pop_back();
        } catch(...) {
            swap(dump);
            throw;
        }
    }

    iterator begin() noexcept {
        return iterator(pointers[head_pointer], head_pos);
    }

    iterator end() noexcept {
        return iterator(pointers[tail_pointer], tail_pos);
    }

    const_iterator begin() const noexcept {
        return const_iterator(pointers[head_pointer], head_pos);
    }

    const_iterator end() const noexcept {
        return const_iterator(pointers[tail_pointer], tail_pos);
    }

    const_iterator cbegin() const noexcept {
        return const_iterator(pointers[head_pointer], head_pos);
    }

    const_iterator cend() const noexcept {
        return const_iterator(pointers[tail_pointer], tail_pos);
    }

    reverse_iterator rbegin() noexcept {
        return reverse_iterator(end());
    }

    const_reverse_iterator rbegin() const noexcept {
        return const_reverse_iterator(end());
    }

    const_reverse_iterator crbegin() const noexcept {
        return const_reverse_iterator(end());
    }

    reverse_iterator rend() noexcept {
        return reverse_iterator(begin());
    }

    const_reverse_iterator rend() const noexcept {
        return const_reverse_iterator(begin());
    }

    const_reverse_iterator crend() const noexcept {
        return const_reverse_iterator(begin());
    }

private:
    T** pointers;
    size_t head_pointer;
    size_t head_pos;
    size_t tail_pointer;
    size_t tail_pos;
    size_t deque_size;
    const size_t ArrSize = 32;

    void allocate() {
        size_t old_size = tail_pointer - head_pointer + 1;
        T** new_pointers;
        try {
            new_pointers = new T*[3 * old_size];
        } catch(...) {
            throw;
        }
        for (size_t i = 0; i < 3 * old_size; ++i) {
            if (i >= old_size && i < 2 * old_size) {
                new_pointers[i] = pointers[i - old_size + head_pointer];
            } else {
                try {
                    new_pointers[i] = reinterpret_cast<T *>
                    (new char[sizeof(T) * ArrSize]);
                } catch(...) {
                    for (size_t j = 0; j < i; ++j) {
                        if (i < old_size || i >= 2 * old_size) {
                            delete[] new_pointers[j];
                        }
                    }
                    throw;
                }
            }
        }
        for (size_t i = 0; i < deque_size; ++i) {
            if (i < head_pointer || i > tail_pointer) {
                delete[] pointers[i];
            }
        }
        delete[] pointers;
        pointers = new_pointers;
        head_pointer = old_size;
        tail_pointer = 2 * old_size - 1;
        deque_size = 3 * old_size;
    }

    void decrease(size_t& pos, size_t& pointer) {
        if (pos == 0) {
            pos = ArrSize;
            if (pointer != 0) {
                pointer--;
            } else {
                throw std::out_of_range("Decrease of 0");
            }
        }
        pos--;
    }
    void increase(size_t& pos, size_t& pointer) noexcept {
        if (pos == ArrSize - 1) {
            pos = 0;
            pointer++;
        } else {
            pos++;
        }
    }
};
//Size
#endif //DEQUE_DEQUE_H
