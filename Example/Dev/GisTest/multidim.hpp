/**
 * \file multidim.hpp
 * \brief Header file of multidim library
 * \version 0.9.0
 * \date 2016-01-17
 * \author Alberto Marnetto
 * \par Webpage
 * <<https://github.com/AlbertoMarnetto/multidim>>
 * \copyright Copyright (c) 2015, 2016 Alberto Marnetto.
 *  Distributed under the MIT License
 *
 * The MIT License (MIT)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

#ifndef MULTIDIM_H
#define MULTIDIM_H

#include <iterator> // begin, end
#include <string> // DefaultCustomScalar, to_string
#include <vector> // computeContainerGeometry
#include <type_traits> // is_lvalue_reference, enable_if etc.
#include <limits> // std::numeric_limits
#include <algorithm> // std::copy_n

// Since this is intended as a general-purpose module,
// I prefer not to #include any Boost library

namespace multidim {

/** \defgroup user_functions Functions
 *  \brief Functions to be used by client code
 */

/** \defgroup custom_scalars "CustomScalar" traits
 * \brief Implementations for the "ScalarPolicy" template argument of
 * the library main classes and functions.
 * Each implementation indicates which classes should be considered scalars,
 * even if they are containers.
 */

/** \defgroup flat_view FlatView
 * \brief FlatView and related classes
 */

/** \defgroup boxed_view BoxedView
 * \brief A class which makes a multilevel range appear as a C array
 * of user-defined bounds, cropping and filling as needed.
 */

/** \defgroup detail Implementation details
 * \brief Low-level machinery extending the `<type_traits>` library.
 * Should not be used by client code
 */


using std::begin;
using std::end;

constexpr size_t NO_VALUE = static_cast<size_t>(-1);

// **************************************************************************
// size
// **************************************************************************
/** \fn auto size(const Container& container) -> size_t
 * \ingroup detail
 * \brief Returns the size of `container`
 * \param container A container (is is only required to support `begin()` and `end()`)
 */
template<typename Container>
static auto _size(const Container& container, long) -> decltype(begin(container), end(container), size_t()) {
    return std::distance(begin(container), end(container));
}
template<typename Container>
static auto _size(const Container& container, int) -> decltype(container.size(), size_t()) {
    return container.size();
}

// Entry point
template<typename Container>
static auto size(const Container& container) -> size_t {
    return _size(container, 0);
}

// **************************************************************************
// Trivial tag classes
// **************************************************************************
/** \brief Trivial tag class to indicate the seek direction of an iterator
 * \ingroup detail
 */
struct Forward {};
/** \brief Trivial tag class to indicate the seek direction of an iterator
 * \ingroup detail
 */
struct Backward {};

//***************************************************************************
// IsRange
//***************************************************************************
/**
 * \brief Provides the member constant `value`, which is
 * `true` if `begin(T_MD)` and `end(T_MD)` return the same type,
 * which must be an iterator class
 * `false` otherwise
 * \ingroup detail
 * \param T_MD (typename)
 * \note Here a "Range" is defined as any class supporting begin()
*/

template <typename T_MD>
struct IsRange {
    template <typename T1 = T_MD>
    static constexpr auto isRange(int)
    -> decltype(
        begin(std::declval<T1&>()),
        end(std::declval<T1&>()),
        std::declval<typename std::iterator_traits<decltype(begin(std::declval<T1&>()))>::value_type>(),
        bool()
    ) {
        return std::is_same<decltype(begin(std::declval<T1&>())), decltype(end(std::declval<T1&>()))>::value;
    }

    static constexpr auto isRange(long) -> decltype(bool()) {
        return false;
    }

    static constexpr bool value = isRange(0);
};

//***************************************************************************
// IteratorType
//***************************************************************************
/**
 * \brief Provides the member typedef `type`
 * as the type of `begin(T_MD)` and `end(T_MD)`
 * \ingroup detail
 * \param T_MD (typename)
 */
template <typename T_MD, bool = IsRange<T_MD>::value>
struct IteratorType;

// Only defined if T_MD supports `begin`
template <typename T_MD>
struct IteratorType<T_MD, true> {
    using type = decltype(begin(std::declval<T_MD&>()));
};

//***************************************************************************
// PointsToRange
//***************************************************************************
/**
 * \brief Provides the member constant `value`, which is
 * `true` if `Iterator` is an iterator whose pointee is a range,
 * `false` otherwise
 * \ingroup detail
 * \param Iterator (typename)
*/

template <typename Iterator>
struct PointsToRange {
    template <typename Iterator1 = Iterator>
    static constexpr auto _pointsToRange(int)
    -> decltype(
        std::declval<typename std::iterator_traits<Iterator1>::reference_type>(),
        bool()
    ) {
        return IsRange<Iterator1>::value;
    }

    static constexpr auto _pointsToRange(long) -> decltype(bool()) {
        return false;
    }

    static constexpr bool value = _pointsToRange(0);
};

//***************************************************************************
// ScalarPolicy
//***************************************************************************
/** \brief Defines `std::string` and `std::wstring` as scalar types.
 * \ingroup custom_scalars
 * \param T_MD (typename, as template parameter)
 */
template<typename T_MD>
struct StringsAsScalars {
    static constexpr bool isCustomScalar =
           std::is_same<typename std::decay<T_MD>::type, std::string>::value
        || std::is_same<typename std::decay<T_MD>::type, std::wstring>::value
    ;
};

/** \brief Defines no scalar types.
 * \ingroup custom_scalars
 * \param T_MD (typename, as template parameter)
 */
template<typename T_MD>
struct NoCustomScalars{static constexpr bool isCustomScalar = false; /*for any T_MD*/};


// **************************************************************************
// IsScalar
/** \brief Provides the member constant `value`, which is
 * `true` if `T_MD` is a scalar (i.e. if it is not a container,
 * or if it defined as custom scalar by the trait)
 * \ingroup detail
 * \param T_MD (typename, as template parameter)
 */
// **************************************************************************
template <template<typename> class ScalarPolicy = NoCustomScalars, typename T_MD = void>
struct IsScalar {
    static constexpr bool value = !IsRange<T_MD>::value || ScalarPolicy<T_MD>::isCustomScalar;
};

// **************************************************************************
// IteratorScalarType
// **************************************************************************
/** \brief Provides the member typedef `type` and `reference`
 * as the value and reference scalar type
 * in a (possibly nested) iterator,
 * e.g. `vector<set<int>>::iterator` --> `type:int, reference = int&`
 * \ingroup detail
 * \param ScalarPolicy (trait template)
 * \param Iterator (typename)
 */
template <
    template<typename> class ScalarPolicy = NoCustomScalars,
    typename Iterator = void,
    bool = IsScalar<ScalarPolicy, typename std::iterator_traits<Iterator>::value_type>::value
>
struct IteratorScalarType;

// Specialization for Iterator pointing to scalar
template <template<typename> class ScalarPolicy, typename Iterator>
struct IteratorScalarType<ScalarPolicy, Iterator, true> {
    using reference = typename std::iterator_traits<Iterator>::reference;
    using type = typename std::iterator_traits<Iterator>::value_type;
};

// Specialization for Iterator pointing to a not scalar: recurse to next level
template <template<typename> class ScalarPolicy, typename Iterator>
struct IteratorScalarType<ScalarPolicy, Iterator, false> {
    using MyScalarType =
        IteratorScalarType<
            ScalarPolicy,
            decltype(begin(
                std::declval<typename std::iterator_traits<Iterator>::reference>()
            ))
        >;

    using reference = typename MyScalarType::reference;
    using type = typename MyScalarType::type;
};

//***************************************************************************
// Dimensionality
//***************************************************************************
/** \brief Provides the `size_t` member `value` as the dimensionality of a type
 * \ingroup detail
 * \param ScalarPolicy (trait template)
 * \param T_MD (typename)
 */
template <
    template<typename> class ScalarPolicy = NoCustomScalars,
    typename T_MD = void,
    bool = IsScalar<ScalarPolicy, T_MD>::value
>
struct Dimensionality;

// Specialization for scalar T_MD
template <template<typename> class ScalarPolicy, typename T_MD>
struct Dimensionality<ScalarPolicy, T_MD, true> {
    static constexpr size_t value = 0;
};

// Specialization for non-scalar T_MD
template <template<typename> class ScalarPolicy, typename T_MD>
struct Dimensionality<ScalarPolicy, T_MD, false> {
    static constexpr size_t value = 1
        + Dimensionality<
            ScalarPolicy,
            typename std::iterator_traits<typename IteratorType<T_MD>::type>::reference
        >::value;
};

// Version for ranges
template <
    template<typename> class ScalarPolicy = NoCustomScalars,
    typename Iterator = void
>
struct DimensionalityRange {
    static constexpr size_t value = 1+Dimensionality<ScalarPolicy, typename std::iterator_traits<Iterator>::reference>::value;
};


// Function versions
/** \brief Returns the dimensionality of the type of the argument
 * (or 0 if such type is a scalar)
 * \ingroup user_functions
 * \param ScalarPolicy (trait template)
 * \param argument the object whose dimensionality will be returned
 */
template <template<typename> class ScalarPolicy = NoCustomScalars, typename T_MD = void>
constexpr size_t dimensionality(T_MD const& arg) {
    return Dimensionality<ScalarPolicy, T_MD const&>::value;
}

/** \brief Returns the dimensionality of the type of the range
 * \ingroup user_functions
 * \param ScalarPolicy (trait template)
 * \param first, last the range whose dimensionality will be returned
 */
template <template<typename> class ScalarPolicy = NoCustomScalars, typename Iterator = void>
constexpr size_t dimensionality(Iterator const& first, Iterator const& last) {
    return DimensionalityRange<ScalarPolicy, Iterator>::value;
}

// **************************************************************************
// bounds
// **************************************************************************
/** \fn bounds(T_MD const& argument);
 * \brief Returns a `vector<size_t>` containing the bounds of an object,
 * i.e. its maximum size in all its subdimensions (or an empty vector
 * is the argument is a scalar)
 * \ingroup user_functions
 * \param ScalarPolicy (trait template)
 * \param argument the container whose bounds will be returned
 */

/** \fn bounds(Iterator first, Iterator last, size_t precomputedDistance = NO_VALUE);
 * \brief Returns a `vector<size_t>` containing the bounds of a range,
 * i.e. its maximum size in all its subdimensions
 * \ingroup user_functions
 * \param ScalarPolicy (trait template)
 * \param first, last  the range whose bounds will be returned
 * \param precomputedDistance (optional) the value of (last - first)
 */

// Version for scalars
template <
    template<typename> class ScalarPolicy = NoCustomScalars,
    typename T_MD = void,
    typename std::enable_if<(true == IsScalar<ScalarPolicy, T_MD>::value), long>::type = 0xBEEF
>
std::vector<size_t> bounds(T_MD const& argument) {
    return std::vector<size_t> {};
}

// forward-declare version for containers
template <
    template<typename> class ScalarPolicy = NoCustomScalars,
    typename T_MD = void,
    typename std::enable_if<(false == IsScalar<ScalarPolicy, T_MD>::value), int>::type = 0xBEEF
>
std::vector<size_t> bounds(const T_MD& argument);


// Version for monolevel ranges
template <
    template<typename> class ScalarPolicy = NoCustomScalars,
    typename Iterator = void,
    typename std::enable_if<
        (true == IsScalar<ScalarPolicy, typename std::iterator_traits<Iterator>::value_type>::value),
        long
    >::type = 0xBEEF
>
std::vector<size_t> bounds(Iterator first, Iterator last, size_t precomputedDistance = NO_VALUE) {
    if (precomputedDistance != NO_VALUE) {
        return std::vector<size_t>{precomputedDistance,};
    } else {
        return std::vector<size_t>{static_cast<size_t>(std::distance(first, last)),};
    }
}

// Version for multilevel ranges
template <
    template<typename> class ScalarPolicy = NoCustomScalars,
    typename Iterator = void,
    typename std::enable_if<
        (false == IsScalar<ScalarPolicy, typename std::iterator_traits<Iterator>::value_type>::value),
        int
    >::type = 0xBEEF
>
std::vector<size_t> bounds(Iterator const& first, Iterator const& last, size_t precomputedDistance = NO_VALUE) {
    constexpr auto rangeDimensionality = dimensionality<ScalarPolicy>(first, last);

    std::vector<size_t> containerBounds(rangeDimensionality);

    // The outermost bound is the size of the range
    if (precomputedDistance != NO_VALUE) {
        containerBounds[0] = precomputedDistance;
    } else {
        containerBounds[0] = std::distance(first, last);
    }

    // The other bounds are computed from the subcontainers info
    for (auto it = first; it != last; ++it) {
        const auto& subContainer = *it;
        const auto& subContainerBounds = bounds<ScalarPolicy>(subContainer);

        if (subContainerBounds.size() != rangeDimensionality-1) {
            throw std::runtime_error("bounds : encountered Container with strange dimensionality");
        }

        // Set tentative bounds in the subdimensions as bounds of the first subcontainer
        if (it == first) {
            std::copy(begin(subContainerBounds), end(subContainerBounds), begin(containerBounds)+1);
            continue;
        }

        // Then adapt it by looking the other subcontainers
        for (size_t dimension = 0; dimension < subContainerBounds.size(); ++dimension) {
            // if one of the other subcontainers is larger, enlarge the bounding box and set the "jagged" flag
            if (containerBounds[dimension+1] < subContainerBounds[dimension]) {
                containerBounds[dimension+1] = subContainerBounds[dimension];
            }
        }
    }

    return containerBounds;
}

// Version for containers
template <
    template<typename> class ScalarPolicy,
    typename T_MD,
    typename std::enable_if<(false == IsScalar<ScalarPolicy, T_MD>::value), int>::type
>
std::vector<size_t> bounds(T_MD const& argument) {
    return bounds<ScalarPolicy>(begin(argument), end(argument), size(argument));
}

// **************************************************************************
// scalarSize
// **************************************************************************
/** \fn scalarSize(T_MD const& argument);
 * \brief Determines the number of scalar elements contained in the argument
 * (or 1 is the argument is a scalar)
 * \ingroup user_functions
 * \param ScalarPolicy (trait template)
 * \param argument the container whose number of scalar elements will be returned
 */

/** \fn scalarSize(Iterator first, Iterator last, size_t precomputedDistance = NO_VALUE);
 * \brief Determines the bounds of a range, i.e. its maximum
 * size in all its subdimensions
 * \ingroup user_functions
 * \param ScalarPolicy (trait template)
 * \param first, last the range whose number of scalar elements will be returned
 * \param precomputedDistance (optional) the value of (last - first)
 */

// Version for scalars
template <
    template<typename> class ScalarPolicy = NoCustomScalars,
    typename T_MD = void,
    typename std::enable_if<(true == IsScalar<ScalarPolicy, T_MD>::value), long>::type = true
>
constexpr size_t scalarSize(T_MD const& argument) {
    return 1;
}

// forward-declare version for containers
template <
    template<typename> class ScalarPolicy = NoCustomScalars,
    typename T_MD = void,
    typename std::enable_if<(false == IsScalar<ScalarPolicy, T_MD>::value), int>::type = 0xBEEF
>
size_t scalarSize(const T_MD& argument);


// Version for monolevel range
template <
    template<typename> class ScalarPolicy = NoCustomScalars,
    typename Iterator = void,
    typename std::enable_if<
        (true == IsScalar<ScalarPolicy, typename std::iterator_traits<Iterator>::value_type>::value),
        long
    >::type = 0xBEEF
>
size_t scalarSize(Iterator first, Iterator last, size_t precomputedDistance = NO_VALUE) {
    if (precomputedDistance != NO_VALUE) {
        return precomputedDistance;
    } else {
        return static_cast<size_t>(std::distance(first, last));
    }
}

// Version for multilevel range
template <
    template<typename> class ScalarPolicy = NoCustomScalars,
    typename Iterator = void,
    typename std::enable_if<
        (false == IsScalar<ScalarPolicy, typename std::iterator_traits<Iterator>::value_type>::value),
        int
    >::type = 0xBEEF
>
size_t scalarSize(Iterator first, Iterator last, size_t precomputedDistance = NO_VALUE) {
    size_t result = 0;

    for (auto it = first; it != last; ++it) {
        result += scalarSize<ScalarPolicy>(*it);
    }

    return result;
}

// Version for containers
template <
    template<typename> class ScalarPolicy,
    typename T_MD,
    typename std::enable_if<(false == IsScalar<ScalarPolicy, T_MD>::value), int>::type
>
size_t scalarSize(T_MD const& argument) {
    return scalarSize<ScalarPolicy>(begin(argument), end(argument), size(argument));
}

// **************************************************************************
// scalarType
// **************************************************************************
/** \fn scalarType(T_MD const& argument);
 * \brief This function is not actually defined, it is only meant to be used
 * as parameter of `decltype`. Its return type is the type of the "leaf"
 * elements of the container passed as argument
 * \ingroup user_functions
 * \param ScalarPolicy (trait template)
 * \param argument the container of which the type of "leaf" elements will be
 * determined
 */
template <
    template<typename> class ScalarPolicy = NoCustomScalars,
    typename T_MD
>
constexpr auto scalarType(T_MD const& argument)
    -> typename IteratorScalarType<
        ScalarPolicy,
        T_MD*
       >::type;

/** \fn scalarType(Iterator first, Iterator last);
 * \brief This function is not actually defined, it is only meant to be used
 * as parameter of `decltype`. Its return type is the type of the "leaf"
 * elements of the range passed as argument
 * \ingroup user_functions
 * \param ScalarPolicy (trait template)
 * \param first, last the range of which the type of "leaf" elements will be
 * determined
 */
template <
    template<typename> class ScalarPolicy = NoCustomScalars,
    typename Iterator
>
constexpr auto scalarType(Iterator first, Iterator last)
    -> typename IteratorScalarType<ScalarPolicy,Iterator>::type;

} // namespace multidim - Basics


// **************************************************************************
// **************************************************************************
// **************************************************************************
// @FlatView
// **************************************************************************
// **************************************************************************
// **************************************************************************

namespace multidim {

// **************************************************************************
// forward declaration of FlatViewIterator
/** \brief The iterator used in FlatView.
 *  \ingroup flat_view
 */
template <
    template<typename> class ScalarPolicy,
    typename RawIterator,
    bool isConstIterator,
    bool hasSubIterator = !IsScalar<ScalarPolicy, typename std::iterator_traits<RawIterator>::value_type>::value
> class FlatViewIterator;

// operator+
template <template<typename> class ScalarPolicy, typename RawIterator, bool isConstIterator>
auto operator+(
        typename FlatViewIterator<ScalarPolicy, RawIterator, isConstIterator>::difference_type n,
        const FlatViewIterator<ScalarPolicy, RawIterator, isConstIterator>& other)
    -> FlatViewIterator<ScalarPolicy, RawIterator, isConstIterator>;

// **************************************************************************

//***************************************************************************
// FlatView
//***************************************************************************
/** \brief A class which makes the pointed range appear as a linear array.
 * It respects many of the General container requirements,
 * ([container.requirements.general]),
 * the discrepancies arising from the fact that the View does not own
 * its elements.
 * In general, we tried to follow the semantics of C++17's `string_view`.
 * \param ScalarPolicy (trait template)
 * \ingroup flat_view
 */
template <template<typename> class ScalarPolicy, typename RawIterator>
class FlatView {
public:
    using iterator = FlatViewIterator<ScalarPolicy, RawIterator, false>;
    using const_iterator =  FlatViewIterator<ScalarPolicy, RawIterator, true>;
    using value_type = typename iterator::value_type;
    using reference = typename iterator::reference;
    using const_reference = typename const_iterator::reference;
    using difference_type = typename iterator::difference_type;
    using size_type = size_t;   /**< \todo should actually be generic */

    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    FlatView() {}
    FlatView(RawIterator first, RawIterator last) : begin_{first}, end_{last} {}
    FlatView(const FlatView& other) = default;  /**< \note It performs a shallow copy! */
    ~FlatView() = default;
    FlatView& operator=(const FlatView& other) = default; /**< \note It performs a shallow copy! */

    // **************************************************************************
    // Standard members
    // **************************************************************************
    bool operator==(const FlatView& other) const {return equal(other);}
    bool operator!=(const FlatView& other) const {return !equal(other);}
    bool operator <(const FlatView& other) const {return compare(other);}
    bool operator >(const FlatView& other) const {return other.compare(*this);}
    bool operator<=(const FlatView& other) const {return !(*this > other);}
    bool operator>=(const FlatView& other) const {return !(other > *this);}

    iterator begin() {return iterator::makeBegin(begin_, end_);}
    const_iterator begin() const {return const_iterator::makeBegin(begin_, end_);}
    const_iterator cbegin() const {return const_iterator::makeBegin(begin_, end_);}
    iterator end() {return iterator::makeEnd(begin_, end_);}
    const_iterator end() const {return const_iterator::makeEnd(begin_, end_);}
    const_iterator cend() const {return const_iterator::makeEnd(begin_, end_);}
    reverse_iterator rbegin() {return reverse_iterator{iterator::makeEnd(begin_, end_)};}
    const_reverse_iterator rbegin() const {return const_reverse_iterator{const_iterator::makeEnd(begin_, end_)};}
    const_reverse_iterator crbegin() const {return const_reverse_iterator{const_iterator::makeEnd(begin_, end_)};}
    reverse_iterator rend() {return reverse_iterator{iterator::makeBegin(begin_, end_)};}
    const_reverse_iterator rend() const {return const_reverse_iterator{const_iterator::makeBegin(begin_, end_)};}
    const_reverse_iterator crend() const {return const_reverse_iterator{const_iterator::makeBegin(begin_, end_)};}

    reference front() {return *begin();}
    const_reference front() const {return *cbegin();}
    reference back() {return *--end();}
    const_reference back() const {return *--cend();}

    reference operator[](size_type n) {return *(begin()+ n);}
    const_reference operator[](size_type n) const {return *(begin()+ n);}

    void swap(const FlatView& other) {std::swap(*this, other);}
    size_type size() const {
        // size should be constant, I guess that amortized constant will do
        if (cachedSize_ == NO_VALUE) {
            cachedSize_ = std::distance(begin(), end());
        }
        return cachedSize_;
    }
    size_type max_size() const {return std::numeric_limits<size_type>::max();};
     /**<  \note Since C++17 the result of max_size should be divided by `sizeof(value_type)`,
           see http://en.cppreference.com/w/cpp/memory/allocator_traits/max_size */

    bool empty() const {return (size() == 0);}

private:
    bool equal(const FlatView& other) const {
        return (size() == other.size()) &&
               std::equal(begin(), end(), other.begin());
    }

    bool compare(const FlatView& other) const {
        return std::lexicographical_compare(begin(), end(), other.begin(), other.end());
    }

private:
    RawIterator begin_ = nullptr;
    RawIterator end_ = nullptr;
    mutable size_t cachedSize_ = NO_VALUE;
};


//***************************************************************************
// makeFlatView
//***************************************************************************
/** \brief Factory method to build a FlatView of a container.
 * \ingroup user_functions
 * \param ScalarPolicy (trait template)
 * \param container the container on which the View will be based
 */
template <template<typename> class ScalarPolicy = NoCustomScalars, typename Container = void>
auto makeFlatView(Container& container) -> FlatView<ScalarPolicy, decltype(begin(container))>  {
    return FlatView<ScalarPolicy, decltype(begin(container))>{begin(container), end(container)};
}
/** \brief Factory method to build a FlatView of a range
 * \ingroup user_functions
 * \param ScalarPolicy : (trait template)
 * \param first, last the range on which the View will be based
 */
template <template<typename> class ScalarPolicy = NoCustomScalars, typename Iterator = void>
auto makeFlatView(Iterator first, Iterator last) -> FlatView<ScalarPolicy, Iterator>  {
    return FlatView<ScalarPolicy, Iterator>{first, last};
}

//***************************************************************************
// FlatViewIterator
//***************************************************************************
// Specialization for RawIterator pointing to a subcontainer
// Invariant: the iterator can only be in one of these states:
// (1) (valid() == false && current_ == begin_) :
//     represents the element "one before the first".
// (2) (valid() == true && current_ >= begin_ && current_ < end_ && child_.valid() == true) :
//     represents a valid scalar element
// (3) (valid() == false && current_ == end_) :
//     represents the element "one past the end"
// If (begin_ == end_) then states (1) and (3) collapse, but this does not pose a problem

template <template<typename> class ScalarPolicy, typename RawIterator, bool isConstIterator>
class FlatViewIterator<ScalarPolicy, RawIterator, isConstIterator, true>
{
public:
    using RawValueType = typename IteratorScalarType<ScalarPolicy, RawIterator>::type;
    using ConstValueType = typename std::add_const<RawValueType>::type;
    using RawReferenceType = typename IteratorScalarType<ScalarPolicy, RawIterator>::reference;
    using ConstReferenceType = typename std::add_const<RawValueType>::type;
    // [iterator.traits]
    using iterator_category = std::random_access_iterator_tag;
    using value_type = typename std::conditional<isConstIterator, ConstValueType, RawValueType>::type;
    using difference_type = ptrdiff_t;
    using pointer = value_type*;
    using reference = typename std::conditional<isConstIterator, ConstReferenceType, RawReferenceType>::type;

    // **************************************************************************
    // ctors
    // **************************************************************************
    FlatViewIterator() : child_{}, begin_{nullptr}, current_{nullptr}, end_{nullptr} {}
    /* \brief Default constructor. */

    FlatViewIterator(const FlatViewIterator&) = default;
    /* \brief Copy constructor. Note that FlatViewIterator is TriviallyCopyable */

    static FlatViewIterator makeBegin(RawIterator first, RawIterator last) {
        return FlatViewIterator<ScalarPolicy, RawIterator, isConstIterator>{first, last, Forward{}};
    }
    static FlatViewIterator makeEnd(RawIterator first, RawIterator last) {
        return FlatViewIterator<ScalarPolicy, RawIterator, isConstIterator>{first, last, Backward{}};
    }

    // conversion to const_iterator
    template<bool _isConstIterator = isConstIterator>
    FlatViewIterator(FlatViewIterator<ScalarPolicy, RawIterator, false, true> other,
                     typename std::enable_if<_isConstIterator,int>::type* = nullptr) :
        child_{other.child_},
        begin_{other.begin_},
        current_{other.current_},
        end_{other.end_}
    {}

    // **************************************************************************
    // Standard members
    // **************************************************************************
    // [iterator.iterators]
    reference operator*() const {return dereference();}
    FlatViewIterator& operator++() {increment(); return *this;}

    // [input.iterators] and [output.iterators]
    bool operator==(const FlatViewIterator& other) const {return equal(other);}
    bool operator!=(const FlatViewIterator& other) const {return !((*this) == other);}
    pointer operator->() const {return &(**this);}
    FlatViewIterator operator++(int) {auto tmp = *this; ++(*this); return tmp;}

    // [bidirectional.iterators]
    FlatViewIterator& operator--() {decrement(); return *this;}
    FlatViewIterator operator--(int) {auto other = *this; --(*this); return other;}

    // [random.access.iterators]
    FlatViewIterator& operator+=(difference_type n) {advance(n); return *this;}
    FlatViewIterator operator+(difference_type n) const {auto result = (*this); result += n; return result;}
    template <template<typename> class, typename, bool>
    friend FlatViewIterator operator+(difference_type n, const FlatViewIterator& other);
    FlatViewIterator& operator-=(difference_type n) {return (*this += (-n));}
    FlatViewIterator operator-(difference_type n) const {return (*this + (-n));}

    difference_type operator-(const FlatViewIterator& other) const {return other.distance_to(*this);}
    bool operator<(const FlatViewIterator& other) const {return (other - *this) > 0;}
    bool operator>(const FlatViewIterator& other) const {return (other - *this) < 0;}
    bool operator>=(const FlatViewIterator& other) const {return !((*this) < other);}
    bool operator<=(const FlatViewIterator& other) const {return !((*this) > other);}

    reference operator[](difference_type n) const {return *(*this + n);}

    // **************************************************************************

    bool valid() const {return (current_ < end_) && (child_.valid());}

private: // funcs
    using ChildRawIterator = typename IteratorType<typename std::iterator_traits<RawIterator>::reference>::type;
    using ChildIterator = FlatViewIterator<ScalarPolicy, ChildRawIterator, isConstIterator>;

    // needed for conversion to const_iterator
    friend class FlatViewIterator<ScalarPolicy, RawIterator, true, true>;

    FlatViewIterator(RawIterator first, RawIterator last, Forward)
        : begin_{first}
        , current_{first}
        , end_{last}
    {
        increment();
    }
    FlatViewIterator(RawIterator first, RawIterator last, Backward)
        : begin_{first}
        , current_{last}
        , end_{last}
    {
    }

    // Note the asimmetry between increment and decrement:
    // it is provoked by the asimmetry between begin(),
    // which points to a potentially valid element,
    // and end(), which does not
    void increment() {
        if (valid()) {
            // child_ is already in a valid state,
            // move it forward
            ++child_;
            if (child_.valid()) return;

            // no more scalar elements at *current_
            // move ourselves forward
            ++current_;
        }

        while(1) {
            // just updated current_, try to build
            // a valid subiterator at this location
            if (current_ == end_) return;
            child_ = FlatViewIterator<ScalarPolicy, ChildRawIterator, isConstIterator>::makeBegin(begin(*current_), end(*current_));
            if (child_.valid()) return;
            ++current_;
        }
    }

    void decrement() {
        if (valid()) {
            // child_ is already in a valid state,
            // move it backward
            --child_;
            if (child_.valid()) return;
        }

        // no more scalar elements at *curr,
        // move ourselves backward
        while(1) {
            if (current_ == begin_) return;
            --current_;
            child_ = FlatViewIterator<ScalarPolicy, ChildRawIterator, isConstIterator>::makeEnd(begin(*current_), end(*current_));
            --child_;
            if (child_.valid()) return;
        }
    }

    void advance(difference_type n) {
        if (n > 0) for (difference_type i = 0; i < n; ++i) ++(*this);
        if (n < 0) for (difference_type i = 0; i > n; --i) --(*this);
    }

    reference dereference() const {
        if (!valid()) throw std::runtime_error("FlatViewIterator: access out of bounds");
        return *child_;
    }

    bool equal(const FlatViewIterator& other) const {
        return (begin_ == other.begin_)
            && (current_ == other.current_)
            && (end_ == other.end_)
            && (
                     (!valid() && !other.valid())
                  || (valid()  &&  other.valid() && child_ == other.child_)
                );
    }

    difference_type distance_to(const FlatViewIterator& other) const {
        // Since we have no guarantee about
        // uniform allocation, we have to explicitly search forwards and backwards
        if ((*this) == other) return 0;

        // search forward
        difference_type result = 0;
        auto tmp = (*this);
        while (tmp.valid()) {
            ++tmp;
            ++result;
            if (tmp == other) return result;
        }

        // if not found: search backward
        result = 0;
        tmp = (*this);
        do {
            --tmp;
            --result;
            if (tmp == other) return result;
        } while (tmp.valid());

        throw std::runtime_error(
            "Attempted to compare iterators not belonging to same container"
        );
    }


private: // members
    ChildIterator child_;
    RawIterator begin_;
    RawIterator current_;
    RawIterator end_;
};

// **************************************************************************
// Specialization for RawIterator pointing to a scalar
template <template<typename> class ScalarPolicy, typename RawIterator, bool isConstIterator>
class FlatViewIterator<ScalarPolicy, RawIterator, isConstIterator, false>
{
public:
    using RawValueType = typename IteratorScalarType<ScalarPolicy, RawIterator>::type;
    using ConstValueType = typename std::add_const<RawValueType>::type;
    using RawReferenceType = typename IteratorScalarType<ScalarPolicy, RawIterator>::reference;
    using ConstReferenceType = typename std::add_const<RawValueType>::type;

    // [iterator.traits]
    using iterator_category = std::random_access_iterator_tag;
    using value_type = typename std::conditional<isConstIterator, ConstValueType, RawValueType>::type;
    using difference_type = ptrdiff_t;
    using pointer = value_type*;
    using reference = typename std::conditional<isConstIterator, ConstReferenceType, RawReferenceType>::type;

    // **************************************************************************
    // ctors
    // **************************************************************************
    FlatViewIterator() : valid_{false}, begin_{}, current_{}, end_{} {}
    /* \brief Default constructor. */

    FlatViewIterator(const FlatViewIterator&) = default;
    /* \brief Copy constructor. Note that FlatViewIterator is TriviallyCopyable */

    static FlatViewIterator makeBegin(RawIterator first, RawIterator last) {
        return FlatViewIterator<ScalarPolicy, RawIterator, isConstIterator>{first, last, Forward{}};
    }
    static FlatViewIterator makeEnd(RawIterator first, RawIterator last) {
        return FlatViewIterator<ScalarPolicy, RawIterator, isConstIterator>{first, last, Backward{}};
    }

    // conversion to const_iterator
    template<bool _isConstIterator = isConstIterator>
    FlatViewIterator(FlatViewIterator<ScalarPolicy, RawIterator, false, false> other,
                     typename std::enable_if<_isConstIterator,int>::type* = nullptr) :
        valid_{other.valid_},
        begin_{other.begin_},
        current_{other.current_},
        end_{other.end_}
    {}

    // **************************************************************************

    bool valid() const {return valid_;}

    // **************************************************************************
    // Standard members
    // **************************************************************************

    // [iterator.iterators]
    reference operator*() const {return dereference();}
    FlatViewIterator& operator++() {increment(); return *this;}

    // [input.iterators] and [output.iterators]
    bool operator==(const FlatViewIterator& other) const {return equal(other);}
    bool operator!=(const FlatViewIterator& other) const {return !((*this) == other);}
    pointer operator->() const {return &(**this);}
    FlatViewIterator operator++(int) {auto tmp = *this; ++(*this); return tmp;}

    // [bidirectional.iterators]
    FlatViewIterator& operator--() {decrement(); return *this;}
    FlatViewIterator operator--(int) {auto other = *this; --(*this); return other;}

    // [random.access.iterators]
    FlatViewIterator& operator+=(difference_type n) {advance(n); return *this;}
    FlatViewIterator operator+(difference_type n) const {auto result = (*this); result += n; return result;}
    template <template<typename> class, typename, bool>
    friend FlatViewIterator operator+(difference_type n, const FlatViewIterator& other);
    FlatViewIterator& operator-=(difference_type n) {return (*this += (-n));}
    FlatViewIterator operator-(difference_type n) const {return (*this + (-n));}

    difference_type operator-(const FlatViewIterator& other) const {return other.distance_to(*this);}
    bool operator<(const FlatViewIterator& other) const {return (other - *this) > 0;}
    bool operator>(const FlatViewIterator& other) const {return (other - *this) < 0;}
    bool operator>=(const FlatViewIterator& other) const {return !((*this) < other);}
    bool operator<=(const FlatViewIterator& other) const {return !((*this) > other);}

    reference operator[](difference_type n) const {return *(*this + n);}

private: // funcs
    // needed for conversion to const_iterator
    friend class FlatViewIterator<ScalarPolicy, RawIterator, true, false>;

    // **************************************************************************
    // private ctors
    // **************************************************************************
    FlatViewIterator(RawIterator first, RawIterator last, Forward)
        : valid_{false}
        , begin_{first}
        , current_{first}
        , end_{last}
    {
        increment();
    }
    FlatViewIterator(RawIterator first, RawIterator last, Backward)
        : valid_{false}
        , begin_{first}
        , current_{last}
        , end_{last}
    {
    }

    // **************************************************************************
    // Basic operations, on the model of Boost's iterator_facade
    // **************************************************************************
    void increment() {
        if (valid_) ++current_;
        valid_ = !(current_ == end_);
    }

    void decrement() {
        if (current_ == begin_) {valid_ = false; return;}
        --current_;
        valid_ = true;
    }

    void advance(difference_type n) {
        if (n > 0) for (difference_type i = 0; i < n; ++i) ++(*this);
        if (n < 0) for (difference_type i = 0; i > n; --i) --(*this);
    }

    reference dereference() const {
        if (!valid_) throw std::runtime_error("FlatViewIterator: access out of bounds");
        return *current_;
        // This is the key difference: being the iterator at the bottom
        // it will return a value, instead of delegating to the subordinate
    }

    bool equal(const FlatViewIterator& other) const {
        return (begin_ == other.begin_) && (current_ == other.current_) && (end_ == other.end_) && (valid_ == other.valid_);
    }

    difference_type distance_to(const FlatViewIterator& other) const {
        // Since this is the bottom level iterator, it could be possible
        // check if the underlying iterator supports random access.
        // But the added complication would buy us very few

        if ((*this) == other) return 0;

        // search forward
        difference_type result = 0;
        auto tmp = (*this);
        while (tmp.valid()) {
            ++tmp;
            ++result;
            if (tmp == other) return result;
        }

        // if not found: search backward
        result = 0;
        tmp = (*this);
        do {
            --tmp;
            --result;
            if (tmp == other) return result;
        } while (tmp.valid());

        throw std::runtime_error(
            "Attempted to compare iterators not belonging to same container"
        );
    }

private: // members
    bool valid_;
    RawIterator begin_;
    RawIterator current_;
    RawIterator end_;
};

// **************************************************************************

template <template<typename> class ScalarPolicy, typename RawIterator, bool isConstIterator>
auto operator+(
        typename FlatViewIterator<ScalarPolicy, RawIterator, isConstIterator>::difference_type n,
        const FlatViewIterator<ScalarPolicy, RawIterator, isConstIterator>& other)
    -> FlatViewIterator<ScalarPolicy, RawIterator, isConstIterator>
{
    return other + n;
}

} // namespace multidim - FlatView

// **************************************************************************
// **************************************************************************
// **************************************************************************
// @BoxedView
// **************************************************************************
// **************************************************************************
// **************************************************************************

namespace multidim {

// **************************************************************************
// forward declarations
/** \brief The iterator used in BoxedView. Unfortunately, it is not
 * a "proper" iterator, since it is proxied.
 *  \ingroup boxed_view
 */
template <
    template<typename> class ScalarPolicy,
    typename RawIterator,
    bool isConstIterator,
    size_t dimensionality
>
class BoxedViewIterator;

// n + iterator
template <template<typename> class ScalarPolicy, typename RawIterator, bool isConstIterator, size_t dimensionality>
auto operator+(
        typename BoxedViewIterator<ScalarPolicy, RawIterator, isConstIterator, dimensionality>::difference_type n,
        const BoxedViewIterator<ScalarPolicy, RawIterator, isConstIterator, dimensionality>& other)
    -> BoxedViewIterator<ScalarPolicy, RawIterator, isConstIterator, dimensionality>;

/** \brief Proxy class representing a reference to a (possibly defaulted)
 *  scalar value. Plays the same role as vector<bool>::reference.
 *  \ingroup boxed_view
 */
template <typename RawIterator, bool isConstProxy>
class BoxedViewScalarProxy;

// **************************************************************************

//***************************************************************************
// BoxedView
//***************************************************************************
/** \brief A class which makes a multilevel range appear as a C array
 * of user-defined bounds.
 * It respects many of the General container requirements
 * ([container.requirements.general]),
 * but to carry out its functions, it needs to use proxied iterators
 * (just like vector<bool>).
 * Other discrepancies arise from the fact that the View does not own
 * its elements.
 * In general, we tried to follow the semantics of C++17's `string_view`.
 * \param ScalarPolicy (trait template)
 * \ingroup boxed_view
 */
template <
    template<typename> class ScalarPolicy,
    typename RawIterator,
    size_t dimensionality_
>
class BoxedView {
public:
    using iterator = BoxedViewIterator<
        ScalarPolicy, RawIterator, false, dimensionality_
    >;
    using const_iterator =  BoxedViewIterator<
        ScalarPolicy, RawIterator, true, dimensionality_
    >;
    using value_type = typename iterator::value_type;
    using reference = typename iterator::reference;
    using const_reference = typename const_iterator::reference;
    using difference_type = typename iterator::difference_type;
    using size_type = size_t;   /**< \todo should actually be generic */

    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    using ScalarType = typename iterator::ScalarType;

    // **************************************************************************
    // ctors
    // **************************************************************************
    BoxedView() {}
    template <typename BoundsIterator>
    BoxedView(
        RawIterator first, RawIterator last,
        ScalarType defaultValue, BoundsIterator boundsFirst
    ) :
        begin_{first},
        end_{last},
        defaultValue_{std::move(defaultValue)}
    {
        std::copy_n(boundsFirst, dimensionality_, bounds_);
    }
    BoxedView(const BoxedView& other) = default;
    /**< \note It performs a shallow copy */
    ~BoxedView() = default;
    BoxedView& operator=(const BoxedView& other) = default;
    /**< \note It performs a shallow copy */

    // **************************************************************************
    // Standard members
    // **************************************************************************
    bool operator==(const BoxedView& other) const {return equal(other);}
    bool operator!=(const BoxedView& other) const {return !equal(other);}
    bool operator <(const BoxedView& other) const {return compare(other);}
    bool operator >(const BoxedView& other) const {return other.compare(*this);}
    bool operator<=(const BoxedView& other) const {return !(*this > other);}
    bool operator>=(const BoxedView& other) const {return !(other > *this);}

    iterator begin() {
        return iterator::makeBegin(begin_, end_, &defaultValue_, bounds_);
    }
    const_iterator begin() const {
        return const_iterator::makeBegin(begin_, end_, &defaultValue_, bounds_);
    }
    const_iterator cbegin() const {
        return const_iterator::makeBegin(begin_, end_, &defaultValue_, bounds_);
    }
    iterator end() {
        return iterator::makeEnd(begin_, end_, &defaultValue_, bounds_);
    }
    const_iterator end() const {
        return const_iterator::makeEnd(begin_, end_, &defaultValue_, bounds_);
    }
    const_iterator cend() const {
        return const_iterator::makeEnd(begin_, end_, &defaultValue_, bounds_);
    }
    reverse_iterator rbegin() {
        return reverse_iterator{
            iterator::makeEnd(begin_, end_, &defaultValue_, bounds_)
        };
    }
    const_reverse_iterator rbegin() const {
        return const_reverse_iterator{
            const_iterator::makeEnd(begin_, end_, &defaultValue_, bounds_)
        };
    }
    const_reverse_iterator crbegin() const {
        return const_reverse_iterator{
            const_iterator::makeEnd(begin_, end_, &defaultValue_, bounds_)
        };
    }
    reverse_iterator rend() {
        return reverse_iterator{
            iterator::makeBegin(begin_, end_, &defaultValue_, bounds_)
        };
    }
    const_reverse_iterator rend() const {
        return const_reverse_iterator{
            const_iterator::makeBegin(begin_, end_, &defaultValue_, bounds_)
        };
    }
    const_reverse_iterator crend() const {
        return const_reverse_iterator{
            const_iterator::makeBegin(begin_, end_, &defaultValue_, bounds_)
        };
    }

    reference front() {return *begin();}
    const_reference front() const {return *cbegin();}
    reference back() {return *--end();}
    const_reference back() const {return *--cend();}

    reference operator[](size_type n) {return *(begin() + n);}
    const_reference operator[](size_type n) const {return *(begin() + n);}

    void swap(const BoxedView& other) {std::swap(*this, other);}
    size_type size() const {
        // size should be constant, I guess that amortized constant will do
        if (cachedSize_ == NO_VALUE) {
            cachedSize_ = std::distance(begin_, end_);
        }
        return cachedSize_;
    }
    size_type max_size() const {return std::numeric_limits<size_type>::max();};
     /**<  \note Since C++17 the result of max_size should be divided
      * by `sizeof(value_type)`,
      * see http://en.cppreference.com/w/cpp/memory/allocator_traits/max_size
      */

    bool empty() const {return (size() == 0);}

private:
    bool equal(const BoxedView& other) const {
        return (begin_ == other.begin_)
            && (end_ == other.end_)
            && std::equal(
                    begin(bounds_), end(bounds_),
                    begin(other.bounds_), end(other.bounds_)
               );
    }

    bool compare(const BoxedView& other) const {
        return std::lexicographical_compare(
            begin(), end(),
            other.begin(), other.end()
        );
    }

private:
    RawIterator begin_ = nullptr;
    RawIterator end_ = nullptr;
    mutable size_t cachedSize_ = NO_VALUE;
    ScalarType defaultValue_;
    size_t bounds_[dimensionality_] = {0};

};

//***************************************************************************
// makeBoxedView
//***************************************************************************
/** \brief Factory method to build a BoxedView of a range
 * \ingroup user_functions
 * \param ScalarPolicy : (trait template)
 * \param first, last the range on which the View will be based
 */
template <
    template<typename> class ScalarPolicy = NoCustomScalars,
    typename Iterator = void,
    typename ScalarValue = void,
    typename BoundsContainer = void
>
auto makeBoxedView(
    Iterator first, Iterator last,
    ScalarValue&& defaultValue, const BoundsContainer& viewBounds
)
    -> BoxedView<
        ScalarPolicy, Iterator,
        DimensionalityRange<ScalarPolicy, Iterator>::value
       >
{
    using std::begin;
    if (begin(viewBounds) ==  end(viewBounds)) {
        auto containerBounds = bounds<ScalarPolicy>(first, last);
        return BoxedView<ScalarPolicy, Iterator, dimensionality(first, last)>
            ( first, last,
              std::forward<ScalarValue>(defaultValue), begin(containerBounds)
             );
    }

    if (
        std::distance(begin(viewBounds), end(viewBounds)) !=
        dimensionality(first, last)
    ) {
        throw std::runtime_error("makeBoxedView : limit list has false size");
    }

    return BoxedView<ScalarPolicy, Iterator, dimensionality(first, last)>
        (first, last, std::forward<ScalarValue>(defaultValue), begin(viewBounds));
}

// The same, with a initializer list
template <
    template<typename> class ScalarPolicy = NoCustomScalars,
    typename Iterator = void,
    typename ScalarValue = void
>
auto makeBoxedView(
    Iterator first, Iterator last,
    ScalarValue&& defaultValue, std::initializer_list<size_t> viewBounds
)
    -> BoxedView<
        ScalarPolicy, Iterator,
        DimensionalityRange<ScalarPolicy, Iterator>::value
       >
{
    using std::begin;
    if (begin(viewBounds) == end(viewBounds)) {
        auto containerBounds = bounds<ScalarPolicy>(first, last);
        return BoxedView<ScalarPolicy, Iterator, dimensionality(first, last)>
            (first, last,
             std::forward<ScalarValue>(defaultValue), begin(containerBounds)
            );
    }

    if (
        std::distance(begin(viewBounds), end(viewBounds)) !=
        dimensionality(first, last)
    ) {
        throw std::runtime_error("makeBoxedView : limit list has false size");

    }

    return BoxedView<ScalarPolicy, Iterator, dimensionality(first, last)>
        (first, last, std::forward<ScalarValue>(defaultValue), begin(viewBounds));
}

/** \brief Factory method to build a BoxedView of a container
 *  (delegates to the range.based version)
 * \ingroup user_functions
 * \param ScalarPolicy (trait template)
 * \param container the container on which the View will be based
 */
template <
    template<typename> class ScalarPolicy = NoCustomScalars,
    typename Container = void,
    typename ScalarValue = void,
    typename BoundsContainer = void
>
auto makeBoxedView(
    Container& container, ScalarValue&& defaultValue,
    const BoundsContainer& viewBounds
)
    -> BoxedView<
            ScalarPolicy, decltype(begin(container)),
            Dimensionality<ScalarPolicy, Container>::value
       >
{
    return makeBoxedView(
        begin(container), end(container),
        std::forward<ScalarValue>(defaultValue), viewBounds
    );
}

// The same, with a initializer list
template <
    template<typename> class ScalarPolicy = NoCustomScalars,
    typename Container = void,
    typename ScalarValue = void
>
auto makeBoxedView(
    Container& container, ScalarValue&& defaultValue,
    std::initializer_list<size_t> viewBounds
)
    -> BoxedView<
        ScalarPolicy, decltype(begin(container)),
        Dimensionality<ScalarPolicy, Container>::value
       >
{
    return makeBoxedView(
        begin(container), end(container),
        std::forward<ScalarValue>(defaultValue), viewBounds
    );
}

//***************************************************************************
// BoxedViewScalarProxy
//***************************************************************************
/** \brief Proxy class representing a reference to a (possibly defaulted)
 *  scalar value. Plays the same role as vector<bool>::reference
 *  \ingroup boxed_view
 */
template <typename RawIterator, bool isConstProxy>
class BoxedViewScalarProxy {
    using RawScalarType = typename std::iterator_traits<RawIterator>::value_type;
    using ConstScalarType = typename std::add_const<RawScalarType>::type;
    using ScalarType = typename std::conditional<
        isConstProxy, ConstScalarType, RawScalarType
    >::type;

public:
    BoxedViewScalarProxy(
        RawIterator target, ScalarType const* defaultValue, bool valid
    ) :
        target_{target},
        defaultValue_{defaultValue},
        valid_{valid} {}

    BoxedViewScalarProxy& operator=(const ScalarType& rhs) {
        if (valid_) *target_ = rhs;
        return *this;
    }

    operator ScalarType() const {
        if (valid_)
            return *target_;
        else
            return *defaultValue_;
    }

private: // members
    RawIterator target_;
    ScalarType const* defaultValue_;
    bool valid_;
};

//***************************************************************************
// BoxedViewIteratorTraits
//***************************************************************************

// typedefs for dimensionality_ > 1
template <
    template<typename> class ScalarPolicy,
    typename RawIterator,
    bool isConstIterator,
    size_t dimensionality_
>
struct BoxedViewIteratorTraits {
    using RawScalarType =
        typename IteratorScalarType<ScalarPolicy, RawIterator>::type;
    using ConstScalarType = typename std::add_const<RawScalarType>::type;
    using ScalarType = typename std::conditional<
        isConstIterator, ConstScalarType, RawScalarType
    >::type;

    using ChildRawIterator = typename IteratorType<
        typename std::iterator_traits<RawIterator>::reference
    >::type;
    using ChildIterator = BoxedViewIterator<
        ScalarPolicy, ChildRawIterator,
        isConstIterator, dimensionality_ - 1
    >;

    // [iterator.traits]
    using value_type = ChildIterator;
    using pointer = value_type*;
    using reference = value_type;
};

// typedefs for dimensionality_ == 1
template <
    template<typename> class ScalarPolicy,
    typename RawIterator,
    bool isConstIterator
>
struct BoxedViewIteratorTraits<
    ScalarPolicy, RawIterator, isConstIterator, /*dimensionality_*/ 1
> {
    using RawScalarType =
        typename IteratorScalarType<ScalarPolicy, RawIterator>::type;
    using ConstScalarType = typename std::add_const<RawScalarType>::type;
    using ScalarType = typename std::conditional<
        isConstIterator, ConstScalarType, RawScalarType
    >::type;

    using ScalarProxy = BoxedViewScalarProxy<RawIterator, isConstIterator>;

    // [iterator.traits]
    using value_type = ScalarType;
    using pointer = ScalarProxy;
    using reference = ScalarProxy;
};

//***************************************************************************
// BoxedViewIterator
//***************************************************************************
// Specialization for RawIterator pointing to a subcontainer
// Precondition: viewBounds_ is an array
// having at least `dimensionality_` elements

template <
    template<typename> class ScalarPolicy,
    typename RawIterator,
    bool isConstIterator,
    size_t dimensionality_
>
class BoxedViewIterator {
public:
    using Traits = BoxedViewIteratorTraits<
        ScalarPolicy, RawIterator, isConstIterator, dimensionality_
    >;
    using ScalarType = typename Traits::ScalarType;

    // [iterator.traits]
    using iterator_category = std::random_access_iterator_tag;
    using value_type = typename Traits::value_type;
    using difference_type = ptrdiff_t;
    using pointer = typename Traits::pointer;
        // Child iterator if dimensionality_ > 1, proxy class if == 1
    using reference = typename Traits::reference;
        // Child iterator if dimensionality_ > 1, proxy class if == 1

    // **************************************************************************
    // ctors
    // **************************************************************************
    BoxedViewIterator() :
        current_{nullptr},
        physicalBound_{0}, viewBounds_{nullptr}, index_{0},
        defaultValue_{nullptr}
        {}
    /* \brief Default constructor. */

    BoxedViewIterator(const BoxedViewIterator&) = default;
    /* \brief Copy constructor. Note that BoxedViewIterator is TriviallyCopyable */

    static BoxedViewIterator makeBegin(
        RawIterator first, RawIterator last,
        ScalarType const* defaultValue, size_t const* viewBounds)
    {
        return BoxedViewIterator<
            ScalarPolicy, RawIterator, isConstIterator, dimensionality_
        > {first, last, defaultValue, viewBounds, Forward{}};
    }
    static BoxedViewIterator makeEnd(
        RawIterator first, RawIterator last,
        ScalarType const* defaultValue, size_t const* viewBounds)
    {
        return BoxedViewIterator<
            ScalarPolicy, RawIterator, isConstIterator, dimensionality_
        >{first, last, defaultValue, viewBounds, Backward{}};
    }

    // conversion to const_iterator
    template<bool _isConstIterator = isConstIterator>
    BoxedViewIterator(
        BoxedViewIterator<ScalarPolicy, RawIterator, false, dimensionality_> other,
        typename std::enable_if<_isConstIterator,int>::type* = nullptr
    ) :
        current_{other.current_},
        physicalBound_{other.physicalBound_},
        viewBounds_{other.viewBounds_},
        index_{other.index_},
        defaultValue_{other.defaultValue_}
    {}

    // **************************************************************************
    // Standard members
    // **************************************************************************
    // [iterator.iterators]
    reference operator*() const {return dereference();}
    BoxedViewIterator& operator++() {increment(); return *this;}

    // [input.iterators] and [output.iterators]
    bool operator==(const BoxedViewIterator& other) const {return equal(other);}
    bool operator!=(const BoxedViewIterator& other) const {
        return !((*this) == other);
    }
    pointer operator->() const {return &(**this);}
    BoxedViewIterator operator++(int) {auto tmp = *this; ++(*this); return tmp;}

    // [bidirectional.iterators]
    BoxedViewIterator& operator--() {decrement(); return *this;}
    BoxedViewIterator operator--(int) {
        auto other = *this;
        --(*this);
        return other;
    }

    // [random.access.iterators]
    BoxedViewIterator& operator+=(difference_type n) {advance(n); return *this;}
    BoxedViewIterator operator+(difference_type n) const {auto result = (*this); result += n; return result;}
    template <template<typename> class, typename, bool>
    friend BoxedViewIterator operator+(difference_type n, const BoxedViewIterator& other);
    BoxedViewIterator& operator-=(difference_type n) {return (*this += (-n));}
    BoxedViewIterator operator-(difference_type n) const {return (*this + (-n));}

    difference_type operator-(const BoxedViewIterator& other) const {return other.distance_to(*this);}
    bool operator<(const BoxedViewIterator& other) const {return (other - *this) > 0;}
    bool operator>(const BoxedViewIterator& other) const {return (other - *this) < 0;}
    bool operator>=(const BoxedViewIterator& other) const {return !((*this) < other);}
    bool operator<=(const BoxedViewIterator& other) const {return !((*this) > other);}

    reference operator[](difference_type n) const {return *(*this + n);}

    // **************************************************************************
    // Not part of the "iterator interface", but since we want to emulate a C array
    // they must be provided
    friend BoxedViewIterator begin(const BoxedViewIterator& it) {
        return it - it.index_;
    }
    friend BoxedViewIterator end(const BoxedViewIterator& it) {
        if (it.viewBounds_ == nullptr) return it;
        return it + (it.viewBounds_[0] - it.index_);
    }
private:
    // needed for conversion to const_iterator
    friend class BoxedViewIterator<ScalarPolicy, RawIterator, true, true>;

    // **************************************************************************
    // private ctors
    // **************************************************************************
    BoxedViewIterator(
        RawIterator first, RawIterator last,
        ScalarType const* defaultValue, size_t const* viewBounds,
        Forward
    ) :
        current_{first},
        physicalBound_{static_cast<size_t>(std::distance(first, last))},
        viewBounds_{viewBounds},
        index_{0},
        defaultValue_{defaultValue}
    {}

    BoxedViewIterator(
        RawIterator first, RawIterator last,
        ScalarType const* defaultValue, size_t const* viewBounds,
        Backward
    ) :
        current_{last},
        physicalBound_{static_cast<size_t>(std::distance(first, last))},
        viewBounds_{viewBounds},
        index_{viewBounds[0]},
        defaultValue_{defaultValue}
    {
        if (viewBounds[0] < physicalBound_) {
            // in this case, current_ points to the wrong place
            // (i.e. to the physical end of the container
            // insted of the end of the view)
            // and must be corrected
            size_t currentIndex_ = physicalBound_;
            while (currentIndex_ > viewBounds[0]) {
                --current_;
                --currentIndex_;
            }
            // TODO: overload for random access iterators
        }
    }


    // **************************************************************************
    // Basic operations, on the model of Boost's iterator_facade
    // **************************************************************************
    static constexpr size_t ONE_BEFORE_THE_FIRST = static_cast<size_t>(-1);

    void increment() {
        // We were not requested to perform this check, but given the nature
        // of this iterator, seems appropriate
        if (viewBounds_ == nullptr || index_ == viewBounds_[0])
            throw std::runtime_error("BoxedViewIterator: access out of bounds");

        if (index_ < physicalBound_) ++current_;
        ++index_;
    }

    void decrement() {
        // We would are not requested to perform this check, but given the nature
        // of this iterator, seems appropriate
        if (viewBounds_ == nullptr || index_ == ONE_BEFORE_THE_FIRST)
            throw std::runtime_error("BoxedViewIterator: access out of bounds");

        if (index_ > 0 && index_ <= physicalBound_) --current_;
        --index_;
        // Note: if the old index_ was == 0,
        // it will assume value ONE_BEFORE_THE_FIRST
    }

    void advance(difference_type n) {
        // We would not be requested to perform this check, but given the nature
        // of this iterator, seems appropriate
        if (
             (
                viewBounds_ == nullptr
             ) || (
                static_cast<difference_type>(index_) + n >
                    static_cast<difference_type>(viewBounds_[0])
             ) || (
                static_cast<difference_type>(index_) + n < -1
             )
        ) {
            throw std::runtime_error("BoxedViewIterator: access out of bounds");
        }

        if (index_ + n > physicalBound_) { // over the upper bound
            std::advance(current_, physicalBound_ - index_);
        } else if (index_ + n < 0) {  // under to lower bound (==0)
            std::advance(current_, -index_);
        } else {
            std::advance(current_, n);
        }

        index_ += n;
    }

    // **************************************************************************
    // distance_to - Version if the underlying raw iterator
    // supports random access (we can use std::distance in this case)
    template<bool rawIteratorIsRandomAccess =
        std::is_same<
            typename std::iterator_traits<RawIterator>::iterator_category,
            std::random_access_iterator_tag
        >::value
    >
    auto distance_to(const BoxedViewIterator& other) const
        -> typename std::enable_if<
               true == rawIteratorIsRandomAccess,
               difference_type
           >::type
    {
        return
            std::distance(this->current_, other.current_) -
            (
                (index_ > physicalBound_) ?
                    (index_ - physicalBound_) : 0
            ) +
            (
                (other.index_ > other.physicalBound_) ?
                    (other.index_ - other.physicalBound_) : 0
            );
    }

    // distance_to - Version if the underlying raw iterator
    // does not support random access: we cannot but crawl in both directions,
    // till we reach the other iterator
    template<bool rawIteratorIsRandomAccess =
        std::is_same<
            typename std::iterator_traits<RawIterator>::iterator_category,
            std::random_access_iterator_tag
        >::value
    >
    auto distance_to(const BoxedViewIterator& other) const
        -> typename std::enable_if<
               false == rawIteratorIsRandomAccess,
               difference_type
           >::type
    {
        if (*this == other) return 0;

        difference_type distance;

        // forward crawl
        distance = 0;
        auto duplicate = *this;
        while (duplicate.index_ < duplicate.viewBounds_[0]) {
            ++duplicate;
            ++distance;
            if (duplicate == other) return distance;
        }

        // backward crawl
        distance = 0;
        duplicate = *this;
        while (duplicate.index_ > 0) {
            --duplicate;
            --distance;
            if (duplicate == other) return distance;
        }

        throw std::runtime_error(
            "Attempted to compare iterators not belonging to same container"
        );
    }

    // **************************************************************************
    // dereference - Version for dimensionality_ > 1
    // returns a BoxedViewIterator to the subcontainer
    template<bool hasSubRange = (dimensionality_ > 1)>
    auto dereference() const
        -> typename std::enable_if<true == hasSubRange, reference>::type
    {
        if (
                (viewBounds_ == nullptr)
             || (index_ == ONE_BEFORE_THE_FIRST)
             || (index_ == viewBounds_[0])
        ) {
            throw std::runtime_error("BoxedViewIterator: access out of bounds");
        }
        if (index_ < physicalBound_) {
            return reference::makeBegin(
                begin(*current_), end(*current_),
                defaultValue_, viewBounds_ + 1
                    // will shift the bound list forward
            );
        } else {
            using ChildRawIterator = typename Traits::ChildRawIterator;
            return reference::makeBegin(
                ChildRawIterator{}, ChildRawIterator{},
                defaultValue_, viewBounds_ + 1
                    // will shift the bound list forward
            );
        }
    }

    // dereference - Version for dimensionality_ == 1
    // returns a proxy to the value
    template<bool hasSubRange = (dimensionality_ > 1)>
    auto dereference() const
        -> typename std::enable_if<false == hasSubRange, reference>::type
    {
        if (
                (viewBounds_ == nullptr)
             || (index_ == ONE_BEFORE_THE_FIRST)
             || (index_ == viewBounds_[0])
        ) {
            throw std::runtime_error("BoxedViewIterator: access out of bounds");
        }
        if (index_ < physicalBound_) {
            return reference{current_, defaultValue_, true};
        } else {
            return reference{current_, defaultValue_, false};
        }
    }

    bool equal(const BoxedViewIterator& other) const {
        if (viewBounds_ == nullptr && other.viewBounds_ == nullptr) return true;
        if (viewBounds_ == nullptr || other.viewBounds_ == nullptr) return false;

        return (current_ == other.current_)
            && (physicalBound_ == other.physicalBound_)
            && (std::equal(viewBounds_, viewBounds_+dimensionality_,
                    other.viewBounds_)
                )
            && (index_ == other.index_)
            && (*defaultValue_ == *other.defaultValue_);
    }


private: // members
    RawIterator   current_;
    size_t        physicalBound_;
        // size of the underlying container
    size_t const* viewBounds_;
        // size the user of the class will see (throug filling or cropping)
    size_t        index_;
    ScalarType const* defaultValue_; // observer_ptr
};


// **************************************************************************

template <
    template<typename> class ScalarPolicy,
    typename RawIterator,
    bool isConstIterator,
    size_t dimensionality
>
auto operator+(
        typename BoxedViewIterator<
            ScalarPolicy, RawIterator,
            isConstIterator, dimensionality
        >::difference_type n,
        const BoxedViewIterator<
            ScalarPolicy, RawIterator,
            isConstIterator, dimensionality
        >& other
)
    -> BoxedViewIterator<ScalarPolicy, RawIterator, isConstIterator, dimensionality>
{
    return other + n;
}

} // namespace - BoxedView

#endif // MULTIDIM_H

