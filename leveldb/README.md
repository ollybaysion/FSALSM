LevelDB에 ext4 File System의 journaling 기법을 적용하여, fsync의 latency를 줄이는 선행연구 (NobLSM) 기반으로 consistency를 고려한 성능 개선 연구 진행

# Building

This project supports [CMake](https://cmake.org/) out of the box.

### Build for POSIX

Quick start:

```bash
mkdir -p build && cd build
cmake -DCMAKE_BUILD_TYPE=Release .. && cmake --build .
```
