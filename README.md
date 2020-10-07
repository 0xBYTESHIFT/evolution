# Evolution

Evolution is an amateur neural network research project. Here we generate random creatures with "brains", put them into the world and watch them looking and fighting for food.

Since their neural networks are completely random (in given parameters boundaries like min. and max. weight value), at first they do it not so good. But as generations pass, they should succseed more and more.

## Dependencies & requirements
* OpenBlas (for faster inference, might be removed later)
* cmake 3.11
* C++17-compatible clang or g++
* internet connection to fetch dependencies
* time & patience

## Building
```bash
mkdir -p build/ && cd build
cmake .. && cmake --build . --config Release --target all -- -j 6
```