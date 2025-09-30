declare -a files=(
    lib/gl 
    lib/driver
    lib/glx
    lib/stubs
    lib/utils
    lib/wgl
    lib/threadrunner
    unittest/cpp
    example/minimal
    example/mipmap
    example/stencilShadow
    example/vbo
    example/util
    example/platformio
)

for i in "${files[@]}"
do
    echo "format $i"
    find $i/* -iname '*.hpp' -o -iname '*.h' -o -iname '*.c' -o -iname '*.cpp' | xargs clang-format -i
done

