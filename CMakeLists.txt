
cmake_minimum_required (VERSION 3.20)
project (HashTable CXX)

add_executable (${PROJECT_NAME}
    main.cpp
    FileBufferizer.cpp 
    hash.cpp 
    TextPreprocessor.cpp
)

target_include_directories (${PROJECT_NAME}
    PRIVATE
    .
)

target_compile_features (${PROJECT_NAME}
    PRIVATE
        cxx_std_17
)

target_compile_options (${PROJECT_NAME}
    PRIVATE
        -fdiagnostics-color=always

        -Wall
        -Wextra
        -fstack-protector-strong
        -fcheck-new
        -fstrict-overflow

        -march=native
        $<$<CONFIG:Debug>:
            -Og
            -g3
            -DDEBUG
            -ggdb
            -fsanitize=address,alignment,bool,bounds,enum,float-cast-overflow,float-divide-by-zero,integer-divide-by-zero,leak,nonnull-attribute,null,object-size,return,returns-nonnull-attribute,shift,signed-integer-overflow,undefined,unreachable,vla-bound,vptr 
        >

        $<$<CONFIG:Release>:
            -O2
            -flto
            -DNDEBUG
        >
)

target_link_options (${PROJECT_NAME}
    PRIVATE
        -march=native
        -Wl,-q
        $<$<CONFIG:Debug>:
            -Og
            -g3
            -DDEBUG
            -ggdb
            -fsanitize=address,alignment,bool,bounds,enum,float-cast-overflow,float-divide-by-zero,integer-divide-by-zero,leak,nonnull-attribute,null,object-size,return,returns-nonnull-attribute,shift,signed-integer-overflow,undefined,unreachable,vla-bound,vptr 
        >

        $<$<CONFIG:Release>:
            -O2
            -flto
            -DNDEBUG
        >
)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON) # to generate compile_commands.json

# cmake -B build -S . -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_COMPILER=g++
# cmake --build build
