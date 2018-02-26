#include <algorithm>
#include <array>
#include <chrono>
#include <cstdlib>
#include <iostream>
#include <iterator>
#include <memory>
#include <new>
#include <random>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#define BSP_STORAGE_POOL_LOG_ERROR(message) std::cerr << "Error! " << message << "\n";

const bool DebugLogAllocations = true;
#define BSP_STORAGE_POOL_ALLOCATION(message, bytes) do { \
        if (DebugLogAllocations) \
			std::cout << "Memory: Allocated " << (bytes / 1024) << "kB \"" << message << "\"\n"; \
		} while(false)

#include <typeinfo>

namespace {
    template <typename T, typename U>
    using type_check = typename std::enable_if<std::is_same<typename std::decay<T>::type, U>::value, void>::type*;

    template <typename T> 
    std::string type_name(type_check<T, int> = 0){
        return typeid(T).name();
    }

    template <typename T, typename U = typename T::value_type> 
    std::string type_name(type_check<T, std::vector<U>> = 0){
        std::ostringstream oss;
        oss << "vector<" << type_name<U>() << ">";
        return oss.str();
    }
}

#define BSP_TYPE_NAME(type) type_name<type>()

#include "../include/storage_pool.h"

#include "catch.hpp"
#include "container_matcher.h"

using bsp::storage_pool;
using Catch::Equals;

TEST_CASE("storage_pool", "[storage_pool]") {    
    SECTION("default construction (ints)"){
        storage_pool<int> arr;
        CHECK(arr.storage_count() == 0); 
    }

    SECTION("construction (ints)"){
        storage_pool<int> arr { 512 };
        CHECK(arr.storage_count() == 1); 
        CHECK(arr.size() == 512);
    }

    SECTION("adding storage (ints)"){
        storage_pool<int> arr { 512 };
        bool success = arr.append_new_storage(256);
        CHECK(success == true);
        CHECK(arr.storage_count() == 2); 
        CHECK(arr.size() == 512 + 256);
    }

    SECTION("creating and destroying ints"){
        storage_pool<int> arr { 512 };
        new (&arr[0]) int {42};
        CHECK(arr[0] == 42);
        // No need to destroy
    }

    using int_vector = std::vector<int>;

    SECTION("default construction (int_vector)"){
        storage_pool<int_vector> arr;
        CHECK(arr.storage_count() == 0); 
    }

    SECTION("construction (int_vector)"){
        storage_pool<int_vector> arr { 512 };
        CHECK(arr.storage_count() == 1); 
        CHECK(arr.size() == 512);
    }

    SECTION("adding storage (int_vector)"){
        storage_pool<int_vector> arr { 512 };
        bool success = arr.append_new_storage(256);
        CHECK(success == true);
        CHECK(arr.storage_count() == 2); 
        CHECK(arr.size() == 512 + 256);
    }

    SECTION("creating and destroying (int_vector)"){
        storage_pool<int_vector> arr { 512 };
        new (&arr[0]) int_vector(100, 42);
        CHECK(arr[0].size() == 100);
        CHECK(arr[0][0] == 42);
        (&arr[0])->~int_vector();
    }

#ifdef BSP_STORAGE_POOL_LOG_ERROR
     SECTION("allocation error"){
        std::cout << "Should cause a bad_array_new_length exception or print an allocation error...\n";

        try {
            storage_pool<int_vector> vec;
            int shrink_factor = 0;
            for (int i=0; i<10; i++){
                bool res = vec.append_new_storage((1 << (18 + i)) >> shrink_factor);
                if (!res) shrink_factor+=4; // make next allocation smaller
            }
        }
        catch (const std::bad_array_new_length& e){
            std::cout << "Exception caught: " << e.what() << "\n";
        }
     }
#endif
}

